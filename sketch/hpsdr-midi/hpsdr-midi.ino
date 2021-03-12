/* HPSDR MIDI Controller
 *
 * Copyright 2021 John Melton. Licenced under the GNU GPL Version 3.
 * Contact: john.d.melton@googlemail.com
 *
 * Source code and hardware can be found at https://github.com/g0orx/HPSDR_MIDI.git
 */
 
#include <MIDIUSB.h>
#include <Arduino.h>
#include "Rotary.h"
#include "pins.h"

//#define DEBUG

midiEventPacket_t input_message;

#define DEBOUNCE 50 // ms
#define UPDATE 100 // ms
unsigned long update_millis=0;
unsigned long t;

#define MAX_ENCODERS 7
#define CHANNEL 3

int counter;

ENCODER encoders[MAX_ENCODERS] = {
  {12, 13, 11, 0, true, false, 0x01, 0x01, 0, NULL},
  {9, 10, 8, 0, true, false, 0x01, 0x01, 0, NULL},
  {6, 7, 5, 0, true, false, 0x01, 0x01, 0, NULL},
  {3, 4, 2, 0, true, false, 0x01, 0x01, 0, NULL},
  {15, 14, 16, 0, true, false, 0x01, 0x01, 0, NULL},
  {18, 17, 19, 0, true, false, 0x01, 0x01, 0, NULL},
  {21, 20, 22, 0, true, false, 0x01, 0x01, 0, NULL}
};

int encoder=0;

#define MAX_SWITCHES 33

#define ENCODER_SWITCH_START (MAX_SWITCHES+1)
#define ENCODER_START (ENCODER_SWITCH_START+MAX_ENCODERS)

#define SHIFT_KEY 11
#define LED 46  // LIT when shift enabled

boolean shifted=false;

SWITCH switches[MAX_SWITCHES] = {
  {A0, 0, true},
  {A1, 0, true},
  {A2, 0, true},
  {A3, 0, true},
  {A4, 0, true},
  {A5, 0, true},
  {A6, 0, true},
  {A7, 0, true},
  {A8, 0, true},
  {A9, 0, true},
  {A10, 0, true},
  {A11, 0, true},   // used as shift key
  {41, 0, true},
  {35, 0, true},
  {29, 0, true},
  {23, 0, true},
  {43, 0, true},
  {37, 0, true},
  {31, 0, true},
  // skip D0 and D1 as they are used for the USB Programming port
  {25, 0, true},
  {45, 0, true},
  {39, 0, true},
  {33, 0, true},
  {27, 0, true},
  {32, 0, true},
  {26, 0, true},
  {28, 0, true},
  {30, 0, true},
  {24, 0, true},
  {47, 0, true},
  {48, 0, true},
  {49, 0, true},
  {50, 0, true},
};

int sw=0;


void noteOn(byte channel, byte pitch, byte velocity) {
  if(shifted) pitch+=64;
  midiEventPacket_t noteOn = {0x09, (byte)(0x90 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}

void noteOff(byte channel, byte pitch, byte velocity) {
  if(shifted) pitch+=64;
  midiEventPacket_t noteOff = {0x08, (byte)(0x80 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}

void controller(byte channel, byte pitch, byte velocity) {
  if(shifted) pitch+=64;
  midiEventPacket_t noteOff = {0x09, (byte)(0xB0 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}

void checkEncoders() {
  for (int i = 0; i < MAX_ENCODERS; i++) {
    if (encoders[i].value != 0) {
#ifdef DEBUG
      Serial.print("Encoder ");
      Serial.print(i);
      Serial.print(" address=");
      Serial.print(encoders[i].A);
      Serial.print(" value=");
      Serial.println(encoiders[i].value);
#endif
      controller(CHANNEL, i+ENCODER_START, 64+encoders[i].value);
      encoders[i].value=0;
    }
  }
}

void buttonPressed(bool encoder, int i) {
#ifdef DEBUG
  if (encoder) {
    Serial.print("buttonPressed: encoder: ");
  } else {
    Serial.print("burronPressed: switch: ");
  }
  Serial.print(i);
  Serial.print(" sw=");
  if (encoder) {
    Serial.println(encoders[i].SW);
  } else {
    Serial.println(switches[i].SW);
  }
#endif
  if (encoder) {
    noteOn(CHANNEL, (byte)i + ENCODER_SWITCH_START, 127);
  } else {
    if (i == SHIFT_KEY) {
      shifted=!shifted;
      digitalWrite(LED, shifted);
    } else {
      noteOn(CHANNEL, (byte)i + 1, 127);
    }
  }
}

void buttonReleased(bool encoder, int i) {
#ifdef DEBUG
  if (encoder) {
    Serial.print("buttonReleased: encoder: ");
  } else {
    Serial.print("burronReleased: switch: ");
  }
  Serial.print(i);
  Serial.print(" sw=");
  if (encoder) {
    Serial.println(encoders[i].SW);
  } else {
    Serial.println(switches[i].SW);
  }
#endif
  if (encoder) {
    //noteOff(channel, (byte)encoders[i].SW, 0);
    noteOff(CHANNEL, (byte)i + ENCODER_SWITCH_START, 0);
  } else {
    if (i != SHIFT_KEY) {
      //noteOff(channel, (byte)switches[i].SW, 0);
      noteOff(CHANNEL, (byte)i + 1, 0);
    }
  }
}

void pollSwitch(boolean encoder,int sw) {
  byte v;
  
  t = millis();
  if(encoder) {
    if (t > encoders[sw].debounce) {
      v = digitalRead(encoders[sw].SW);
      if (v != encoders[sw].state) {
        if (v == PRESSED) {
          buttonPressed(encoder, sw);
        } else {
          buttonReleased(encoder, sw);
        }
        encoders[sw].debounce = t + DEBOUNCE;
        encoders[sw].state = v;
      }
    }
  } else {
    if (t > switches[sw].debounce) {
      v = digitalRead(switches[sw].SW);
      if (v != switches[sw].state) {
        if (v == PRESSED) {
          buttonPressed(encoder, sw);
        } else {
          buttonReleased(encoder, sw);
        }
        switches[sw].debounce = t + DEBOUNCE;
        switches[sw].state = v;
      }
    }
  }
  
}


void setup() {

#ifdef DEBUG
  // DEBUG output
  Serial.begin(115200);
  delay(100);

  Serial.println("HPSDR-MIDI Controller");
#endif
  // initialize LED
#ifdef DEBUG
  Serial.println("Initialize LED");
#endif
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);

  // initialize encoders
#ifdef DEBUG
  Serial.println("Initialize encoders");
#endif
  for (int i = 0; i < MAX_ENCODERS; i++) {
    //pinMode(encoders[i].A, INPUT_PULLUP);
    encoders[i].re=new Rotary(encoders[i].A,encoders[i].B);
    pinMode(encoders[i].SW, INPUT_PULLUP);
    encoders[i].debounce=0;
  }
#ifdef DEBUG
  Serial.println("Initialize switches");
#endif
  for (int i = 0; i < MAX_SWITCHES; i++) {
    pinMode(switches[i].SW, INPUT_PULLUP);
    switches[i].debounce=0;
  }
#ifdef DEBUG
  Serial.println("Exit setup");
  delay(100);
#endif

  counter = 0;
}

void loop() {

  if(MidiUSB.available()!=0) {
    digitalWrite(LED, !shifted);
    input_message=MidiUSB.read();
#ifdef DEBUG
    Serial.print("Input Message: ");
    Serial.print(input_message.header);
    Serial.print(",");
    Serial.print(input_message.byte1);
    Serial.print(",");
    Serial.print(input_message.byte2);
    Serial.print(",");
    Serial.println(input_message.byte3);
#endif
    digitalWrite(LED, shifted);
  }
  
  //encoders[encoder].re->tick();
  unsigned char dir=encoders[encoder].re->process();
  /*pollSwitch(true,encoder);*/
  if(dir==DIR_CW) {
    encoders[encoder].value++;
  } else if(dir==DIR_CCW) {
    encoders[encoder].value--;
  }
  pollSwitch(true,encoder);
  encoder++;
  if(encoder==MAX_ENCODERS) encoder=0;
  
  pollSwitch(false,sw);
  sw++;
  if(sw==MAX_SWITCHES) sw=0;
  counter++;

  t=millis();
  if(t>update_millis) {
    checkEncoders();
    update_millis=t+UPDATE;
  }
}
