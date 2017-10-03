/********************************************************************************
* Smiley game for Arduino - Copyright © 2017 Torben Bruchhaus                   *
*                                                                               *
* Permission is hereby granted, free of charge, to any person obtaining a copy  *
* of this software and associated documentation files (the "Software"), to deal *
* in the Software without restriction, including without limitation the rights  *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell     *
* copies of the Software, and to permit persons to whom the Software is         *
* furnished to do so, subject to the following conditions:                      *
*                                                                               *
* The above copyright notice and this permission notice shall be included in    *
* all copies or substantial portions of the Software.                           *
*                                                                               *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,      *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE   *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER        *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN     *
* THE SOFTWARE.                                                                 *
********************************************************************************/

/*

    WIRINGS
    =======
    
    DISPLAY
    -------
    VSS    => GND
    VDD    => 5v
    V0     => 10K Ohm Potentiometer P2 (Pot P1 => GND, Pot P3 => 5v)
    RS     => P13
    RW     => GND
    E      => P12
    D4..D7 => A0..A3
    A      => 200 Ohm => 5v
    K      => GND

    PIEZO / BUZZER
    --------------
    - => GND
    + => 300 Ohm => P5 (Use less resistance to get louder sound)

    BUTTON
    ------
    1 => 10K Ohm => GND
    2 => P2

    
    Pn = Pin number n on Arduino
    
*/


#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(13,12, A0,A1,A2,A3);

const int SOUND_PIN = 5; //pin used to generate sound
const int BUTTON_PIN = 2; //pin used to read the button
const unsigned int SOUND_LENGTH = 10; //length in ms of sounds
const unsigned int SOUND_MAD = 400; //frequency for mad enemy
const unsigned int SOUND_SAD = 600; //frequency for sad enemy
const unsigned int SOUND_HAPPY = 200; //frequency for happy enemy
const unsigned int SOUND_SHOT = 800; //frequency for a shot
const unsigned int BUTTON_DELAY = 25; //minimum time to elapse between button clicks (prevent flicker, double clicks etc)
const char MSG_GAME_OVER[17] = " GAME IS OVER!! "; //message to display in upper line when game is over
const char MSG_YOUR_SCORE[14] = "Your score = "; //message to display the achieved score
const char MSG_HIGH_SCORE[14] = "High score = "; //message to display the highest schieved score
const char MSG_START_GAME[17] = "CLICK TO START.."; //message to display prior to game start

unsigned long lastMove = 0; //millis for last move
unsigned long lastButtonRead = 0; //millis for last button click
unsigned long loopMillis = 0; //the global millis used
int buttonState = HIGH; //current button state
byte enemyPos = 0; //position of the enemy
byte enemyType = 0; //type of enemy
bool infoShowScore = true; //true if score is showing
bool gameOver = false; //true if the game is over
bool gameStarted = false; //true if the game has been started
unsigned int enemyDelay = 500; //current delay between movement of enemy
unsigned int gamePoints = 0; //current amount of points achieved
unsigned int pointsTotal = 1; //current amount of points possible
unsigned int lastSpeedChange = 0; //interval for last change in speed / level
unsigned int highScore = 0; //highest score achieved (stred to EEPROM)

const byte CHAR_LAUNCHER = 0; //index of the launcher char
byte charLauncher[8] = { //bitmap for the launcher char
  B11000,
  B01100,
  B00110,
  B00011,
  B00011,
  B00110,
  B01100,
  B11000,
};

const byte CHAR_GUN = 1; //index of the gun char
const byte GUN_POS = 9; //fixed position of the gun char in the display
byte charGun[8] = { //bitmap for the gun char
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B11111,
  B11111,
};

const byte CHAR_MAD = 2; //index of mad enemy char
byte charMad[8] = { //bitmap for mad enemy char
  B10001,
  B11011,
  B00000,
  B00100,
  B00100,
  B00000,
  B11111,
  B10001,
};

const byte CHAR_HAPPY = 3; //index of happy enemy char
byte charHappy[8] = { //bitmap for happy enemy char
  B11011,
  B10001,
  B00100,
  B00100,
  B00000,
  B10001,
  B10001,
  B01110,
};

const byte CHAR_SAD = 4; //index of sad enemy char
byte charSad[8] = { //bitmap for sad enemy char
  B11011,
  B00000,
  B00100,
  B00100,
  B00000,
  B01110,
  B10001,
  B10001,
};

bool doShoot() { //return true if shot (button click) is registred

  //read the button state
  int btn = digitalRead(BUTTON_PIN);

  //check if changed
  if (buttonState != btn) {

    //check debounce
    if (loopMillis - lastButtonRead < BUTTON_DELAY) return false;
    lastButtonRead = loopMillis;
    
    //update state
    buttonState = btn;

    //check if LOW
    if (buttonState == LOW)
    {

      //make a tone
      tone(SOUND_PIN, SOUND_SHOT, SOUND_LENGTH * 2);

      //return true for click registered
      return true;
      
    }
    
  }

  //return false
  return false;
  
}

void doMove() {
  
  //increase the position of the enemy
  enemyPos++;
  
  if (enemyPos == 16) {
    
    //enemy has passed the edge, clear it
    lcd.setCursor(15, 0);
    lcd.print(" ");

    //reset enemy to initial position and type
    enemyPos = 0;
    enemyType = CHAR_MAD;
    
    //check if the game speed must be increased
    if ((gamePoints != lastSpeedChange) && (gamePoints % 3 == 0)) {

      //yep, decrease delay between movements
      enemyDelay -= 50;
      lastSpeedChange = gamePoints;
      
    }
    
    //check if there has been three misses
    if (pointsTotal - gamePoints >= 3) {

      //yep, game is over
      gameOver = true;
      
      //do we have a new high score?
      if (gamePoints > highScore) {

        //yep, set it and store it
        highScore = gamePoints;
        EEPROM.put(0, highScore);
        
      }

      //update last move, now used to switch between informations
      lastMove = loopMillis;//millis();
      
    }

    //not threee misses, the game continues
    else pointsTotal++;

    //update info
    doInfo();

    //exit
    return;
    
  } else if (enemyPos > 1) {

    //set cursor to previous enemy position and clear
    lcd.setCursor(enemyPos - 1, 0);
    lcd.print(" ");

    //if mad and passed gun position, set to happy
    if ((enemyType == CHAR_MAD) && (enemyPos > GUN_POS)) enemyType = CHAR_HAPPY;
    
  } else lcd.setCursor(enemyPos, 0); //set cursor to current enemy position (nothing to clear)

  //show the enemy
  lcd.write(enemyType);
  
  //make a sound for the current enemy type
  if (enemyType == CHAR_MAD) tone(SOUND_PIN, SOUND_MAD, SOUND_LENGTH);
  else if (enemyType == CHAR_SAD) tone(SOUND_PIN, SOUND_SAD, SOUND_LENGTH);
  else tone(SOUND_PIN, SOUND_HAPPY, SOUND_LENGTH);
  
}

void doInfo() {

  //display information
  
  if (gameOver) {
    
    //clear display and write "GAME OVER" in the first line
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(MSG_GAME_OVER);

    //set cursor to second line
    lcd.setCursor(0, 1);
    
    if (infoShowScore) {

      //write the score achieved for the played game
      lcd.print(MSG_YOUR_SCORE);
      lcd.print(gamePoints);
      
    } else {

      //write the top score
      lcd.print(MSG_HIGH_SCORE);
      lcd.print(highScore);

      //check for button press
      if (doShoot())
      {

        //button is pressed, reset high score and store it
        highScore = 0;
        EEPROM.put(0, highScore);
        
      }
      
    }

    //toggle the displayed information
    infoShowScore = !infoShowScore;
    
  } else {

    //in game display of actual score and total points played
    lcd.setCursor(0, 1);
    lcd.print(gamePoints);
    lcd.print("/");
    lcd.print(pointsTotal);
    
  }
  
}

void setup() {
  
  //get the high score from EEPROM
  EEPROM.get(0, highScore);
  
  //prepare button pin
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH); //init pullup resistor
  
  //create chars for the display
  lcd.createChar(CHAR_LAUNCHER, charLauncher);
  lcd.createChar(CHAR_MAD, charMad);
  lcd.createChar(CHAR_HAPPY, charHappy);
  lcd.createChar(CHAR_SAD, charSad);
  lcd.createChar(CHAR_GUN, charGun);

  //initialize the display driver
  lcd.begin(16,2);

  //ask user for click to start
  lcd.setCursor(0, 0);
  lcd.print(MSG_START_GAME);

  //show current high score
  lcd.setCursor(0, 1);
  lcd.print(MSG_HIGH_SCORE);
  lcd.print(highScore);

  //set enemy type
  enemyType = CHAR_MAD;

  //show info
  //doInfo();
  
}

void loop() {

  //get the loop millis
  loopMillis = millis();
  
  if (!gameStarted) {
    
    if (doShoot()) {
      
      //if button is pressed, start the game
      gameStarted = true;

      //show launcher and gun
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.write(CHAR_LAUNCHER);
      lcd.setCursor(GUN_POS, 1);
      lcd.write(CHAR_GUN);

      //show info
      doInfo();
    
    }
    
    //exit
    return;
      
  }
  
  //is the game over?
  if (gameOver) {

    //yes, check if we must update info display
    if (loopMillis - lastMove > 2000) {

      //yes, show info and store millis of last update
      doInfo();
      lastMove = loopMillis;
      
    }

    //exit
    return;
    
  }
  
  //is a shot fired?
  if (doShoot()) {

    //yes, check for a hit
    if ((enemyPos == GUN_POS) && (enemyType == CHAR_MAD)) {

      //hit registred, increase points and change enemy to sad
      gamePoints++;
      enemyType = CHAR_SAD;

      //update info to show new score
      doInfo();
      
    }
    
  }
  
  //do we need to update enemy position?
  if (loopMillis - lastMove >= enemyDelay) {

    //yes, perform a move and store millis for last move
    doMove();
    lastMove = loopMillis;
    
  }
  
}
