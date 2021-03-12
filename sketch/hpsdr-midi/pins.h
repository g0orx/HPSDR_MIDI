
#define PRESSED false
#define RELEASED true

typedef struct {
  int A;
  int B;
  int SW;
  unsigned long debounce;
  bool state;
  bool ignore;
  byte A_state;
  byte B_state;
  int value;
  Rotary/*Encoder*/ *re;
} ENCODER;

typedef struct {
  int SW;
  unsigned long debounce;
  bool state;
} SWITCH;
