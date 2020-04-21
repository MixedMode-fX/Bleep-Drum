/*
The Bleep Drum v12
bleeplabs.com
August 2015
Dr. Bleep (John-Mike Reed) drbleep@bleeplabs.com

See http://bleeplabs.com/store/bleep-drum-midi/ for more info
All work licensed under a Creative Commons Attribution-ShareAlike 3.0

Now compatible with current versions of MIDI, bounce and pgmspace.
It is no longer necessary to edit MIDI.h

-------------

Adapted for Arduino Nano + Platformio by Adrien Fauconnet
https://github.com/jacobanana/Bleep-Drum

*/


// Pinout

// CONTROLS
#define PLAY 8    // D8
#define REC 19    // A5
#define TAP 18    // A4
#define SHIFT 17  // A3

// TRIGGERS
#define PIN_RED 2
#define PIN_BLUE 7
#define PIN_GREEN 3
#define PIN_YELLOW 4

// LEDs
#define LED_RED 6
#define LED_GREEN 5
#define LED_BLUE 9

// POTS
#define POT_LEFT 0
#define POT_RIGHT 1

// MIDI NOTES
#define MIDI_RED 40
#define MIDI_BLUE 41
#define MIDI_GREEN 36
#define MIDI_YELLOW 37

// SEQUENCER
#define SEQUENCE_LENGTH 64

#define DEFAULT_SAMPLE_RATE 55

// DAC OUTPUTS
#define OUTPUT_0 0
#define OUTPUT_1 0
#define OUTPUT_2 0
#define OUTPUT_3 1

// DEPENDENCIES

// https://github.com/FortySevenEffects/arduino_midi_library
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>
#define BOUNCE_LOCK_OUT // alternative debouncing method: a lot more responsive, but does not cancel noise.

// DAC
#include <SPI.h>
#define DAC_CS 10 // DAC chip select pin

// AVR PROGRAM SPACE : https://www.nongnu.org/avr-libc/user-manual/pgmspace.html
#include <avr/pgmspace.h>


// SAMPLES DATA
#ifdef DAM
#include "samples_dam.h"
#elif DAM2
#include "samples_dam2.h"
#elif DAM3
#include "samples_dam3.h"
#else
#include "samples_bleep.h"
#endif

// Sine for click track
#include "sine.h"

// PLAYBACK & TRIGGERS
#include "sample_playback.h"
#define N_SAMPLES 4
SamplePlayback samples[N_SAMPLES] = {
  SamplePlayback(length0, PIN_RED, MIDI_RED),
  SamplePlayback(length1, PIN_BLUE, MIDI_BLUE),
  SamplePlayback(length2, PIN_GREEN, MIDI_GREEN),
  SamplePlayback(length3, PIN_YELLOW, MIDI_YELLOW),
};

// Sample Mixer & output
int sample_sum[2];
int sample[2]; 

// Sequences
uint8_t banko = 0; // this defines which 32 steps sequence to use. sequences are stored in a 1D array of length 128

// current step in the sequence
uint8_t loopstep = 0;
uint8_t loopstepf = 0;

// Modes
uint8_t play = 0;
uint8_t recordmode = 1;
uint8_t noise_mode = 1; // noise mode is activated when pressing shift at boot or via midi
uint8_t playmode = 1; // 1 = forward / 0 = reverse

// Noise mode 
const char noise_table[] PROGMEM = {};
uint16_t sample_noise, index_noise;
uint32_t accumulator_noise;
long noise_p1, noise_p2;
int sample_holder1;

// Sample Rate control
uint8_t sample_rate = DEFAULT_SAMPLE_RATE;

// Buttons 
uint8_t recordbutton, precordbutton;
uint8_t revbutton, prevrevbutton;
uint8_t playbutton, pplaybutton;
uint8_t tapbutton, ptapbutton, bft;
uint8_t prevshift, shift_latch;
uint8_t record;

uint8_t prevloopstep;

// Trigger input
uint8_t onetime = 1;
uint8_t trigger_step, triggerled, ptrigger_step;

// Tap tempo, LEDs & other stuff
uint32_t cm;
uint32_t dds_time;
const unsigned long dds_tune = 4294967296 / 9800; // 2^32/measured dds freq but this takes too long
uint16_t click_pitch;
uint8_t click_amp;
uint16_t click_wait;

uint8_t erase_latch;
uint32_t erase_led;
int shift_time;
int shift_time_latch;
uint8_t click_play, click_en;

unsigned long raw1, raw2;
unsigned long log1, log2;

uint8_t t;
long tapbank[2];
long prev; // Previous time
long prevtap;
unsigned long taptempo = 1000;
unsigned long ratepot;
uint8_t eee, ee;
uint8_t shift, bankpg, bankpr, bout, rout, gout;
uint8_t bankpb = 4;
unsigned long recordoffsettimer, offsetamount, taptempof;
uint8_t r, g, b, erase, e, eigth, preveigth;

// MIDI stuff
uint8_t  miditap, pmiditap, miditap2, midistep, pmidistep, miditempo, midinoise;
uint8_t midi_note_check;


// Click Track
int sine_sample;
uint8_t index_sine;
uint32_t acc_sine;


void setup() {
  Serial.begin(9600);


  // Setup sample switches
  for (uint8_t i=0; i<N_SAMPLES; i++){
    samples[i].setup();
  }

  // Default playback speed for bottom 2 samples
  samples[2].setSpeed(157); // default snare is pitched up
  samples[3].setSpeed(128);

  randomSeed(analogRead(0));
  cli(); // disable interrupt

  // Output pins
  pinMode (12, OUTPUT); pinMode (13, OUTPUT); pinMode (11, OUTPUT); pinMode (10, OUTPUT);
  pinMode (9, OUTPUT); pinMode (5, OUTPUT);  pinMode (LED_GREEN, OUTPUT);
  pinMode (16, OUTPUT);

  // Input pins
  pinMode (PLAY, INPUT_PULLUP);   
  pinMode (REC, INPUT_PULLUP);    
  pinMode (TAP, INPUT_PULLUP);    
  pinMode (SHIFT, INPUT_PULLUP);  


  delay(100);

  /* ======= INIT STATE ========= */

  if (digitalRead(TAP) == 1){

      // Assign MIDI channels at boot using coloured buttons
      if (digitalRead(PIN_RED) == LOW) {
        analogWrite(LED_RED, 64);
        MIDI.begin(1);
      }
      else if (digitalRead(PIN_BLUE) == LOW) {
        analogWrite(LED_BLUE, 64); 
        MIDI.begin(2);
      }
      else if (digitalRead(PIN_GREEN) == LOW) {
        analogWrite(LED_GREEN, 64);
        MIDI.begin(3);
      }
      else if (digitalRead(PIN_YELLOW) == LOW) {
        analogWrite(LED_RED, 48);
        analogWrite(LED_GREEN, 16);
        MIDI.begin(4);
      }
      else {
        MIDI.begin(0);
      }
  }
  delay(20000); // we're messing with the timers so this isn't actually 20000 Millis
  MIDI.turnThruOff();

  // SPI initialisation for DAC
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  /* Enable interrupt on timer2 == 127, with clk/8 prescaler. At 16MHz,
     this gives a timer interrupt at 15625Hz. */
  TIMSK2 = (1 << OCIE2A);
  OCR2A = DEFAULT_SAMPLE_RATE; // sets the compare. measured at 9813Hz with OCR2A = 50

  /* clear/reset timer on match */
  TCCR2A = 1 << WGM21 | 0 << WGM20; /* CTC mode, reset on match */
  TCCR2B = 0 << CS22 | 1 << CS21 | 1 << CS20; /* clk, /8 prescaler */

  //pwm
  TCCR0B = B0000001;
  TCCR1B = B0000001;

  sei(); // enable interrupt

  // Enable Noise mode at boot
  if (digitalRead(SHIFT) == 0) {
    noise_mode = 1;
  }
  else {
    noise_mode = 0;
  }


}







void loop() {
  cm = millis();

  midi_note_check = midi_note_on();

  for(uint8_t i=0; i<4; i++) {
    samples[i].btn_update();
  }

  // tap tempo  
  tapbutton = digitalRead(TAP);
  bft = tapbutton == 0 && ptapbutton == 1;
  ptapbutton = tapbutton;

  LEDS();
  BUTTONS();
  RECORD();
  POTS();

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RECORD() {

  pplaybutton = playbutton;
  playbutton = digitalRead(PLAY);
  if (playbutton != pplaybutton && playbutton == LOW && shift == 1) {
    miditempo = 0;
    play = !play;
  }


  precordbutton = recordbutton;
  recordbutton = digitalRead(REC);
  if (recordbutton == 0 && precordbutton == 1) {
    record = !record;
    play = 1;
    erase_latch = 1;
    eee = 0;
  }
  if (recordbutton == 1 && precordbutton == 0) {
    erase_latch = 0;
  }

  if (play == 0) {
    record = 0;
  }


  ////////////////////////////////////////////////////////////////////erase

  if (recordbutton == 0) {
    if (playbutton == 0 &&  erase_latch == 1) {
      eee++;
      if (eee >= 800) {
        eee = 0;
        erase_latch = 0;
        erase = 1;
        erase_led = millis();
        play = 1;
        record = 0;
        for(uint8_t i=0; i<N_SAMPLES; i++){
          for (uint8_t j; j < 32; j++) {
            samples[i].setStep(j + banko, 0, 0);
          }
        }
      }
    }
  }

  if (millis() - erase_led > 10000) {
    erase = 0;
  }

  if (record == 1 && miditempo == 0)
  {

    for(uint8_t i=0; i<N_SAMPLES; i++){
      if(samples[i].getTriggerFlag()) samples[i].setStep(loopstepf + banko, 1, samples[i].getSpeed());
    }

  }

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDS() {

  analogWrite(LED_BLUE, bout >> 1); //Blue
  analogWrite(LED_GREEN, (gout >> 1) + triggerled); //green
  analogWrite(LED_RED, rout >> 1);

  if (noise_mode == 1) {
    rout = r;
    gout = g;
    bout = b;
    if (shift_latch == 1) {
      if (record == 0 && play == 0 ) {
        r += (sample[0] + sample[1]) >> 4;
        b += 4;
      }
    }
    if (shift_latch == 0) {
      if (record == 0 && play == 0 ) {
        g += (sample[0] + sample[1]) >> 4;
        b += 2;
      }
    }
  }

  preveigth = eigth;

  if (g > 1) {
    g--;
  }
  if (g <= 1) {
    g = 0;
  }

  if (r > 1) {
    r--;
  }
  if (r <= 1) {
    r = 0;
  }

  if (b > 1) {
    b--;
  }
  if (b <= 1) {
    b = 0;
  }
  if (noise_mode == 0) {
    if (record == 0 && play == 0 ) {

      rout = 16;
      gout = 16;
      bout = 16;
    }

  }


  if (play == 1 && record == 0) {
    bout = b*!erase;
    rout = r*!erase;
    gout = g*!erase;

    if ( loopstep == 0 ) {
      r = 12;
      g = 15;
      b = 12;
    }
    else if ( loopstep % 4 == 0) {
      r = 5;
      g = 5;
      b = 5;
    }
    else {
      b = bankpb;
      r = bankpr;
      g = bankpg;
    }

  }

  if (play == 1 && record == 1 ) {
    bout = b;
    rout = r;
    gout = g;

    if ( loopstep == 0 ) {
      r = 30;
      g = 6;
      b = 6;
    }
    else if ( loopstep % 4 == 0) {
      r = 20;
      g = 2;
      b = 2;
    }
    else {
      b = bankpb;
      r = bankpr;
      g = bankpg;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void BUTTONS() {
  prevshift = shift;
  shift = digitalRead(SHIFT);

  if (shift == 0 && prevshift == 1) {
    shift_latch = !shift_latch;
    shift_time = 0;
    shift_time_latch = 1;

  }

  if (shift == 0 && shift_time_latch == 1) {
    shift_time++;
    if (shift_time > 800 ) {
      click_en = !click_en;
      shift_time = 0;
      shift_time_latch = 0;
    }
  }



  ///////////////////////////////////////////////////sequence select

  if (shift == 0 && recordbutton == 1) {

    if (samples[0].read() == 0 ) { //red
      banko = 63;
    }
    if (samples[1].read() == 0) { //yellow
      banko = 31;
      bankpr = 4;
      bankpg = 4;
      bankpb = 0;
    }
    if (samples[2].read() == 0 || banko == 0) { //blue
      banko = 0;
      bankpr = 0;
      bankpg = 0;
      bankpb = 8;
    }
    if (samples[3].read() == 0) { //green
      banko = 95;
      bankpr = 0;
      bankpg = 3;
      bankpb = 0;

    }

    if (tapbutton == LOW) {
      play = 1;
      ratepot = analogRead(POT_LEFT);
      taptempo = ratepot << 14;
      sample_rate = map(analogRead(POT_RIGHT), 0, 1023, DEFAULT_SAMPLE_RATE, 255);
    }

    revbutton = digitalRead(PLAY);
    if (revbutton == 0 && prevrevbutton == 1) {
      playmode = !playmode;

    }
    prevrevbutton = revbutton;
  }

  if (shift == 1) {

    for(uint8_t i=0; i<N_SAMPLES; i++){
      samples[i].setTriggerFlag(midi_note_check);
    }

  }

  ////////////////////////////////////////////

}

void POTS(){

  raw1 = (analogRead(POT_LEFT) - 1024) * -1;
  log1 = raw1 * raw1;
  raw2 = (analogRead(POT_RIGHT) - 1024) * -1;
  log2 = raw2 * raw2;


  if (noise_mode == 0) {
      samples[shift_latch * 2].setSpeed((log1 >> 11) + 2);
      samples[shift_latch * 2 + 1].setSpeed((log2 >> 11) + 42);
  }

  if (noise_mode == 1) {

    if (shift_latch == 0) {
      samples[0].setSpeed((log1 >> 11) + 2);
      samples[1].setSpeed((log2 >> 12) + 2);
    }
    if (shift_latch == 1) {
      noise_p1 = (log1 >> 6) + 1;
      noise_p2 = (log2 >> 8) + 1;
    }

  }
}

int midi_note_on() {

  int note, velocity;
  uint8_t r = MIDI.read();
  if (r == 1) {                  // Is there a MIDI message incoming ?
    uint8_t type = MIDI.getType();
    switch (type) {
      case 0x90: //note on. For some reasong "NoteOn" won't work enven though it's declared in midi_Defs.h
        note = MIDI.getData1();
        velocity = MIDI.getData2();
        if (velocity == 0) {
          note = 0;
        }

        break;
      case 0x80: //note off
        note = 0;
        break;

      case 0xB0: //control change
        switch (MIDI.getData1()){
            case 70:
              samples[0].setSpeed((MIDI.getData2() << 2) + 3);
              break;
            case 71:
              samples[1].setSpeed((MIDI.getData2() << 2) + 3);
              break;
            case 72:
              samples[2].setSpeed((MIDI.getData2() << 2) + 3);
              break;
            case 73:
              samples[3].setSpeed((MIDI.getData2() << 2) + 3);
              break;
            case 57:
              midistep = 1;
              miditempo = 1;
              digitalWrite(5, HIGH);
              break;
            case 67:
              play = !play;
              break;
            case 68:
              midinoise = 1;
              shift_latch = 1;
              noise_mode = !noise_mode;
              break;
            case 69:
              playmode = !playmode;
              break;
            case 75:
              banko = 0; //blue  
              break;
            case 76:
              banko = 31; // yellow
              break;
            case 77:
              banko = 63; // red
              break;
            case 78:
              banko = 95;
              break;

        };

        if (MIDI.getData1() != 57) midistep = 0;
        miditap2 = MIDI.getData1() == 58;

      default:
        note = 0;

    }
  }

  else {
    note = 0;
  }

  pmiditap = miditap;
  pmidistep = midistep;

  return note;
}

