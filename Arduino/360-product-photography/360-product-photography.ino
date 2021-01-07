/*
  Project: 360 Product Photograpy
  Author: Bahtiyar Bayram
  Date: 13.12.2020
*/
#include "SevSeg.h"
#include <EEPROM.h>

SevSeg sevseg; //Instantiate a seven segment controller object

/** the current address in the EEPROM (i.e. which byte we're going to write to next) **/
int divNumAddr = 0, turnOnTimeAddr = 1, turnOffTimeAddr = 2;

// defines pins numbers
const int enablePin = 2;
const int stepPin = 3;
const int dirPin = 4;
const int startButton = 16;
const int divideButton = 17;
const int shutter = 18;
int divNum = 0, turnOnTime = 2, turnOffTime = 15;
int divArr[10] = {1, 2, 4, 5, 8, 10, 20, 25, 40, 50};

void setup() {
  // Sets the two pins as Outputs
  pinMode(enablePin, OUTPUT);
  // Disable Step Motor
  digitalWrite(enablePin, HIGH);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(startButton, INPUT);
  pinMode(divideButton, INPUT);
  pinMode(shutter, OUTPUT);

  byte numDigits = 2;
  byte digitPins[] = {14, 15};
  byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
  bool resistorsOnSegments = true; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_ANODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = true; // Use 'true' if your decimal point doesn't exist or isn't connected

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
               updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(50);

  //Serial.begin(9600);
  digitalWrite(dirPin, HIGH); // Enables the motor to move in a particular direction
  // Turn off shutter
  digitalWrite(shutter, LOW);

  // Read stored datas
  divNum = EEPROM.read(divNumAddr);
  turnOnTime = EEPROM.read(turnOnTimeAddr);
  turnOffTime = EEPROM.read(turnOffTimeAddr);

  // Set turnOnTime
  if (digitalRead(startButton)) {
    sevseg.setNumber(turnOnTime);
    while (digitalRead(startButton)) {
      sevseg.refreshDisplay();
      delay(10);
    }
    while (!digitalRead(divideButton)) {
      if (digitalRead(startButton)) {
        // Increase turnOnTime
        turnOnTime++;
        if (turnOnTime == 99) {
          turnOnTime = 1;
        }
        EEPROM.write(turnOnTimeAddr, turnOnTime);
        delay(200);
      }
      sevseg.setNumber(turnOnTime);
      sevseg.refreshDisplay(); // Must run repeatedly  }
    }
  }
  // Set turnOffTime
  else if (digitalRead(divideButton)) {
    sevseg.setNumber(turnOffTime);
    while (digitalRead(divideButton)) {
      sevseg.refreshDisplay();
      delay(10);
    }
    while (!digitalRead(startButton)) {
      if (digitalRead(divideButton)) {
        // Increase turnOffTime
        turnOffTime++;
        if (turnOffTime == 99) {
          turnOffTime = 1;
        }
        EEPROM.write(turnOffTimeAddr, turnOffTime);
        delay(200);
      }
      sevseg.setNumber(turnOffTime);
      sevseg.refreshDisplay(); // Must run repeatedly  }
    }
  }

  sevseg.setNumber(divArr[divNum]);
}

void loop() {

  // If divideButton is pressed
  if (digitalRead(divideButton)) {
    // Wait for release
    while (digitalRead(divideButton));
    // Increase divNum
    divNum++;
    if (divNum == 10) {
      divNum = 0;
    }
    sevseg.setNumber(divArr[divNum]);
    sevseg.refreshDisplay(); // Must run repeatedly
    /***
      Write the value to the appropriate byte of the EEPROM.
      these values will remain there when the board is
      turned off.
    ***/

    EEPROM.write(divNumAddr, divNum);

    delay(100);
  }

  // If startButton is pressed
  if (digitalRead(startButton)) {
    // Delay for a 1 second
    delay(2000);
    // If divideBuuton is pressed after startButton is pressed
    if (digitalRead(divideButton)) {
      // Enable Step Motor
      digitalWrite(enablePin, LOW);
      while (1) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(15000);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(15000);
        sevseg.setChars("--");
        sevseg.refreshDisplay();

        if (digitalRead(startButton)) {
          // Disable Step Motor
          digitalWrite(enablePin, HIGH);
          sevseg.setNumber(divArr[divNum]);
          break;
        }
      }
    }
    else {
      sevseg.blank();
      // Enable Step Motor
      digitalWrite(enablePin, LOW);
      delay(1);

      // First time SHUT
      // Turn on shutter
      digitalWrite(shutter, HIGH);
      sevseg.setChars("oo");
      sevseg.refreshDisplay();
      delay(turnOnTime * 100);
      // Turn off shutter
      digitalWrite(shutter, LOW);
      sevseg.setChars("--");
      sevseg.refreshDisplay();
      delay(turnOffTime * 100);

      for (int i = 0; i < divArr[divNum] - 1; i++) {
        // Makes 200 pulses for making one full cycle rotation
        for (int x = 0; x < (200 / divArr[divNum]); x++) {
          digitalWrite(stepPin, HIGH);
          delay(30);
          digitalWrite(stepPin, LOW);
          delay(30);
        }
        // SHUTTER
        // Turn on shutter
        digitalWrite(shutter, HIGH);
        sevseg.setChars("oo");
        sevseg.refreshDisplay();
        delay(turnOnTime * 100);
        // Turn off shutter
        digitalWrite(shutter, LOW);
        sevseg.setChars("--");
        sevseg.refreshDisplay();
        delay(turnOffTime * 100);
      }
      // Go to the start position
      for (int x = 0; x < (200 / divArr[divNum]); x++) {
        digitalWrite(stepPin, HIGH);
        delay(30);
        digitalWrite(stepPin, LOW);
        delay(30);
      }

      // Disable Step Motor
      digitalWrite(enablePin, HIGH);
      sevseg.setNumber(divArr[divNum]);
    }
    delay(2000);
  }
  /*
    Serial.print(divNum);
    Serial.print(" - ");
    Serial.println(digitalRead(divideButton));
  */
  sevseg.refreshDisplay(); // Must run repeatedly
}
