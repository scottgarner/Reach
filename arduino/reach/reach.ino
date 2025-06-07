#include <MIDIUSB.h>

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
  byte note;
  byte buffer[BUFFER_LENGTH];
  byte bufferSum;
  boolean pressed;
} ReachInput;

ReachInput inputs[INPUT_COUNT] = {
    {A6, 60}, {A7, 62}, {A8, 64}, {A9, 65}, {A0, 67}, {A1, 69}, {A2, 71}, {A3, 72}};

uint8_t channel = 0;

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

  channel = (msb << 1) | lsb;
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
        byte pitch = inputs[i].note;
        byte velocity = 127;

        if (inputs[i].pressed &&
            inputs[i].bufferSum < releaseThreshold)
        {
          inputs[i].pressed = false;

          midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
          MidiUSB.sendMIDI(noteOff);
          MidiUSB.flush();
        }
        else if (!inputs[i].pressed &&
                 inputs[i].bufferSum > pressThreshold)
        {
          inputs[i].pressed = true;

          midiEventPacket_t noteOn = {0x09, (uint8_t)(0x90) | channel, pitch, velocity};
          MidiUSB.sendMIDI(noteOn);
          MidiUSB.flush();
        }
      }
    }

    lastSample = now;
  }
}