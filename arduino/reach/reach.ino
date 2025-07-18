#include <MIDIUSB.h>

#define BOARD_COUNT 3
#define INPUT_COUNT 8
#define BUFFER_LENGTH 64
#define SAMPLE_INTERVAL 250

const uint8_t boardSelectA = 2;
const uint8_t boardSelectB = 3;

int sampleIndex = 0;
unsigned long lastSample = 0;

float pressThreshold = BUFFER_LENGTH * 0.65;
float releaseThreshold = BUFFER_LENGTH * 0.5;

typedef struct
{
  byte pinNumber;
  byte buffer[BUFFER_LENGTH];
  byte bufferSum;
  boolean pressed;
} ReachInput;

typedef struct
{
  byte channel;
  byte note;
} ReachOutput;

ReachInput inputs[INPUT_COUNT] = {
    {A6}, {A7}, {A8}, {A9}, {A0}, {A1}, {A2}, {A3}};

ReachOutput outputs[BOARD_COUNT][INPUT_COUNT] = {
    // Panel A
    {
        {0, 60}, // Star A
        {0, 64}, // Star B
        {0, 67}, // Star C
        {0, 72}, // Star D
        {0, 76}, // Star E
        {0, 79}, // Star F
        {1, 60}, // Star G
        {1, 64}, // Star H
    },
    // Panel B
    {
        {1, 67}, // Star A
        {1, 72}, // Star B
        {1, 76}, // Star C
        {1, 79}, // Star D
        {2, 60}, // Star E
        {2, 64}, // Star F
        {2, 67}, // Star G
        {2, 72}, // Star H
    },
    // Panel C C
    {
        {2, 74}, // Star A
        {2, 79}, // Star B
        {3, 60}, // Star C
        {3, 64}, // Star D
        {3, 67}, // Star E
        {3, 72}, // Star F
        {3, 76}, // Star G
        {3, 79}, // Star H
    },
};

uint8_t board = 0;

void setup()
{
  Serial.begin(115200);

  for (int i = 0; i < INPUT_COUNT; i++)
  {
    pinMode(inputs[i].pinNumber, INPUT);
  }

  pinMode(boardSelectA, INPUT_PULLUP);
  pinMode(boardSelectB, INPUT_PULLUP);

  int lsb = (digitalRead(boardSelectA) == LOW) ? 1 : 0;
  int msb = (digitalRead(boardSelectB) == LOW) ? 1 : 0;

  board = (msb << 1) | lsb;
}

void loop()
{
  unsigned long now = micros();

  if (now - lastSample >= SAMPLE_INTERVAL)
  {
    // Update buffers.
    {
      for (int i = 0; i < INPUT_COUNT; i++)
      {
        byte previousValue = inputs[i].buffer[sampleIndex];
        byte newValue = !digitalRead(inputs[i].pinNumber);

        inputs[i].bufferSum -= previousValue;
        inputs[i].bufferSum += newValue;

        inputs[i].buffer[sampleIndex] = newValue;
      }

      sampleIndex = (sampleIndex + 1) % BUFFER_LENGTH;
    }

    // Update states.
    {
      for (int i = 0; i < INPUT_COUNT; i++)
      {
        byte channel = outputs[board][i].channel;
        byte note = outputs[board][i].note;

        byte velocity = 127;

        if (inputs[i].pressed &&
            inputs[i].bufferSum < releaseThreshold)
        {
          inputs[i].pressed = false;

          midiEventPacket_t noteOff = {0x08, 0x80 | channel, note, velocity};
          MidiUSB.sendMIDI(noteOff);
          MidiUSB.flush();
        }
        else if (!inputs[i].pressed &&
                 inputs[i].bufferSum > pressThreshold)
        {
          inputs[i].pressed = true;

          midiEventPacket_t noteOn = {0x09, (uint8_t)(0x90) | channel, note, velocity};
          MidiUSB.sendMIDI(noteOn);
          MidiUSB.flush();
        }
      }
    }

    lastSample = now;
  }
}