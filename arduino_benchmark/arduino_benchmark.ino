/*
  
  Copyright © 2018 - Torben Bruchhaus
  https://github.com/bswebdk/arduino/arduino_benchmark
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

//Uncomment the following line to do additional benchmarks with interrupts disabled:
//#define NO_INTERRUPTS

enum DATATYPE  { DT_8BIT=0, DT_16BIT, DT_32BIT, DT_FLOAT };
const char* DATATYPE_NAME[4]  = { " 8-bit", "16-bit", "32-bit", "Floats" };

enum OPERATION { OP_ADD=0, OP_SUB, OP_MUL, OP_DIV }; 
const char* OPERATION_NAME[4] = { "Addition", "Subtraction", "Multiplication", "Division" };

char cb[25];
bool intr, by_operation = true;
unsigned long start_us, end_us, count;

/*

Arduino Nano results (various junk attached):

[Addition]
 8-bit  : 0.17 us
16-bit  : 0.26 us
32-bit  : 0.36 us
Floats  : 6.78 us

[Subtraction]
 8-bit  : 0.14 us
16-bit  : 0.20 us
32-bit  : 0.33 us
Floats  : 6.98 us

[Multiplication]
 8-bit  : 0.33 us
16-bit  : 0.77 us
32-bit  : 5.08 us
Floats  : 8.61 us

[Division]
 8-bit  : 5.26 us
16-bit  : 12.69 us
32-bit  : 38.35 us
Floats  : 30.27 us

[ 8-bit]
Addition        : 0.14 us
Subtraction     : 0.14 us
Multiplication  : 0.33 us
Division        : 5.26 us

[16-bit]
Addition        : 0.20 us
Subtraction     : 0.20 us
Multiplication  : 0.77 us
Division        : 12.76 us

[32-bit]
Addition        : 0.33 us
Subtraction     : 0.33 us
Multiplication  : 5.08 us
Division        : 38.35 us

[Floats]
Addition        : 6.78 us
Subtraction     : 6.98 us
Multiplication  : 8.61 us
Division        : 30.27 us

*/

float while_us = 0, while_us_nointr = 0;
void estimate_while_us()
{

  //Estimate the time used by a while loop which increments a counter
  count = 0;
  start_us = micros();
  while (micros() - start_us < 1000) count++;
  end_us = micros();
  while_us = (float)(end_us - start_us) / (float)count;

  count = 0;
  noInterrupts();
  start_us = micros();
  while (micros() - start_us < 1000) count++;
  end_us = micros();
  interrupts();
  while_us_nointr = (float)(end_us - start_us) / (float)count;

  /*Serial.print("while overhead  : ");
  Serial.println(while_us);
  Serial.print("while overhead*  : ");
  Serial.println(while_us_nointr);
  Serial.println();*/
}

template <class T> void benchmark(T a, T b, OPERATION operation, DATATYPE datatype)
{
  
  count = 0;
  if (!intr) noInterrupts();
  
  switch (operation)
  {
    case OP_ADD:
      start_us = micros();
      while (micros() - start_us < 1000) { a += b; count++; }
      end_us = micros();
      break;
      
    case OP_SUB:
      start_us = micros();
      while (micros() - start_us < 1000) { a -= b; count++; }
      end_us = micros();
      break;
      
    case OP_MUL:
      start_us = micros();
      while (micros() - start_us < 1000) { a *= b; count++; }
      end_us = micros();
      break;
      
    case OP_DIV:
      start_us = micros();
      while (micros() - start_us < 1000) { a /= b; count++; }
      end_us = micros();
      break;
  }
  
  if (!intr) interrupts();
  
  if (a == b) Serial.println(); //Prevent results from being optimized away by the compiler
  
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
    
  //float score = ((float)ms / 10.0f) / (float)loop_size;
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
    benchmark<unsigned char>(0x00, 3, OP_ADD, DT_8BIT);
    benchmark<unsigned int>(0x0000, 3, OP_ADD, DT_16BIT);
    benchmark<unsigned long>(0x00000000UL, 3, OP_ADD, DT_32BIT);
    benchmark<float>(1234.56f, 3.45f, OP_ADD, DT_FLOAT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<unsigned char>(0x00, 3, OP_ADD, DT_8BIT);
      benchmark<unsigned int>(0x0000, 3, OP_ADD, DT_16BIT);
      benchmark<unsigned long>(0x00000000UL, 3, OP_ADD, DT_32BIT);
      benchmark<float>(1234.56f, 3.45f, OP_ADD, DT_FLOAT);
    #endif
    
    print_header(OPERATION_NAME[OP_SUB]);
    benchmark<unsigned char>(0xFF, 3, OP_SUB, DT_8BIT);
    benchmark<unsigned int>(0xFFFF, 3, OP_SUB, DT_16BIT);
    benchmark<unsigned long>(0xFFFFFFFFUL, 3, OP_SUB, DT_32BIT);
    benchmark<float>(1234.56f, 3.45f, OP_SUB, DT_FLOAT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<unsigned char>(0xFF, 3, OP_SUB, DT_8BIT);
      benchmark<unsigned int>(0xFFFF, 3, OP_SUB, DT_16BIT);
      benchmark<unsigned long>(0xFFFFFFFFUL, 3, OP_SUB, DT_32BIT);
      benchmark<float>(1234.56f, 3.45f, OP_SUB, DT_FLOAT);
    #endif
    
    print_header(OPERATION_NAME[OP_MUL]);
    benchmark<unsigned char>(0x01, 3, OP_MUL, DT_8BIT);
    benchmark<unsigned int>(0x0001, 3, OP_MUL, DT_16BIT);
    benchmark<unsigned long>(0x00000001UL, 3, OP_MUL, DT_32BIT);
    benchmark<float>(1234.56f, 1.56f, OP_MUL, DT_FLOAT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<unsigned char>(0x01, 3, OP_MUL, DT_8BIT);
      benchmark<unsigned int>(0x0001, 3, OP_MUL, DT_16BIT);
      benchmark<unsigned long>(0x00000001UL, 3, OP_MUL, DT_32BIT);
      benchmark<float>(1234.56f, 1.56f, OP_MUL, DT_FLOAT);
    #endif
    
    print_header(OPERATION_NAME[OP_DIV]);
    benchmark<unsigned char>(0xFF, 3, OP_DIV, DT_8BIT);
    benchmark<unsigned int>(0xFFFF, 3, OP_DIV, DT_16BIT);
    benchmark<unsigned long>(0xFFFFFFFFUL, 3, OP_DIV, DT_32BIT);
    benchmark<float>(1234.56f, 1.56f, OP_DIV, DT_FLOAT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<unsigned char>(0xFF, 3, OP_DIV, DT_8BIT);
      benchmark<unsigned int>(0xFFFF, 3, OP_DIV, DT_16BIT);
      benchmark<unsigned long>(0xFFFFFFFFUL, 3, OP_DIV, DT_32BIT);
      benchmark<float>(1234.56f, 1.56f, OP_DIV, DT_FLOAT);
    #endif
    
  }
  else
  {
  
    print_header(DATATYPE_NAME[DT_8BIT]);
    benchmark<unsigned char>(0x00, 3, OP_ADD, DT_8BIT);
    benchmark<unsigned char>(0xFF, 3, OP_SUB, DT_8BIT);
    benchmark<unsigned char>(0x01, 3, OP_MUL, DT_8BIT);
    benchmark<unsigned char>(0xFF, 3, OP_DIV, DT_8BIT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<unsigned char>(0x00, 3, OP_ADD, DT_8BIT);
      benchmark<unsigned char>(0xFF, 3, OP_SUB, DT_8BIT);
      benchmark<unsigned char>(0x01, 3, OP_MUL, DT_8BIT);
      benchmark<unsigned char>(0xFF, 3, OP_DIV, DT_8BIT);
    #endif
    
    print_header(DATATYPE_NAME[DT_16BIT]);
    benchmark<unsigned int>(0x0000, 3, OP_ADD, DT_16BIT);
    benchmark<unsigned int>(0xFFFF, 3, OP_SUB, DT_16BIT);
    benchmark<unsigned int>(0x0001, 3, OP_MUL, DT_16BIT);
    benchmark<unsigned int>(0xFFFF, 3, OP_DIV, DT_16BIT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<unsigned int>(0x0000, 3, OP_ADD, DT_16BIT);
      benchmark<unsigned int>(0xFFFF, 3, OP_SUB, DT_16BIT);
      benchmark<unsigned int>(0x0001, 3, OP_MUL, DT_16BIT);
      benchmark<unsigned int>(0xFFFF, 3, OP_DIV, DT_16BIT);
    #endif
    
    print_header(DATATYPE_NAME[DT_32BIT]);
    benchmark<unsigned long>(0x00000000UL, 3, OP_ADD, DT_32BIT);
    benchmark<unsigned long>(0xFFFFFFFFUL, 3, OP_SUB, DT_32BIT);
    benchmark<unsigned long>(0x00000001UL, 3, OP_MUL, DT_32BIT);
    benchmark<unsigned long>(0xFFFFFFFFUL, 3, OP_DIV, DT_32BIT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<unsigned long>(0x00000000UL, 3, OP_ADD, DT_32BIT);
      benchmark<unsigned long>(0xFFFFFFFFUL, 3, OP_SUB, DT_32BIT);
      benchmark<unsigned long>(0x00000001UL, 3, OP_MUL, DT_32BIT);
      benchmark<unsigned long>(0xFFFFFFFFUL, 3, OP_DIV, DT_32BIT);
    #endif
    
    print_header(DATATYPE_NAME[DT_FLOAT]);
    benchmark<float>(1234.56f, 3.45f, OP_ADD, DT_FLOAT);
    benchmark<float>(1234.56f, 3.45f, OP_SUB, DT_FLOAT);
    benchmark<float>(1234.56f, 1.56f, OP_MUL, DT_FLOAT);
    benchmark<float>(1234.56f, 1.56f, OP_DIV, DT_FLOAT);
    #ifdef NO_INTERRUPTS
      intr = false;
      benchmark<float>(1234.56f, 3.45f, OP_ADD, DT_FLOAT);
      benchmark<float>(1234.56f, 3.45f, OP_SUB, DT_FLOAT);
      benchmark<float>(1234.56f, 1.56f, OP_MUL, DT_FLOAT);
      benchmark<float>(1234.56f, 1.56f, OP_DIV, DT_FLOAT);
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
