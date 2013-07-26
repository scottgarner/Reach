/*

 Reach // Scott Garner 2013
 
 Most logic lifted from the Makey Makey firmware and rewritten just so I could grasp it.
 https://github.com/sparkfun/MaKeyMaKey/
 
 */

#include "settings.h"

//
// Indexes
//

int bitIndex = 0;
int byteIndex = 0;

//
// Timing
//

unsigned long loopTime = 0;
unsigned long previousTime = 0;

//
// Thresholds
//

int pressThreshold = int(13.2 + 1.5);
int releaseThreshold = int(13.2 - 1.5);

//
// Input Structure
//

typedef struct {
  int pinNumber;
  int ledPin;
  char keyDown;
  char keyUp;
  byte buffer[BUFFER_LENGTH];
  byte bufferSum;
  boolean pressed;
}
ReachInput;

ReachInput inputs[INPUT_COUNT];

//
// Setup
//

void setup() {

  Serial.begin(9600);

  // Initialze pins

  for (int i=0; i<INPUT_COUNT; i++)
  {
    // Input Pins

    pinMode(pinNumbers[i], INPUT);
    digitalWrite(pinNumbers[i], LOW);

    // Led Pins
    
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], HIGH);    

    // Setup Inputs

    inputs[i].pinNumber = pinNumbers[i];
    inputs[i].ledPin = ledPins[i];
    inputs[i].keyDown = keyDowns[i];
    inputs[i].keyUp = keyUps[i];

    inputs[i].pressed = false;
    for (int j=0; j<BUFFER_LENGTH; j++) {
      inputs[i].buffer[j] = 0;
    }    
  
  }  

  Keyboard.begin();
}

//
// Loop
//

void loop() {
  
  updateBuffers();
  updateStates();
  updateIndices();

  forceDelay();
}

//
// Check pins and update filter buffer
//

void updateBuffers() {

  for (int i=0; i<INPUT_COUNT; i++) {
    byte currentByte = inputs[i].buffer[byteIndex];

    int currentValue = digitalRead(inputs[i].pinNumber);
    currentValue = !currentValue;

    inputs[i].bufferSum -= (currentByte >> bitIndex) & 0x01;
    inputs[i].bufferSum += currentValue;    

    if (currentValue) {
      currentByte |= (1<<bitIndex);
    } 
    else {
      currentByte &= ~(1<<bitIndex);
    }    

    inputs[i].buffer[byteIndex] = currentByte;
  }

}

//
// Look at thresholds and send key presses if needed
//

void updateStates() {

  for (int i=0; i<INPUT_COUNT; i++) {

    if (inputs[i].pressed) {
      if (inputs[i].bufferSum < releaseThreshold) {  
        inputs[i].pressed = false;
        //Keyboard.print(inputs[i].keyUp);
        Serial.print(inputs[i].keyUp); 
        digitalWrite(inputs[i].ledPin, LOW);
      }

    } 
    else if (!inputs[i].pressed) {
      if (inputs[i].bufferSum > pressThreshold) {  // input becomes pressed
        inputs[i].pressed = true; 
        //Keyboard.print(inputs[i].keyDown);
        Serial.print(inputs[i].keyDown);
        digitalWrite(inputs[i].ledPin, HIGH);
      }
    }    

  }

}

//
// Update buffer indices
//

void updateIndices() {
  bitIndex++;

  if (bitIndex == 8) {
    bitIndex = 0;
    byteIndex++;
    if (byteIndex == BUFFER_LENGTH) {
      byteIndex = 0; 
    }
  }
}

//
// Make sure each loop lasts at least TARGET_LOOP_TIME
//

void forceDelay() {

  loopTime = micros() - previousTime;
  if (loopTime < TARGET_LOOP_TIME) {
    delayMicroseconds(TARGET_LOOP_TIME - loopTime);
  }

  previousTime = micros();

}




