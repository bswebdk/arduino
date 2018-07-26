/*
  
  Copyright © 2018 - Torben Bruchhaus
  TDuino.bruchhaus.dk - github.com/bswebdk/TDuino
  File: arduino_benchmark.ino
  
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>
  
*/

// Notes:
// ------
// With a 16MHz processor the resolution of the timer is 4us and with 8MHz the
// resolution is 8us. Considering the timer register is only 8 bit, this gives
// an expected minimum rollover at (256*4) 1024. When interrupts are disabled
// the rollover will not be handled and this means that testing without interrupts
// is limited to a maximum of 1024us. From this value we must subtract the time it
// takes to perform a single operation plus the overhead of the while loop and
// this gives us an approximate maximum of 900us to do our benchmarks. Since
// longer testing time yields better results (*), testing with interrupts disabled
// does not yield good results. In fact I would consider the results from testing
// without interrupts as being unreliable.
//
// (*) Floating point performance - especially division - will decrease as the
// testing time increases whereas most other data types will not. This behaviour
// is most likely caused by more interference from interrupts during longer tests.

#define TEST_US_NOINTR 900
#define TEST_US 100000 //100 millis

//Uncomment the following line to do additional benchmarks with interrupts
//disabled - read notes above first, though:
//#define NO_INTERRUPTS

enum DATATYPE  { DT_8BIT=0, DT_16BIT, DT_32BIT, DT_FLOAT };
const char* DATATYPE_NAME[4]  = { " 8-bit", "16-bit", "32-bit", "Floats" };

enum OPERATION { OP_ADD=0, OP_SUB, OP_MUL, OP_DIV }; 
const char* OPERATION_NAME[4] = { "Addition", "Subtraction", "Multiplication", "Division" };

char cb[25];
bool intr, by_operation = true;
unsigned long start_us, end_us, test_us, count;

/*

Arduino Nano results (various junk attached):

[Addition]
 8-bit  : 0.15 us
16-bit  : 0.20 us
32-bit  : 0.32 us
Floats  : 8.42 us

[Subtraction]
 8-bit  : 0.14 us
16-bit  : 0.20 us
32-bit  : 0.32 us
Floats  : 8.34 us

[Multiplication]
 8-bit  : 0.33 us
16-bit  : 0.76 us
32-bit  : 5.05 us
Floats  : 6.11 us

[Division]
 8-bit  : 5.25 us
16-bit  : 12.60 us
32-bit  : 36.49 us
Floats  : 71.53 us

[ 8-bit]
Addition        : 0.14 us
Subtraction     : 0.14 us
Multiplication  : 0.33 us
Division        : 5.24 us

[16-bit]
Addition        : 0.21 us
Subtraction     : 0.20 us
Multiplication  : 0.77 us
Division        : 12.61 us

[32-bit]
Addition        : 0.33 us
Subtraction     : 0.33 us
Multiplication  : 5.05 us
Division        : 36.51 us

[Floats]
Addition        : 8.44 us
Subtraction     : 8.35 us
Multiplication  : 6.11 us
Division        : 71.57 us

*/

float while_us = 0, while_us_nointr = 0;
void estimate_while_us()
{

  //Estimate the time used by a while loop which increments a counter
  count = 0;
  test_us = TEST_US;
  start_us = micros();
  while (micros() - start_us < test_us) { count++; }
  end_us = micros();
  while_us = (float)(end_us - start_us) / (float)count;

  count = 0;
  test_us = TEST_US_NOINTR;
  noInterrupts();
  start_us = micros();
  while (micros() - start_us < test_us) { count++; }
  end_us = micros();
  interrupts();
  while_us_nointr = (float)(end_us - start_us) / (float)count;

  Serial.print("while overhead  : ");
  Serial.println(while_us);
  Serial.print("while overhead* : ");
  Serial.println(while_us_nointr);
}

template <class T> void benchmark(T a, T b, OPERATION operation, DATATYPE datatype)
{
  
  count = 0;
  test_us = TEST_US;
  if (!intr)
  {
    test_us = TEST_US_NOINTR;
    noInterrupts();
  }
  
  switch (operation)
  {
    case OP_ADD:
      start_us = micros();
      while (micros() - start_us < test_us) { a += b; count++; }
      end_us = micros();
      break;
      
    case OP_SUB:
      start_us = micros();
      while (micros() - start_us < test_us) { a -= b; count++; }
      end_us = micros();
      break;
      
    case OP_MUL:
      start_us = micros();
      while (micros() - start_us < test_us) { a *= b; count++; }
      end_us = micros();
      break;
      
    case OP_DIV:
      start_us = micros();
      while (micros() - start_us < test_us) { a /= b; count++; }
      end_us = micros();
      break;
  }
  
  if (!intr) interrupts();
  
  if (a == b) Serial.println(); //Prevent operations from being optimized away by the compiler
  
  byte i;
  cb[0] = 0;
  if (by_operation)
  {
    strcat(cb, DATATYPE_NAME[datatype]);
    i = 8;
  }
  else
  {
    strcat(cb, OPERATION_NAME[operation]);
    i = 16;
  }
  if (!intr) strcat(cb, "*");
  while (strlen(cb) < i) strcat(cb, " ");
  strcat(cb, ": ");
    
  float score = ((float)(end_us - start_us) / (float)count) - (intr ? while_us : while_us_nointr);
  Serial.print(cb);
/*
  
  Copyright © 2018 - Torben Bruchhaus
  TDuino.bruchhaus.dk - github.com/bswebdk/TDuino
  File: arduino_benchmark.ino
  
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>
  
*/

// Notes:
// ------
// With a 16MHz processor the resolution of the timer is 4us and with 8MHz the
// resolution is 8us. Considering the timer register is only 8 bit, this gives
// an expected minimum rollover at (256*4) 1024. When interrupts are disabled
// the rollover will not be handled and this means that testing without interrupts
// is limited to a maximum of 1024us. From this value we must subtract the time it
// takes to perform a single operation plus the overhead of the while loop and
// this gives us an approximate maximum of 900us to do our benchmarks. Since
// longer testing time yields better results (*), testing with interrupts disabled
// does not yield good results. In fact I would consider the results from testing
// without interrupts as being unreliable.
//
// (*) Floating point performance - especially division - will decrease as the
// testing time increases whereas most other data types will not. This behaviour
// is most likely caused by more interference from interrupts during longer tests.

#define TEST_US_NOINTR 900
#define TEST_US 100000 //100 millis

//Uncomment the following line to do additional benchmarks with interrupts
//disabled - read notes above first, though:
//#define NO_INTERRUPTS

//Data types used for various tests
#define DT8BIT  uint8_t
#define DT16BIT uint16_t
#define DT32BIT uint32_t
#define DT64BIT uint64_t
#define DTFLOAT float

enum DATATYPE  { DT_8BIT=0, DT_16BIT, DT_32BIT, DT_64BIT, DT_FLOAT };
const char* DATATYPE_NAME[]  = { " 8-bit", "16-bit", "32-bit", "64-bit", "Floats" };

enum OPERATION { OP_ADD=0, OP_SUB, OP_MUL, OP_DIV }; 
const char* OPERATION_NAME[] = { "Addition", "Subtraction", "Multiplication", "Division" };

char cb[25];
bool intr, by_operation = true;
unsigned long start_us, end_us, test_us, count;

/*

Arduino Nano results (various junk attached):

[Addition]
 8-bit  : 0.15 us
16-bit  : 0.20 us
32-bit  : 0.32 us
64-bit  : 2.97 us
Floats  : 8.42 us

[Subtraction]
 8-bit  : 0.14 us
16-bit  : 0.20 us
32-bit  : 0.32 us
64-bit  : 2.97 us
Floats  : 8.34 us

[Multiplication]
 8-bit  : 0.33 us
16-bit  : 0.76 us
32-bit  : 5.05 us
64-bit  : 21.61 us
Floats  : 6.11 us

[Division]
 8-bit  : 5.25 us
16-bit  : 12.60 us
32-bit  : 36.48 us
64-bit  : 18.59 us
Floats  : 71.53 us

[ 8-bit]
Addition        : 0.14 us
Subtraction     : 0.14 us
Multiplication  : 0.33 us
Division        : 5.24 us

[16-bit]
Addition        : 0.21 us
Subtraction     : 0.20 us
Multiplication  : 0.77 us
Division        : 12.61 us

[32-bit]
Addition        : 0.33 us
Subtraction     : 0.33 us
Multiplication  : 5.05 us
Division        : 36.50 us

[64-bit]
Addition        : 2.98 us
Subtraction     : 2.97 us
Multiplication  : 21.62 us
Division        : 18.60 us

[Floats]
Addition        : 8.44 us
Subtraction     : 8.35 us
Multiplication  : 6.11 us
Division        : 71.57 us

*/

float while_us = 0, while_us_nointr = 0;
void estimate_while_us()
{

  //Estimate the time used by a while loop which increments a counter
  count = 0;
  test_us = TEST_US;
  start_us = micros();
  while (micros() - start_us < test_us) { count++; }
  end_us = micros();
  while_us = (float)(end_us - start_us) / (float)count;

  count = 0;
  test_us = TEST_US_NOINTR;
  noInterrupts();
  start_us = micros();
  while (micros() - start_us < test_us) { count++; }
  end_us = micros();
  interrupts();
  while_us_nointr = (float)(end_us - start_us) / (float)count;

  Serial.print("while overhead  : ");
  Serial.println(while_us);
  Serial.print("while overhead* : ");
  Serial.println(while_us_nointr);
}

template <class T> void benchmark(T a, T b, OPERATION operation, DATATYPE datatype)
{
  
  count = 0;
  test_us = TEST_US;
  if (!intr)
  {
    test_us = TEST_US_NOINTR;
    noInterrupts();
  }
  
  switch (operation)
  {
    case OP_ADD:
      start_us = micros();
      while (micros() - start_us < test_us) { a += b; count++; }
      end_us = micros();
      break;
      
    case OP_SUB:
      start_us = micros();
      while (micros() - start_us < test_us) { a -= b; count++; }
      end_us = micros();
      break;
      
    case OP_MUL:
      start_us = micros();
      while (micros() - start_us < test_us) { a *= b; count++; }
      end_us = micros();
      break;
      
    case OP_DIV:
      start_us = micros();
      while (micros() - start_us < test_us) { a /= b; count++; }
      end_us = micros();
      break;
  }
  
  if (!intr) interrupts();
  
  if (a == b) Serial.println(); //Prevent operations from being optimized away by the compiler
  
  cb[0] = 0;
  uint8_t i = 8;
  if (by_operation) strcat(cb, DATATYPE_NAME[datatype]);
  else
  {
    strcat(cb, OPERATION_NAME[operation]);
    i = 16;
  }
  if (!intr) strcat(cb, "*");
  while (strlen(cb) < i) strcat(cb, " ");
  strcat(cb, ": ");
    
  float score = ((float)(end_us - start_us) / (float)count) - (intr ? while_us : while_us_nointr);
  Serial.print(cb);
  Serial.print(score);
  Serial.println(" us");
  
}

void print_header(const char* cs)
{
  Serial.println();
  Serial.print("[");
  Serial.print(cs);
  Serial.println("]");
  intr = true;
}

void setup()
{
  Serial.begin(9600);
  estimate_while_us();
}

void loop()
{
  
  if (by_operation)
  {
  
    print_header(OPERATION_NAME[OP_ADD]);
    benchmark<DT8BIT>(0x00, 3, OP_ADD, DT_8BIT);
    benchmark<DT16BIT>(0x0000, 33, OP_ADD, DT_16BIT);
    benchmark<DT32BIT>(0x00000000UL, 333, OP_ADD, DT_32BIT);
    benchmark<DT64BIT>(0x0000000000000000ULL, 3333, OP_ADD, DT_64BIT);
    benchmark<DTFLOAT>(1234.56f, 3.45f, OP_ADD, DT_FLOAT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<DT8BIT>(0x00, 3, OP_ADD, DT_8BIT);
      benchmark<DT16BIT>(0x0000, 33, OP_ADD, DT_16BIT);
      benchmark<DT32BIT>(0x00000000UL, 333, OP_ADD, DT_32BIT);
      benchmark<DT64BIT>(0x0000000000000000ULL, 3333, OP_ADD, DT_64BIT);
      benchmark<DTFLOAT>(1234.56f, 3.45f, OP_ADD, DT_FLOAT);
    #endif
    
    print_header(OPERATION_NAME[OP_SUB]);
    benchmark<DT8BIT>(0xFF, 3, OP_SUB, DT_8BIT);
    benchmark<DT16BIT>(0xFFFF, 33, OP_SUB, DT_16BIT);
    benchmark<DT32BIT>(0xFFFFFFFFUL, 333, OP_SUB, DT_32BIT);
    benchmark<DT64BIT>(0xFFFFFFFFFFFFFFFFULL, 3333, OP_SUB, DT_64BIT);
    benchmark<DTFLOAT>(1234.56f, 3.45f, OP_SUB, DT_FLOAT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<DT8BIT>(0xFF, 3, OP_SUB, DT_8BIT);
      benchmark<DT16BIT>(0xFFFF, 33, OP_SUB, DT_16BIT);
      benchmark<DT32BIT>(0xFFFFFFFFUL, 333, OP_SUB, DT_32BIT);
      benchmark<DT64BIT>(0xFFFFFFFFFFFFFFFFULL, 3333, OP_SUB, DT_64BIT);
      benchmark<DTFLOAT>(1234.56f, 3.45f, OP_SUB, DT_FLOAT);
    #endif
    
    print_header(OPERATION_NAME[OP_MUL]);
    benchmark<DT8BIT>(0x01, 3, OP_MUL, DT_8BIT);
    benchmark<DT16BIT>(0x0101, 33, OP_MUL, DT_16BIT);
    benchmark<DT32BIT>(0x00010001UL, 333, OP_MUL, DT_32BIT);
    benchmark<DT64BIT>(0x0000000100000001ULL, 3333, OP_MUL, DT_64BIT);
    benchmark<DTFLOAT>(1234.56f, 1.56f, OP_MUL, DT_FLOAT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<DT8BIT>(0x01, 3, OP_MUL, DT_8BIT);
      benchmark<DT16BIT>(0x0101, 33, OP_MUL, DT_16BIT);
      benchmark<DT32BIT>(0x00010001UL, 333, OP_MUL, DT_32BIT);
      benchmark<DT64BIT>(0x0000000100000001ULL, 3333, OP_MUL, DT_64BIT);
      benchmark<DTFLOAT>(1234.56f, 1.56f, OP_MUL, DT_FLOAT);
    #endif
    
    print_header(OPERATION_NAME[OP_DIV]);
    benchmark<DT8BIT>(0xFF, 3, OP_DIV, DT_8BIT);
    benchmark<DT16BIT>(0xFFFF, 3, OP_DIV, DT_16BIT);
    benchmark<DT32BIT>(0xFFFFFFFFUL, 33, OP_DIV, DT_32BIT);
    benchmark<DT64BIT>(0xFFFFFFFFFFFFFFFFULL, 33, OP_DIV, DT_64BIT);
    benchmark<DTFLOAT>(1234.56f, 1.56f, OP_DIV, DT_FLOAT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<DT8BIT>(0xFF, 3, OP_DIV, DT_8BIT);
      benchmark<DT16BIT>(0xFFFF, 3, OP_DIV, DT_16BIT);
      benchmark<DT32BIT>(0xFFFFFFFFUL, 33, OP_DIV, DT_32BIT);
      benchmark<DT64BIT>(0xFFFFFFFFFFFFFFFFULL, 33, OP_DIV, DT_64BIT);
      benchmark<DTFLOAT>(1234.56f, 1.56f, OP_DIV, DT_FLOAT);
    #endif
    
  }
  else
  {
  
    print_header(DATATYPE_NAME[DT_8BIT]);
    benchmark<DT8BIT>(0x00, 3, OP_ADD, DT_8BIT);
    benchmark<DT8BIT>(0xFF, 3, OP_SUB, DT_8BIT);
    benchmark<DT8BIT>(0x01, 3, OP_MUL, DT_8BIT);
    benchmark<DT8BIT>(0xFF, 3, OP_DIV, DT_8BIT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<DT8BIT>(0x00, 3, OP_ADD, DT_8BIT);
      benchmark<DT8BIT>(0xFF, 3, OP_SUB, DT_8BIT);
      benchmark<DT8BIT>(0x01, 3, OP_MUL, DT_8BIT);
      benchmark<DT8BIT>(0xFF, 3, OP_DIV, DT_8BIT);
    #endif
    
    print_header(DATATYPE_NAME[DT_16BIT]);
    benchmark<DT16BIT>(0x0000, 33, OP_ADD, DT_16BIT);
    benchmark<DT16BIT>(0xFFFF, 33, OP_SUB, DT_16BIT);
    benchmark<DT16BIT>(0x0101, 33, OP_MUL, DT_16BIT);
    benchmark<DT16BIT>(0xFFFF, 3, OP_DIV, DT_16BIT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<DT16BIT>(0x0000, 33, OP_ADD, DT_16BIT);
      benchmark<DT16BIT>(0xFFFF, 33, OP_SUB, DT_16BIT);
      benchmark<DT16BIT>(0x0101, 33, OP_MUL, DT_16BIT);
      benchmark<DT16BIT>(0xFFFF, 3, OP_DIV, DT_16BIT);
    #endif
    
    print_header(DATATYPE_NAME[DT_32BIT]);
    benchmark<DT32BIT>(0x00000000UL, 333, OP_ADD, DT_32BIT);
    benchmark<DT32BIT>(0xFFFFFFFFUL, 333, OP_SUB, DT_32BIT);
    benchmark<DT32BIT>(0x00010001UL, 333, OP_MUL, DT_32BIT);
    benchmark<DT32BIT>(0xFFFFFFFFUL, 33, OP_DIV, DT_32BIT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<DT32BIT>(0x00000000UL, 333, OP_ADD, DT_32BIT);
      benchmark<DT32BIT>(0xFFFFFFFFUL, 333, OP_SUB, DT_32BIT);
      benchmark<DT32BIT>(0x00010001UL, 333, OP_MUL, DT_32BIT);
      benchmark<DT32BIT>(0xFFFFFFFFUL, 33, OP_DIV, DT_32BIT);
    #endif
    
    print_header(DATATYPE_NAME[DT_64BIT]);
    benchmark<DT64BIT>(0x0000000000000000ULL, 3333, OP_ADD, DT_64BIT);
    benchmark<DT64BIT>(0xFFFFFFFFFFFFFFFFULL, 3333, OP_SUB, DT_64BIT);
    benchmark<DT64BIT>(0x0000000100000001ULL, 3333, OP_MUL, DT_64BIT);
    benchmark<DT64BIT>(0xFFFFFFFFFFFFFFFFULL, 33, OP_DIV, DT_64BIT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<DT64BIT>(0x0000000000000000ULL, 3333, OP_ADD, DT_64BIT);
      benchmark<DT64BIT>(0xFFFFFFFFFFFFFFFFULL, 3333, OP_SUB, DT_64BIT);
      benchmark<DT64BIT>(0x0000000100000001ULL, 3333, OP_MUL, DT_64BIT);
      benchmark<DT64BIT>(0xFFFFFFFFFFFFFFFFULL, 33, OP_DIV, DT_64BIT);
    #endif
    
    print_header(DATATYPE_NAME[DT_FLOAT]);
    benchmark<DTFLOAT>(1234.56f, 3.45f, OP_ADD, DT_FLOAT);
    benchmark<DTFLOAT>(1234.56f, 3.45f, OP_SUB, DT_FLOAT);
    benchmark<DTFLOAT>(1234.56f, 1.56f, OP_MUL, DT_FLOAT);
    benchmark<DTFLOAT>(1234.56f, 1.56f, OP_DIV, DT_FLOAT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<DTFLOAT>(1234.56f, 3.45f, OP_ADD, DT_FLOAT);
      benchmark<DTFLOAT>(1234.56f, 3.45f, OP_SUB, DT_FLOAT);
      benchmark<DTFLOAT>(1234.56f, 1.56f, OP_MUL, DT_FLOAT);
      benchmark<DTFLOAT>(1234.56f, 1.56f, OP_DIV, DT_FLOAT);
    #endif
    
  }
  
  by_operation = !by_operation;
  if (by_operation)
  {
    #ifdef NO_INTERRUPTS
      Serial.println();
      Serial.println("* = without interrupts");
    #endif
    Serial.println();
    Serial.println();
    delay(30000);
  }
  
}

