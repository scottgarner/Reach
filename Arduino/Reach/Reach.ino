#include <MIDIUSB.h>

#define BOARD_COUNT 3
#define INPUT_COUNT 8

#define SAMPLE_COUNT 32
#define SAMPLE_INTERVAL 1

#define PRESS_THRESHOLD (SAMPLE_COUNT * 0.7)
#define RELEASE_THRESHOLD (SAMPLE_COUNT * 0.45)

#define NOTE_TIMEOUT 5000

const uint8_t boardSelectA = 2;
const uint8_t boardSelectB = 3;

int sampleIndex = 0;
unsigned long lastSample = 0;

typedef struct
{
  byte pinNumber;
  byte buffer[SAMPLE_COUNT];
  byte bufferSum;
  boolean pressed;
  boolean noteOn;
  unsigned long noteOnTime;
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

  // Force clear all notes.
  {
    for (int channel = 0; channel < 4; channel++)
    {
      midiEventPacket_t allNotesOff = {0x0B, 0xB0 | channel, 123, 0};
      MidiUSB.sendMIDI(allNotesOff);
    }
    MidiUSB.flush();

    delay(100);
  }
}

void loop()
{
  unsigned long now = millis();
  bool midiSent = false;

  if (now - lastSample >= SAMPLE_INTERVAL)
  {
    for (int i = 0; i < INPUT_COUNT; i++)
    {
      byte channel = outputs[board][i].channel;
      byte note = outputs[board][i].note;

      // Update buffers.
      {
        byte previousValue = inputs[i].buffer[sampleIndex];
        byte newValue = !digitalRead(inputs[i].pinNumber);

        inputs[i].bufferSum -= previousValue;
        inputs[i].bufferSum += newValue;

        inputs[i].buffer[sampleIndex] = newValue;

        bool wasPressed = inputs[i].pressed;
        if (!wasPressed && inputs[i].bufferSum > PRESS_THRESHOLD)
        {
          inputs[i].pressed = true;
        }
        else if (wasPressed && inputs[i].bufferSum < RELEASE_THRESHOLD)
        {
          inputs[i].pressed = false;
        }
      }

      // Update states.
      {
        if (inputs[i].pressed && !inputs[i].noteOn)
        {
          inputs[i].noteOn = true;
          inputs[i].noteOnTime = now;

          midiEventPacket_t noteOn = {0x09, (uint8_t)(0x90) | channel, note, 127};
          MidiUSB.sendMIDI(noteOn);
          midiSent = true;
        }
        else if (!inputs[i].pressed && inputs[i].noteOn)
        {
          inputs[i].noteOn = false;

          midiEventPacket_t noteOff = {0x08, 0x80 | channel, note, 0};
          MidiUSB.sendMIDI(noteOff);
          midiSent = true;
        }
      }

      // Timeout
      {
        if (inputs[i].noteOn && (now - inputs[i].noteOnTime > NOTE_TIMEOUT))
        {
          inputs[i].noteOn = false;

          midiEventPacket_t noteOff = {0x08, 0x80 | channel, note, 0};
          MidiUSB.sendMIDI(noteOff);
          midiSent = true;
        }
      }
    }

    if (midiSent)
    {
      MidiUSB.flush();
    }

    sampleIndex = (sampleIndex + 1) % SAMPLE_COUNT;
    lastSample = now;
  }
}