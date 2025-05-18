#include <MIDIUSB.h>

#define INPUT_COUNT 8
#define BUFFER_LENGTH 64
#define SAMPLE_INTERVAL 250

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
    {4, 60}, {6, 62}, {8, 64}, {9, 65}, {A0, 67}, {A1, 69}, {A2, 71}, {A3, 72}};

void setup()
{
  Serial.begin(115200);

  for (int i = 0; i < INPUT_COUNT; i++)
  {
    pinMode(inputs[i].pinNumber, INPUT);
  }
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
      byte channel = 0;
      byte pitch = 60;
      byte velocity = 127;

      for (int i = 0; i < INPUT_COUNT; i++)
      {
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

          midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
          MidiUSB.sendMIDI(noteOn);
          MidiUSB.flush();
        }
      }
    }

    lastSample = now;
  }
}