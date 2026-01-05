#include <MIDIUSB.h>
#include <avr/wdt.h>

#define BOARD_COUNT 3
#define INPUT_COUNT 8

#define SAMPLE_COUNT 16
#define SAMPLE_INTERVAL 4

#define PRESS_THRESHOLD 256
#define RELEASE_THRESHOLD 384

#define NOTE_TIMEOUT 5000

#define DEBUG_TIMEOUT 2500

const uint8_t boardSelectA = 2;
const uint8_t boardSelectB = 3;

int sampleIndex = 0;
unsigned long lastSample = 0;

unsigned long lastDebug = 0;

typedef struct
{
  byte pinNumber;
  uint16_t buffer[SAMPLE_COUNT];
  uint32_t bufferSum;
  boolean pressed;
  boolean noteOn;
  unsigned long noteOnTime;
} ReachInput;

typedef struct
{
  byte channel;
  byte note;
} ReachOutput;

#define CHANNEL_A 0
#define CHANNEL_B 1
#define CHANNEL_C 2
#define CHANNEL_D 3

#define NOTE_ONE 60
#define NOTE_TWO 64
#define NOTE_THREE 67
#define NOTE_FOUR 72
#define NOTE_FIVE 76
#define NOTE_SIX 79

ReachInput inputs[INPUT_COUNT] = {
    {A6}, {A7}, {A8}, {A9}, {A0}, {A1}, {A2}, {A3}};

ReachOutput outputs[BOARD_COUNT][INPUT_COUNT] = {
    // Panel A
    {
        {CHANNEL_C, NOTE_FIVE},  // Star 1
        {CHANNEL_A, NOTE_FIVE},  // Star 2
        {CHANNEL_B, NOTE_THREE}, // Star 3
        {CHANNEL_C, NOTE_FOUR},  // Star 4
        {CHANNEL_C, NOTE_ONE},   // Star 5
        {CHANNEL_D, NOTE_ONE},   // Star 6
        {CHANNEL_A, NOTE_TWO},   // Star 7
        {CHANNEL_A, NOTE_FOUR},  // Star 8
    },
    // Panel B
    {
        {CHANNEL_B, NOTE_FIVE},  // Star 1
        {CHANNEL_D, NOTE_THREE}, // Star 2
        {CHANNEL_A, NOTE_ONE},   // Star 3
        {CHANNEL_B, NOTE_TWO},   // Star 4
        {CHANNEL_B, NOTE_FOUR},  // Star 5
        {CHANNEL_D, NOTE_SIX},   // Star 6
        {CHANNEL_B, NOTE_SIX},   // Star 7
        {CHANNEL_D, NOTE_TWO},   // Star 8
    },
    // Panel C
    {
        {CHANNEL_C, NOTE_THREE}, // Star 1
        {CHANNEL_D, NOTE_FIVE},  // Star 2
        {CHANNEL_A, NOTE_SIX},   // Star 3
        {CHANNEL_B, NOTE_ONE},   // Star 4
        {CHANNEL_D, NOTE_FOUR},  // Star 5
        {CHANNEL_C, NOTE_SIX},   // Star 6
        {CHANNEL_C, NOTE_TWO},   // Star 7
        {CHANNEL_A, NOTE_THREE}, // Star 8
    },
};

uint8_t board = 0;

void setup()
{
  // Disable watchdog.
  wdt_disable();

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

  // Enable watchdog.
  delay(1000);
  wdt_enable(WDTO_4S);
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
        uint16_t previousValue = inputs[i].buffer[sampleIndex];
        uint16_t newValue = analogRead(inputs[i].pinNumber);

        inputs[i].bufferSum -= previousValue;
        inputs[i].bufferSum += newValue;

        inputs[i].buffer[sampleIndex] = newValue;

        uint16_t averageValue = inputs[i].bufferSum / SAMPLE_COUNT;

        bool wasPressed = inputs[i].pressed;
        if (!wasPressed && averageValue < PRESS_THRESHOLD)
        {
          inputs[i].pressed = true;
        }
        else if (wasPressed && averageValue > RELEASE_THRESHOLD)
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

    sampleIndex = (sampleIndex + 1) % SAMPLE_COUNT;
    lastSample = now;
  }

  // Status debug.
  if (now - lastDebug >= DEBUG_TIMEOUT)
  {
    // Heartbeat to flash TX LED.
    // MidiUSB.sendMIDI({0x04, 0xF0, 0x7D, 0x01});
    // MidiUSB.sendMIDI({0x06, 0x00, 0xF7, 0x00});
    // midiSent = true;

    for (int i = 0; i < INPUT_COUNT; i++)
    {
      uint16_t averageValue = inputs[i].bufferSum / SAMPLE_COUNT;
      Serial.print(averageValue);
      Serial.print(", ");
    }

    Serial.println();
    lastDebug = now;
  }

  // Send all messages.
  if (midiSent)
  {
    MidiUSB.flush();
  }

  // Reset watchdog.
  wdt_reset();
}