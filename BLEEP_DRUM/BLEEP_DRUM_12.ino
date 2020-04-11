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
#define PLAY 8    // D8
#define REC 19    // A5
#define TAP 18    // A4
#define SHIFT 17  // A3

#define RED_PIN 2
#define BLUE_PIN 7
#define GREEN_PIN 3
#define YELLOW_PIN 4

#define LED_RED 6
#define LED_GREEN 5
#define LED_BLUE 9

#define POT_LEFT 0
#define POT_RIGHT 1

// MIDI NOTES
#define MIDI_RED 40
#define MIDI_BLUE 41
#define MIDI_GREEN 36
#define MIDI_YELLOW 37

#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

#include <SPI.h>
#include <Bounce2.h>

#define BOUNCE_LOCK_OUT
// activate the alternative debouncing method. This method is a lot more responsive, but does not cancel noise.

#include <avr/pgmspace.h>

#include "sample.h"

#ifdef DAM
#include "samples_dam.h"
#elif DAM2
#include "samples_dam2.h"
#elif DAM3
#include "samples_dam3.h"
#else
#include "samples_bleep.h"
#endif

Bounce debouncerRed = Bounce(); 
Bounce debouncerGreen = Bounce(); 
Bounce debouncerBlue = Bounce(); 
Bounce debouncerYellow = Bounce(); 

int sample_holder1;
byte eee, ee;
byte shift, bankpg, bankpr, bout, rout, gout;
byte bankpb = 4;
int pot1 = 127;
int pot2 = 4;
long pot3, pot4;

// Sequences
byte banko = 0; // this defines which 32 steps sequence to use. sequences are stored in a 1D array of length 128
byte B1_sequence[128] = {}; // triggers sequences
byte B2_sequence[128] = {};
byte B3_sequence[128] = {};
byte B4_sequence[128] = {
  1, 0, 0, 0 , 0, 0, 0, 0 , 1, 0, 0, 0 , 0, 0, 0, 0 , 1, 0, 0, 0 , 0, 0, 0, 0 , 1, 0, 0, 0 , 0, 0, 0, 0, // banko = 0
  1, 0, 0, 0 , 0, 0, 0, 0 , 1, 0, 0, 0 , 0, 0, 0, 0 , 1, 0, 0, 0 , 0, 0, 0, 0 , 1, 0, 0, 0 , 0, 0, 0, 0, // banko = 31
  0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0, // banko = 63
  1, 1, 1, 1 , 1, 1, 1, 1 , 1, 1, 1, 1 , 1, 1, 1, 1 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0, // banko = 95
};

int B1_freq_sequence[128] = {}; // frequency pots sequences
int B2_freq_sequence[128] = {};

byte loopstep = 0;
byte loopstepf = 0;


// Sample playback
byte playmode = 1; // 1 = forward / 0 = reverse

// samples
#define N_SAMPLES 6
Sample samples[N_SAMPLES] = {
  Sample(*table0, length0),
  Sample(*table1, length1),
  Sample(*table2, length2),
  Sample(*table3, length3),
  Sample(*table0, length0),
  Sample(*table1, length1),
};

// Sample MIXER
int sample_sum, sample_sum_b;
int sample, sample_b; 


// Noise mode 
const char noise_table[] PROGMEM = {};
uint16_t noise_sample, index_noise;
uint32_t accumulator_noise;
long noise_p1, noise_p2;


// Modes
byte play = 0;
byte recordmode = 1;
byte noise_mode = 1; // noise mode is activated when pressing shift at boot

// Triggers
byte B1_trigger, B1_loop_trigger;
byte B2_trigger, B2_loop_trigger;
byte B3_trigger, B3_loop_trigger;
byte B4_trigger, B4_loop_trigger;

long prev; // Previous time
long prevtap;
unsigned long taptempo = 8000000;
unsigned long ratepot;
byte r, g, b, erase, e, eigth, preveigth;

// Buttons 
byte recordbutton, precordbutton;
byte revbutton, prevrevbutton;
byte playbutton, pplaybutton;
byte tapbutton, ptapbutton, bft;
byte preva, prevb;


byte record;
byte looptrigger, prevloopstep;

// Trigger input
byte onetime = 1;
byte trigger_step, triggerled, ptrigger_step;

byte midi_note_check;

byte prevshift, shift_latch;

// Tap tempo
byte t, tiggertempo;
long tapbank[4];

// MIDI stuff
byte  miditap, pmiditap, miditap2, midistep, pmidistep, miditempo, midinoise;

unsigned long recordoffsettimer, offsetamount, taptempof;

void setup() {
  Serial.begin(9600);

  samples[2].setSpeed(157); // default snare is pitched up
  samples[3].setSpeed(128);

  randomSeed(analogRead(0));
  cli(); // disable interrupt
  taptempo = 4000000;

  // Output pins
  pinMode (12, OUTPUT); pinMode (13, OUTPUT); pinMode (11, OUTPUT); pinMode (10, OUTPUT);
  pinMode (9, OUTPUT); pinMode (5, OUTPUT);  pinMode (LED_GREEN, OUTPUT);
  pinMode (16, OUTPUT);

  // Input pins
  pinMode (PLAY, INPUT_PULLUP);   
  pinMode (REC, INPUT_PULLUP);    
  pinMode (TAP, INPUT_PULLUP);    
  pinMode (SHIFT, INPUT_PULLUP);  

  pinMode (GREEN_PIN, INPUT_PULLUP);   //low left clap green
  pinMode (YELLOW_PIN, INPUT_PULLUP);   // low right kick yellow
  pinMode (BLUE_PIN, INPUT_PULLUP);    //Up Right tom Blue
  pinMode (RED_PIN, INPUT_PULLUP);   // Up right pew red

  // Debouncing on note triggers
  debouncerGreen.attach(GREEN_PIN);
  debouncerGreen.interval(2); // interval in ms
  debouncerYellow.attach(YELLOW_PIN);
  debouncerYellow.interval(2); // interval in ms  
  debouncerBlue.attach(BLUE_PIN);
  debouncerBlue.interval(2); // interval in ms
  debouncerRed.attach(RED_PIN);
  debouncerRed.interval(2); // interval in ms

  delay(100);

  /* ======= INIT STATE ========= */

  // Assign MIDI channels at boot using coloured buttons
  if (digitalRead(RED_PIN) == LOW) {
    analogWrite(LED_RED, 64);
    MIDI.begin(1);
  }
  else if (digitalRead(BLUE_PIN) == LOW) {
    analogWrite(LED_BLUE, 64); 
    MIDI.begin(2);
  }
  else if (digitalRead(GREEN_PIN) == LOW) {
    analogWrite(LED_GREEN, 64);
    MIDI.begin(3);
  }
  else if (digitalRead(YELLOW_PIN) == LOW) {
    analogWrite(LED_RED, 48);
    analogWrite(LED_GREEN, 16);
    MIDI.begin(4);
  }
  else {
    MIDI.begin(0);
  }
  delay(20000); // we're messing with the timers so this isn't actually 20000 Millis
  MIDI.turnThruOff();

  // Enable Noise mode at boot
  if (digitalRead(SHIFT) == 0) {
    noise_mode = 1;
  }
  else {
    noise_mode = 0;
  }


  // SPI initialisation for DAC
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  /* Enable interrupt on timer2 == 127, with clk/8 prescaler. At 16MHz,
     this gives a timer interrupt at 15625Hz. */
  TIMSK2 = (1 << OCIE2A);
  OCR2A = 128;

  /* clear/reset timer on match */
  TCCR2A = 1 << WGM21 | 0 << WGM20; /* CTC mode, reset on match */
  TCCR2B = 0 << CS22 | 1 << CS21 | 1 << CS20; /* clk, /8 prescaler */

  //pwm
  TCCR0B = B0000001;
  TCCR1B = B0000001;

  sei(); // enable interrupt

}








void loop() {

  midi_note_check = midi_note_on();

  debouncerRed.update();
  debouncerBlue.update();
  debouncerGreen.update();
  debouncerYellow.update();
  
  tapbutton = digitalRead(TAP);

  if (tapbutton == 0 && ptapbutton == 1) {
    bft = 1;
  }
  else {
    bft = 0;
  }

  HANDLE_MIDI();
  LEDS();
  BUTTONS();
  RECORD();
  POTS();

  /////////////////////////////////////////////////////////////////  loopstep


  taptempof = taptempo;

  recordoffsettimer = micros() - prev ;
  offsetamount = taptempof - (taptempof >> 2 );

  if ((recordoffsettimer) > (offsetamount))
  {

    loopstepf = loopstep + 1;
    loopstepf %= 32;

  }

  if (play == 1) {

    if (onetime == 1) {
      taptempo = 4000000;
      onetime = 0;
    }
    else {
      prevloopstep = loopstep;
      preva = eigth;

      if (recordmode == 1 && miditempo == 0 && tiggertempo == 0) {
        if (micros() - prev > (taptempof) ) {
          prev = micros();

          loopstep++;
          if (loopstep > 31)
          {
            loopstep = 0;
          }
        }

      }

      if (miditempo == 1) {

        if (midistep == 1) {

          loopstep++;
          if (loopstep > 31)
          {
            loopstep = 0;
          }
        }
      }


      if (tiggertempo == 1) {

        if (trigger_step == 1 && ptrigger_step == 0) {

          triggerled = 30;

          loopstep++;
          if (loopstep > 31)
          {
            loopstep = 0;
          }

        }
        else
          triggerled = 0;

      }

      ptrigger_step = trigger_step;

    }

    B1_loop_trigger = B1_sequence[loopstep + banko];
    B2_loop_trigger = B2_sequence[loopstep + banko];
    B3_loop_trigger = B3_sequence[loopstep + banko];
    B4_loop_trigger = B4_sequence[loopstep + banko];

  }

  if (play == 0) {
    loopstep = 31;
    prev = 0;
    B1_loop_trigger = 0;
    B2_loop_trigger = 0;
    B3_loop_trigger = 0;
    B4_loop_trigger = 0;

  }


  if (B1_trigger == 1) {
    samples[0].trigger();
  }

  if (loopstep != prevloopstep && B1_loop_trigger == 1) {
    samples[4].trigger();

    if (tiggertempo == 0) {
      samples[4].setSpeed(B1_freq_sequence[loopstepf + banko]);
    }
    else{
      samples[4].setSpeed(B1_freq_sequence[loopstep + banko]);
    }

  }


  if (B2_trigger == 1) {
    samples[1].trigger();
  }
  
  if (loopstep != prevloopstep && B2_loop_trigger == 1) {
    samples[5].trigger();

    if (tiggertempo == 0) {
      samples[5].setSpeed(B2_freq_sequence[loopstepf + banko]);
    }
    else{
      samples[5].setSpeed(B2_freq_sequence[loopstep + banko]);
    }

  }

  if (B3_trigger == 1 || (loopstep != prevloopstep && B3_loop_trigger == 1)) {
    samples[2].trigger();
  }
  
  if (B4_trigger == 1 || (loopstep != prevloopstep && B4_loop_trigger == 1)) {
    samples[3].trigger();
  }
  
  //////////////////////////////////////////////////////////////// T A P



  if (shift == 1) {

    if (bft == 1 || miditap2 == 1) {
      tiggertempo = 0;

      t++;
      t %= 2;
      tapbank[t] = ((micros()) - prevtap) >> 2;
      taptempo = ((tapbank[0] + tapbank[1]) >> 1);
      prevtap = micros();

    }
    else {
    }
  }


}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RECORD() {

  playbutton = digitalRead(PLAY);
  if (playbutton != pplaybutton && playbutton == LOW && shift == 1) {
    miditempo = 0;
    play++;
    play %= 2;
  }
  else {
  }
  pplaybutton = playbutton;


  recordbutton = digitalRead(REC);
  if (recordbutton == LOW && recordbutton != precordbutton) {
    record++;
    record %= 2;
    play = 1;
  }

  else {
  }

  if (play == 0) {
    record = 0;
  }


  ////////////////////////////////////////////////////////////////////erase

  precordbutton = recordbutton;

  if (playbutton == LOW && recordbutton == LOW) {
    eee++;
    if (eee >= 84) {
      erase = 1;
      play = 1;
      record = 0;
      B1_sequence[ee + banko] = 0;
      B2_sequence[ee + banko] = 0;
      B4_sequence[ee + banko] = 0;
      B3_sequence[ee + banko] = 0;
      ee++;
      if (ee == 32) {
        ee = 0;
        eee = 0;
      }
    }

  }
  else {
    erase = 0;
  }



  if (record == 1 && tiggertempo == 0 && miditempo == 0)
  {

    if (B1_trigger == 1) {
      B1_sequence[loopstepf + banko] = 1;
      B1_freq_sequence[loopstepf + banko] = samples[0].getSpeed();

    }

    if (B2_trigger == 1) {
      B2_sequence[loopstepf + banko] = 1;
      B2_freq_sequence[loopstepf + banko] = samples[1].getSpeed();
    }

    if (B4_trigger == 1) {
      B4_sequence[loopstepf + banko] = 1;
    }

    if (B3_trigger == 1) {
      B3_sequence[loopstepf + banko] = 1;
    }

  }

  if (record == 1)
  {
    if (tiggertempo == 1 || miditempo == 1)
    {

      if (B1_trigger == 1) {
        B1_sequence[loopstep + banko] = 1;
        B1_freq_sequence[loopstep + banko] = samples[0].getSpeed();

      }

      if (B2_trigger == 1) {
        B2_sequence[loopstep + banko] = 1;
        B2_freq_sequence[loopstep + banko] = samples[1].getSpeed();
      }

      if (B4_trigger == 1) {
        B4_sequence[loopstep + banko] = 1;
      }

      if (B3_trigger == 1) {
        B3_sequence[loopstep + banko] = 1;
      }

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
        r += sample >> 3;
        b += 4;
      }
    }
    if (shift_latch == 0) {

      if (record == 0 && play == 0 ) {
        g += sample >> 3;
        b += 2;

      }

    }
  }

  else {
  }

  preveigth = eigth;

  if (erase == 1) {
    e = 16;
  }
  if (erase == 0) {
    e = 0;
  }

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
    bout = b;
    rout = r;
    gout = g;


    ///////////////////////////////////////////////////CHANGE TO ONLY += WHEN LOOPSTEP != PREVLOOPSTEP

    if ( loopstep == 0 && prevloopstep == 1 ) {
      r = 64;
      g = 64;
      b = 64;

    }
    /*
    else  if( loopstep==16 ){
     r=4;
     g=64;
     b=4;
     }
     */
    else if ( loopstep % 4 == 0 && prevloopstep % 4 != 0 ) {
      r += 64;
      g += 64;
      b += 64;

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
      r = 32;
      g = 16;
      b = 6;
    }
    else  if ( loopstep == 16 ) {
      r = 32;
      g = 0;
      b = 0;
    }

    else if ( loopstep == 4 || loopstep == 8 || loopstep == 12   || loopstep == 20 || loopstep == 24 || loopstep == 28) {
      r = 48;
      g = 0;
      b = 0;
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
  shift = digitalRead(SHIFT);

  if (shift == 0 && prevshift == 1) {
    shift_latch++;
    shift_latch %= 2;
  }

  prevshift = shift;
  ///////////////////////////////////////////////////sequence select

  if (shift == 0 && recordbutton == 1) {
    if (debouncerRed.read() == 0 ) { //red
      banko = 63;

    }
    if (debouncerYellow.read() == 0) { //yellow
      banko = 31;
      bankpr = 4;
      bankpg = 4;
      bankpb = 0;
    }
    if (debouncerBlue.read() == 0 || banko == 0) { //blue
      banko = 0;
      bankpr = 0;
      bankpg = 0;
      bankpb = 8;
    }
    if (debouncerGreen.read() == 0) { //green
      banko = 95;
      bankpr = 0;
      bankpg = 3;
      bankpb = 0;

    }


    if (tapbutton == LOW) {
      play = 1;
      ratepot = (analogRead(14));
      taptempo = ratepot << 14;
    }
    revbutton = digitalRead(PLAY);
    if (revbutton == 0 && prevrevbutton == 1) {
      playmode++;
      playmode %= 2;

    }
    prevrevbutton = revbutton;
  }

  if ( banko == 63) {

    bankpr = 4;
    bankpg = 0;
    bankpb = 0;
  }
  if ( banko == 31) {
    bankpr = 4;
    bankpg = 4;
    bankpb = 0;
  }
  if ( banko == 0) {
    bankpr = 0;
    bankpg = 0;
    bankpb = 8;
  }

  if ( banko == 95) {
    bankpr = 0;
    bankpg = 3;
    bankpb = 0;

  }


  if (shift == 1) {

    if (debouncerRed.fell() == 1 || midi_note_check == MIDI_RED) {
      B1_trigger = 1;
    }
    else {
      B1_trigger = 0;
    }

    if (debouncerBlue.fell() == 1 || midi_note_check == MIDI_BLUE) {
      B2_trigger = 1;
    }
    else {
      B2_trigger = 0;
    }

    if (debouncerGreen.fell() == 1 || midi_note_check == MIDI_GREEN) {
      B3_trigger = 1;
    }
    else {
      B3_trigger = 0;
    }

    if (debouncerYellow.fell() == 1 || midi_note_check == MIDI_YELLOW) {
      B4_trigger = 1;
    }
    else {
      B4_trigger = 0;
    }


  }

  ////////////////////////////////////////////

}

void POTS(){
    if (noise_mode == 0) {
    // USE DAM POT MAPPINGS BECAUSE IT'S FATTER
    samples[0].setSpeed((analogRead(POT_LEFT) >> 1) + 40);
    samples[1].setSpeed((analogRead(POT_RIGHT) >> 2) + 2);
  }

  if (noise_mode == 1) {

    if (midinoise == 1) {
      samples[0].setSpeed((samples[0].getSpeed() >> 1) + 1);
      samples[1].setSpeed((samples[1].getSpeed() >> 2) + 1);
      samples[2].setSpeed((samples[2].getSpeed() + 1) << 4);
      samples[3].setSpeed((samples[3].getSpeed() + 1) << 2);
    }
    if (midinoise == 0) {


      if (shift_latch == 0) {
        samples[0].setSpeed((analogRead(POT_LEFT) >> 1) + 1);
        samples[1].setSpeed((analogRead(POT_RIGHT) >> 2) + 1);
      }
      if (shift_latch == 1) {
        noise_p1 = (analogRead(POT_RIGHT) << 4); ////////////////MAKE ME BETTERERER
        noise_p2 = (analogRead(POT_LEFT) << 2);
      }
    }
  }
}

int midi_note_on() {

  int note, velocity;
  byte r = MIDI.read();
  if (r == 1) {                  // Is there a MIDI message incoming ?
    byte type = MIDI.getType();
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
        if (MIDI.getData1() == 70) {
          samples[0].setSpeed((MIDI.getData2() << 2) + 3);
        }

        if (MIDI.getData1() == 71) {
          samples[1].setSpeed((MIDI.getData2() << 2) + 3);
        }

        if (MIDI.getData1() == 72) {
          samples[2].setSpeed((MIDI.getData2() << 2));
        }

        if (MIDI.getData1() == 73) {
          samples[3].setSpeed(MIDI.getData2() << 2);
        }


      default:
        note = 0;

    }
  }

  else {
    note = 0;
  }

  return note;
}

void HANDLE_MIDI(){
    if (midi_note_check == 58) {
    miditap2 = 1;
  }
  else {
    miditap2 = 0;
  }
  if (midi_note_check == 57) {
    midistep = 1;
    miditempo = 1;
    digitalWrite(5, HIGH);
  }

  else {
    midistep = 0;
  }

  if (midi_note_check == 67) {
    play++;
    play %= 2;
  }

  if (midi_note_check == 69) {
    playmode++;
    playmode %= 2;
  }


  if (midi_note_check == 70) {
    midinoise = 1;
    shift_latch = 1;
    noise_mode++;
    noise_mode %= 2;
    //   digitalWrite(5,HIGH);
  }

  if (midi_note_check == 72) {
    banko = 0; //blue
  }
  if (midi_note_check == 74) {
    banko = 31; // yellow
  }
  if (midi_note_check == 76) {
    banko = 63; //red
  }
  if (midi_note_check == 77) {
    banko = 95; //green
  }

  ptapbutton = tapbutton;
  pmiditap = miditap;
  pmidistep = midistep;
}