/*
The Bleep Drum v12
bleeplabs.com
August 2015
Dr. Bleep (John-Mike Reed) drbleep@bleeplabs.com

See http://bleeplabs.com/store/bleep-drum-midi/ for more info
All work licensed under a Creative Commons Attribution-ShareAlike 3.0

Now compatible with current versions of MIDI, bounce and pgmspace.
It is no longer necessary to edit MIDI.h

*/



// !!
// Most problems compiling come from having an old version of the midi library of multiple ones installed.
// !!

#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

#include "Arduino.h"


#include <avr/pgmspace.h>

#ifdef DAM
#include "samples_dam.h"
#elseif DAM2
#include "samples_dam2.h"
#elseif DAM3
#include "samples_dam3.h"
#else
#include "samples_bleep.h"
#endif

#include <SPI.h>
#include <Bounce2.h>
#define BOUNCE_LOCK_OUT

Bounce debouncerRed = Bounce(); 
Bounce debouncerGreen = Bounce(); 
Bounce debouncerBlue = Bounce(); 
Bounce debouncerYellow = Bounce(); 

const char noise_table[] PROGMEM = {};


int sample_holder1, sample_holder2;
byte eee, ee, ef, eef;
byte shift, bankpg, bankpr, bout, rout, gout, prevpot2;
byte      banko = 0;
byte n1, n2;
int n3;
byte bankpb = 4;
byte beat;
int pot1 = 127;
int pot2 = 4;
long pot3, pot4;
int kick_sample, snare_sample, sample, hat_sample, noise_sample, bass_sample, B2_freq_sample, B1_freq_sample;
uint16_t increment, increment2, increment3, increment4, increment5, increment2v, increment4v;
uint32_t accumulator, accumulator2, accumulator3, accumulator4, accumulator5, accu_freq_1, accu_freq_2;
int rando;
//byte B1_sequence[16]={0,1,0,1 ,1,1,0,0 ,0,1,0,1 ,1,1,1,1};
//byte B4_sequence[16]={1,0,1,0 ,1,0,1,0 ,1,0,1,0 ,1,1,1,1};
byte B2_sequence[128] = {};
byte B3_sequence[128] = {};
byte B1_sequence[128] = {};
byte B4_sequence[128] = {
  1, 0, 0, 0 , 0, 0, 0, 0 , 1, 0, 0, 0 , 0, 0, 0, 0 , 1, 0, 0, 0 , 0, 0, 0, 0 , 1, 0, 0, 0 , 0, 0, 0, 0,
  1, 0, 0, 0 , 0, 0, 0, 0 , 1, 0, 0, 0 , 0, 0, 0, 0 , 1, 0, 0, 0 , 0, 0, 0, 0 , 1, 0, 0, 0 , 0, 0, 0, 0,
  0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0,
  1, 1, 1, 1 , 1, 1, 1, 1 , 1, 1, 1, 1 , 1, 1, 1, 1 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0,
};

int B2_freq_sequence[128] = {
  0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0,
  0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0,
  0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0,
  0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0,

};
int B1_freq_sequence[128] = {};
int sample_sum;
int j, k, freq3, cc;
int kf, pf, holdkf, kfe;
int shiftcount = 0;
int t1, c1, count1, dd;
byte noise_type;
uint16_t index, index2, index3, index4, index5, index4b, index_freq_1, index_freq_2, index4bv;
uint16_t indexr, index2r, index3r, index4r, index4br, index2vr, index4vr;
int osc, oscc;
byte ledstep;
byte noise_mode = 1;
unsigned long freq, freq2;

int wavepot, lfopot, arppot;

byte loopstep = 0;
byte loopstepf = 0;

byte recordbutton, prevrecordbutton, record, looptrigger, prevloopstep, revbutton, prevrevbutton, preva, prevb;
int looprate;
//int clapbank[16]={1,0,0,0,1,0,1,0,1,1,1,1,0,0,0,0};
long prev, prev2, prev3;
byte playmode = 1;
byte play = 0;
byte playbutton, pplaybutton;
byte prevbanktrigger, banktrigger;
byte pkbutton, kbutton, B4_trigger, B4_latch, cbutton, pcbutton, B4_loop_trigger, B1_trigger, kick, B1_latch, clap, B1_loop_trigger, B4_seq_trigger, B3_seq_trigger;
byte ptbutton, tbutton, ttriger, B1_seq_trigger, B3_latch, B2_trigger, bc, B2_loop_trigger, B3_loop_trigger;
byte B2_latch, B3_trigger, B2_seq_trigger, pbutton, ppbutton;
byte kicktriggerv, B2_seq_latch, kickseqtriggerv,  B1_seq_latch, pewseqtriggerv, precordbutton;
byte measure, half;
byte recordmode = 1;
byte tap, tapbutton, ptapbutton, eigth;
long tapholder, prevtap;
unsigned long taptempo = 8000000;
unsigned long ratepot;
byte r, g, b, erase, e, preveigth;
//Bounce bouncertap = Bounce(TAP, 100);
byte trigger_input, trigger_output,   trigger_out_latch, tl;
byte onetime = 1;
//Bounce bouncer1 = Bounce(2, 200); //not actuall 2 seconds since timers are running at 64kHz.
//Bounce bouncer4 = Bounce(18, 200);
//Bounce bouncer2 = Bounce(19, 200);
//Bounce bouncer3 = Bounce( 17, 200);
byte button1, button2, button3, button4, tapb;
byte pbutton1, pbutton2, pbutton3, pbutton4, ptapb;
byte prev_trigger_in_read, trigger_in_read, tiggertempo, trigger_step, triggerled, ptrigger_step;

byte bf1, bf2, bf3, bf4, bft;
uint16_t midicc3 = 128;
uint16_t midicc4 = 157;
uint16_t midicc1, midicc2, midicc5, midicc6, midicc7, midicc8;

byte midi_note_check;

byte prevshift, shift_latch;
byte tick;
byte t;
long tapbank[4];
//int what,pwhat;
byte  mnote, mvelocity, miditap, pmiditap, miditap2, midistep, pmidistep, miditempo, midinoise;

unsigned long recordoffsettimer, offsetamount, taptempof;
int potX;

#define PLAY 8
#define REC 19
#define TAP 18
#define SHIFT 17

// #define REC 28
// #define TAP 27
// #define SHIFT 26


#define red_pin 6
#define blue_pin 2
#define green_pin 3
#define yellow_pin 4

#define LED_GREEN 9

void setup() {
  //dac.setGain(1);


  // analogWrite(9,2); //Blue


  randomSeed(analogRead(0));
  // delay(100);


  //  delay(200);
  // MIDI.setHandleControlChange(cc1);
  //  MIDI.setHandleNoteOn(noteon);
  //  MIDI.setHandleNoteOff(noteOff);

  cli();
  taptempo = 4000000;

  pinMode (12, OUTPUT); pinMode (13, OUTPUT); pinMode (11, OUTPUT); pinMode (10, OUTPUT);
  pinMode (9, OUTPUT); pinMode (5, OUTPUT);  pinMode (LED_GREEN, OUTPUT);
  pinMode (16, OUTPUT);

  pinMode (PLAY, INPUT_PULLUP);   
  pinMode (REC, INPUT_PULLUP);    
  pinMode (TAP, INPUT_PULLUP);    
  pinMode (SHIFT, INPUT_PULLUP);  
  pinMode (12, INPUT);   

  pinMode (green_pin, INPUT_PULLUP);   //low left clap green
  pinMode (yellow_pin, INPUT_PULLUP);   // low right kick yellow
  pinMode (blue_pin, INPUT_PULLUP);    //Up Right tom Blue
  pinMode (red_pin, INPUT_PULLUP);   // Up right pew red

  debouncerGreen.attach(green_pin);
  debouncerGreen.interval(2); // interval in ms
  debouncerYellow.attach(yellow_pin);
  debouncerYellow.interval(2); // interval in ms  
  debouncerBlue.attach(blue_pin);
  debouncerBlue.interval(2); // interval in ms
  debouncerRed.attach(red_pin);
  debouncerRed.interval(2); // interval in ms

  delay(100);

  if (digitalRead(green_pin) == LOW) {
    analogWrite(LED_GREEN, 64); //green
    MIDI.begin(3);
    delay(20000);

  }
  else if (digitalRead(red_pin) == LOW) {
    analogWrite(5, 64); //RED
    MIDI.begin(1);
    delay(20000); // we're messing with the timers so this isn't actually 20000 Millis

  }
  else if (digitalRead(blue_pin) == LOW) {
    analogWrite(9, 64); //Blue
    MIDI.begin(2);
    delay(20000);

  }
  else if (digitalRead(yellow_pin) == LOW) {
    analogWrite(5, 48); //yellow
    analogWrite(LED_GREEN, 16);

    MIDI.begin(4);
    delay(20000);

  }

  else {
    MIDI.begin(0);
  }

  MIDI.turnThruOff();
  
  //pinMode (16, INPUT); digitalWrite (16, HIGH);
  digitalWrite(16, HIGH);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  /* Enable interrupt on timer2 == 127, with clk/8 prescaler. At 16MHz,
     this gives a timer interrupt at 15625Hz. */
  TIMSK2 = (1 << OCIE2A);
  OCR2A = 128;
  //OCR2B = 127;

  /* clear/reset timer on match */
  TCCR2A = 1 << WGM21 | 0 << WGM20; /* CTC mode, reset on match */
  TCCR2B = 0 << CS22 | 1 << CS21 | 1 << CS20; /* clk, /8 prescaler */

  //dac
  //   SPCR = 0x50;
  //   SPSR = 0x01;
  //   DDRB |= 0x2E;
  ///   PORTB |= (1<<1);

  //pwm
  TCCR0B = B0000001;
  TCCR1B = B0000001;

  //    TCCR0B = TCCR0B & 0b11111000 | 0x03;
  //   TCCR1B = TCCR1B & 0b11111000 | 0x03;


  sei();
  if (digitalRead(SHIFT) == 0) {
    noise_mode = 1;
  }
  else {
    noise_mode = 0;
  }


}








void loop() {

  midi_note_check = midi_note_on();

  pbutton1 = button1;
  pbutton2 = button2;
  pbutton3 = button3;
  pbutton4 = button4;

  debouncerRed.update();
  debouncerBlue.update();
  debouncerGreen.update();
  debouncerYellow.update();
  
  button1 = debouncerRed.read();
  // button1 = digitalRead(2);
  
  button2 = debouncerBlue.read();
  //button2 = digitalRead(19);
  
  button3 = debouncerGreen.read();
  //button3 = digitalRead(17);
  
  button4 = debouncerYellow.read();
  //button4 = digitalRead(18);
  
  tapb = digitalRead(TAP);

  if (button1 == 0 && pbutton1 == 1) {
    bf1 = 1;
  }
  else {
    bf1 = 0;
  }
  if (button2 == 0 && pbutton2 == 1) {
    bf2 = 1;
  }
  else {
    bf2 = 0;
  }

  if (button3 == 0 && pbutton3 == 1) {
    bf3 = 1;
  }
  else {
    bf3 = 0;
  }
  if (button4 == 0 && pbutton4 == 1) {
    bf4 = 1;
  }
  else {
    bf4 = 0;
  }
  if (tapb == 0 && ptapb == 1) {
    bft = 1;
  }
  else {
    bft = 0;
  }
  if (midi_note_check == 58) {
    miditap2 = 1;
    //   digitalWrite(5,HIGH);
  }
  else {
    //      digitalWrite(9,HIGH);
    miditap2 = 0;
  }
  if (midi_note_check == 57) {
    midistep = 1;
    miditempo = 1;
    digitalWrite(5, HIGH);
  }

  else {
    //      digitalWrite(9,HIGH);
    midistep = 0;
  }

  if (midi_note_check == 67) {

    play++;
    play %= 2;
    //   digitalWrite(5,HIGH);
  }

  if (midi_note_check == 69) {
    playmode++;
    playmode %= 2;
    //   digitalWrite(5,HIGH);
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



  ptapb = tapb;
  pmiditap = miditap;
  pmidistep = midistep;

  LEDS();
  BUTTONS();
  RECORD();

  if (noise_mode == 0) {
    // USE DAM POT MAPPINGS BECAUSE IT'S FATTER
    pot1 = ((analogRead(1)) >> 2) + 2;
    pot2 = ((analogRead(0)) >> 1) + 40;
  }

  if (noise_mode == 1) {

    if (midinoise == 1) {
      pot1 = (midicc1 >> 1) + 1;
      pot2 = (midicc2 >> 2) + 1;
      pot3 = (midicc3 + 1) << 4;
      pot4 = (midicc4 + 1) << 2;

    }
    if (midinoise == 0) {


      if (shift_latch == 0) {
        pot1 = ((analogRead(1)) >> 1) + 1;
        pot2 = ((analogRead(0)) >> 2) + 1;
      }
      if (shift_latch == 1) {

        pot3 = (analogRead(1)) << 2; ////////////////MAKE ME BETTERERER
        pot4 = analogRead(0) << 3;

      }
    }
  }

  trigger_in_read = digitalRead(16);

  if (trigger_in_read == 1 && prev_trigger_in_read == 0) {
    trigger_input = 1;
  }
  else {
    trigger_input = 0;

  }
  prev_trigger_in_read = trigger_in_read;

  eigth = loopstep % 4;

  if (tiggertempo == 0) {

    if (eigth == 0) {
      digitalWrite(12, HIGH);
      // tl++;
    }
    else {
      digitalWrite(12, LOW);
    }

  }

  //////////////////////////////////////////// intput trigger



  prev_trigger_in_read = trigger_in_read;

  trigger_in_read = digitalRead(12);

  if (trigger_in_read == 0 && prev_trigger_in_read == 1) {
    tiggertempo = 1;
    trigger_step = 1;
    //digitalWrite(LED_GREEN,HIGH);

  }

  else {
    trigger_step = 0;
    //digitalWrite(LED_GREEN,LOW);
  }



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

    B4_loop_trigger = B4_sequence[loopstep + banko];
    B1_loop_trigger = B1_sequence[loopstep + banko];
    B2_loop_trigger = B2_sequence[loopstep + banko];
    B3_loop_trigger = B3_sequence[loopstep + banko];
  }

  if (play == 0) {
    loopstep = 31;
    prev = 0;
    B4_loop_trigger = 0;
    B1_loop_trigger = 0;
    B2_loop_trigger = 0;
    B3_loop_trigger = 0;

  }

  if (loopstep != prevloopstep && B3_loop_trigger == 1) {

    B3_seq_trigger = 1;
    //freq3=kickfreqsequence[loopstepf];
  }
  else {
    B3_seq_trigger = 0;
  }

  if (loopstep != prevloopstep && B2_loop_trigger == 1) {

    B2_seq_trigger = 1;
    //freq3=kickfreqsequence[loopstepf];
  }
  else {
    B2_seq_trigger = 0;
  }

  if (loopstep != prevloopstep && B4_loop_trigger == 1) {

    B4_seq_trigger = 1;
    //freq3=kickfreqsequence[loopstepf];
  }
  else {
    B4_seq_trigger = 0;
  }

  if (loopstep != prevloopstep && B1_loop_trigger == 1) {

    B1_seq_trigger = 1;
  }
  else {
    B1_seq_trigger = 0;
  }



  if (B3_trigger == 1 || B3_seq_trigger == 1) {
    index3 = 0;
    accumulator3 = 0;
    B3_latch = 1;
  }

  if (B4_trigger == 1 || B4_seq_trigger == 1) {
    index4 = 0;
    accumulator4 = 0;
    B4_latch = 1;
  }
  if (B1_trigger == 1) {
    index = 0;
    accumulator = 0;
    B1_latch = 1;
  }

  if (B1_seq_trigger == 1) {
    index_freq_1 = 0;
    accu_freq_1 = 0;
    B1_seq_latch = 1;
  }
  if (B2_seq_trigger == 1) {
    index_freq_2 = 0;
    accu_freq_2 = 0;
    B2_seq_latch = 1;
  }

  if (B2_trigger == 1) {
    index2 = 0;
    accumulator2 = 0;
    B2_latch = 1;
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
      B1_freq_sequence[loopstepf + banko] = pot1;

    }

    if (B2_trigger == 1) {
      B2_sequence[loopstepf + banko] = 1;
      B2_freq_sequence[loopstepf + banko] = (pot2);
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
        B1_freq_sequence[loopstep + banko] = pot1;

      }

      if (B2_trigger == 1) {
        B2_sequence[loopstep + banko] = 1;
        B2_freq_sequence[loopstep + banko] = (pot2);
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
  //  analogWrite(9,0);
  //       analogWrite(5,0);
  analogWrite(9, bout >> 1); //Blue
  analogWrite(LED_GREEN, (gout >> 1) + triggerled); //green
  analogWrite(5, rout >> 1);

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
    prevpot2 = pot2;
    if (button1 == 0 ) { //red
      banko = 63;

    }
    if (button4 == 0) { //yellow
      banko = 31;
      bankpr = 4;
      bankpg = 4;
      bankpb = 0;
    }
    if (button2 == 0 || banko == 0) { //blue
      banko = 0;
      bankpr = 0;
      bankpg = 0;
      bankpb = 8;
    }
    if (button3 == 0) { //green
      banko = 95;
      bankpr = 0;
      bankpg = 3;
      bankpb = 0;

    }


    if (tapb == LOW) {
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
    //       if (bf1==1){

    if (bf1 == 1 || midi_note_check == 60) {
      B1_trigger = 1;
    }
    else {
      B1_trigger = 0;
    }
    //    if (bf4==1){

    if (bf4 == 1 || midi_note_check == 65) {
      B4_trigger = 1;
    }
    else {
      B4_trigger = 0;
    }
    //    if (bf2==1){

    if (bf2 == 1 || midi_note_check == 62) {
      B2_trigger = 1;
    }
    else {
      B2_trigger = 0;
    }
    //   if (bf3==1){
    if (bf3 == 1 || midi_note_check == 64) {
      B3_trigger = 1;
    }
    else {
      B3_trigger = 0;
    }

  }



  ////////////////////////////////////////////

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
          midicc1 = (MIDI.getData2() << 2) + 3;
        }

        if (MIDI.getData1() == 71) {
          midicc2 = (MIDI.getData2() << 2) + 3;
        }

        if (MIDI.getData1() == 72) {
          midicc3 = (MIDI.getData2() << 2);
        }

        if (MIDI.getData1() == 73) {
          midicc4 = (MIDI.getData2() << 2);
        }
      /*
                       if (MIDI.getData1()==74){
              midicc5 = (MIDI.getData2());
             }

                       if (MIDI.getData1()==75){
              midicc6 = (MIDI.getData2());
             }

                       if (MIDI.getData1()==76){
              midicc7 = (MIDI.getData2());
             }

                       if (MIDI.getData1()==77){
              midicc8 = (MIDI.getData2());
             }
         */


      default:
        note = 0;

    }
  }

  else {
    note = 0;
  }

  return note;
}
