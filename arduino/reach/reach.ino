/*

 Reach // Scott Garner 2013
 
 Most logic lifted from the Makey Makey firmware and rewritten just so I could grasp it.
 https://github.com/sparkfun/MaKeyMaKey/
 
 */




#define INPUT_COUNT 16
#define BUFFER_LENGTH 3
#define TARGET_LOOP_TIME 744

int pressThreshold = int(13.2 + 1.5);
int releaseThreshold = int(13.2 - 1.5);

// Indexes

int bitIndex = 0;
int byteIndex = 0;

// Timing

unsigned long loopTime = 0;
unsigned long previousTime = 0;

// Input Structure

typedef struct {
  int pinNumber;
  char keyDown;
  char keyUp;
  byte buffer[BUFFER_LENGTH];
  byte bufferSum;
  boolean pressed;
}
ReachInput;

ReachInput inputs[INPUT_COUNT];

// 

int pinNumbers[INPUT_COUNT] = {
  22,21,20,19,18,2,3,4,5,6,7,8,9,10,11,12};
char keyDowns[INPUT_COUNT] = {
  'Q','W','E','R','T','Y','U','I','A','S','D','F','G','H','J','K'};
char keyUps[INPUT_COUNT] = {
  'q','w','e','r','t','y','u','i','a','s','d','f','g','h','j','k'};

// Functions

void setup() {

  Serial.begin(9600);

  // Initialze pins

  for (int i=0; i<INPUT_COUNT; i++)
  {
    pinMode(pinNumbers[i], INPUT);
    digitalWrite(pinNumbers[i], LOW);

    // Setup Inputs

    inputs[i].pinNumber = pinNumbers[i];
    inputs[i].keyDown = keyDowns[i];
    inputs[i].keyUp = keyUps[i];

    inputs[i].pressed = false;
    for (int j=0; j<BUFFER_LENGTH; j++) {
      inputs[i].buffer[j] = 0;
    }    

  }  

  Keyboard.begin();
}

void loop() {
  
  updateBuffers();
  updateStates();
  updateIndices();

  forceDelay();
}

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

void updateStates() {

  for (int i=0; i<INPUT_COUNT; i++) {

    if (inputs[i].pressed) {
      if (inputs[i].bufferSum < releaseThreshold) {  
        inputs[i].pressed = false;
        //Keyboard.print(inputs[i].keyUp);
        Serial.print(inputs[i].keyUp); 
      }

    } 
    else if (!inputs[i].pressed) {
      if (inputs[i].bufferSum > pressThreshold) {  // input becomes pressed
        inputs[i].pressed = true; 
        //Keyboard.print(inputs[i].keyDown);
        Serial.print(inputs[i].keyDown);

      }
    }    

  }

}

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

void forceDelay() {

  loopTime = micros() - previousTime;
  if (loopTime < TARGET_LOOP_TIME) {
    delayMicroseconds(TARGET_LOOP_TIME - loopTime);
  }

  previousTime = micros();

}




