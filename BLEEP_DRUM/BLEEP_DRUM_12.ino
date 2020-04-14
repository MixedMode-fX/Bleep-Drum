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



// DEPENDENCIES

// https://github.com/FortySevenEffects/arduino_midi_library
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>
#define BOUNCE_LOCK_OUT // alternative debouncing method: a lot more responsive, but does not cancel noise.

// DAC
#include <SPI.h>

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

// Pot position sequences for the top 2 samples
uint16_t B1_freq_sequence[128] = {}; // because of the amount of RAM we have available, we can only have 2. 
uint16_t B2_freq_sequence[128] = {}; // Technically, there is enough space for 3 but that would feel awkward

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
uint16_t noise_sample, index_noise;
uint32_t accumulator_noise;
long noise_p1, noise_p2;
int sample_holder1;

// Output selection
#ifdef STEREO
uint8_t outputs[4] = {1,1,1,0}; // kick is sent to its own output by default
#else
uint8_t outputs[] = {0,0,0,0};
#endif

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
uint8_t t;
long tapbank[2];
long prev; // Previous time
long prevtap;
unsigned long taptempo = 8000000;
unsigned long ratepot;
uint8_t eee, ee;
uint8_t shift, bankpg, bankpr, bout, rout, gout;
uint8_t bankpb = 4;
unsigned long recordoffsettimer, offsetamount, taptempof;
uint8_t r, g, b, erase, e, eigth, preveigth;


// MIDI stuff
uint8_t  miditap, pmiditap, miditap2, midistep, pmidistep, miditempo, midinoise;
uint8_t midi_note_check;



void setup() {
  Serial.begin(9600);


  // Setup sample switches
  for (uint8_t i=0; i<N_SAMPLES; i++){
    samples[i].setup();
  }

  // Default playback speed for bottom 2 samples
  samples[2].setSpeed(157); // default snare is pitched up
  samples[3].setSpeed(128);

  samples[0].setSpeedSequence(B1_freq_sequence);
  samples[1].setSpeedSequence(B2_freq_sequence);
  

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
  else { // press and hold TAP at boot + a pad to assign to 2nd output
    MIDI.begin(0);
    #ifdef STEREO
    outputs[0] = digitalRead(PIN_RED);
    outputs[1] = digitalRead(PIN_BLUE);
    outputs[2] = digitalRead(PIN_GREEN);
    outputs[3] = digitalRead(PIN_YELLOW);
    outputs[4] = digitalRead(PIN_RED);
    outputs[5] = digitalRead(PIN_BLUE);
    #endif
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

  /////////////////////////////////////////////////////////////////  loopstep


  taptempof = taptempo;

  recordoffsettimer = micros() - prev ;
  offsetamount = taptempof - (taptempof >> 2 );

  if ((recordoffsettimer) > (offsetamount)) loopstepf = (loopstep + 1) % 32;

  if (play == 1) {

    if (onetime == 1) {
      taptempo = 4000000;
      onetime = 0;
    }
    else {
      prevloopstep = loopstep;

      if (recordmode == 1 && miditempo == 0 && micros() - prev > (taptempof)) {
          prev = micros();
          loopstep = (loopstep + 1) % 32;
      }

      if (miditempo == 1 && midistep == 1) loopstep = (loopstep + 1) % 32;
      ptrigger_step = trigger_step;
    }

    samples[0].setLoopTrigger(loopstep + banko);
    samples[1].setLoopTrigger(loopstep + banko);
    samples[2].setLoopTrigger(loopstep + banko);
    samples[3].setLoopTrigger(loopstep + banko);

  }

  if (play == 0) {
    loopstep = 31; // reset sequencer to start on first step when pressing play
    prev = 0;
    samples[0].setLoopTrigger(0);
    samples[1].setLoopTrigger(0);
    samples[2].setLoopTrigger(0);
    samples[3].setLoopTrigger(0);
  }


  for(uint8_t i=0; i<N_SAMPLES; i++){
    
    // live triggers
    if(samples[i].getTriggerFlag()) {
      samples[i].trigger();
    }

    // sequenced triggers
    if(loopstep != prevloopstep && samples[i].getLoopTrigger() == 1){
       // the first 2 samples use the 2nd phaser 
      samples[i].trigger(i < 2);
      // this allows a different playback speed for the sequenced sounds
      if(i < 2) samples[i].setSpeed(samples[i].getSpeedStep(loopstepf + banko), 1);
    }

  }
  
  //////////////////////////////////////////////////////////////// T A P



  if (shift == 1) {

    if (bft == 1 || miditap2 == 1) {
      t = !t;
      tapbank[t] = ((micros()) - prevtap) >> 2;
      taptempo = ((tapbank[0] + tapbank[1]) >> 1);
      prevtap = micros();

    }

  }


}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RECORD() {

  playbutton = digitalRead(PLAY);
  if (playbutton != pplaybutton && playbutton == LOW && shift == 1) {
    miditempo = 0;
    play = !play;
  }
  else {
  }
  pplaybutton = playbutton;


  recordbutton = digitalRead(REC);
  if (recordbutton == LOW && recordbutton != precordbutton) {
    record = !record;
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
      samples[0].setStep(ee + banko, 0, 0);
      samples[1].setStep(ee + banko, 0, 0);
      samples[2].setStep(ee + banko, 0, 0);
      samples[3].setStep(ee + banko, 0, 0);
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


  if (noise_mode == 1) {
    rout = r;
    gout = g;
    bout = b;
    if (shift_latch == 1) {
      if (record == 0 && play == 0 ) {
        r += sample[0] >> 3;
        b += 4;
      }
    }
    if (shift_latch == 0) {

      if (record == 0 && play == 0 ) {
        g += sample[0] >> 3;
        b += 2;

      }

    }
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

    else if (loopstep % 4 == 0) {
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
  if (shift == 0 && prevshift == 1) shift_latch = !shift_latch;
  prevshift = shift;


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
      ratepot = (analogRead(POT_LEFT));
      taptempo = ratepot << 14;
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
    if (noise_mode == 0) {
    // USE DAM POT MAPPINGS BECAUSE IT'S FATTER
      samples[shift_latch * 2].setSpeed((analogRead(POT_LEFT) >> 1) + 40);
      samples[shift_latch * 2 + 1].setSpeed((analogRead(POT_RIGHT) >> 2) + 2);
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

