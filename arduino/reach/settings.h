#define DEVICE 0

#define INPUT_COUNT 8
#define BUFFER_LENGTH 3
#define TARGET_LOOP_TIME 744

//
// Different keys for each device
//

int pinNumbers[INPUT_COUNT] = {
  2,3,4,5,6,7,8,9};

int ledPins[INPUT_COUNT] = {
  10,11,23,22,21,20,19,18 
};

#if DEVICE == 0

char keyDowns[INPUT_COUNT] = {
  'Q','W','E','R','T','Y','U','I'};
char keyUps[INPUT_COUNT] = {
  'q','w','e','r','t','y','u','i'};

#elif DEVICE == 1

char keyDowns[INPUT_COUNT] = {
  'A','S','D','F','G','H','J','K'};
char keyUps[INPUT_COUNT] = {
  'a','s','d','f','g','h','j','k'};

#elif DEVICE == 2

char keyDowns[INPUT_COUNT] = {
  'Z','X','C','V','B','N','M','<'};
char keyUps[INPUT_COUNT] = {
  'z','x','c','v','b','n','m',','};

#endif
