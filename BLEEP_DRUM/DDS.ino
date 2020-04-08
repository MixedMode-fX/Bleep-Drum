ISR(TIMER2_COMPA_vect) {
  OCR2A = 40;


  if(playmode){
    sample3 = (pgm_read_byte(&snare_table[(index3)])) - 127;
    sample4 = (pgm_read_byte(&kick_table[(index4)])) - 127;
    sample2 = (pgm_read_byte(&tick_table[(index2)])) - 127;
    sample1 = (((pgm_read_byte(&bass_table[(index1)])))) - 127;
    sample_freq_1 = pgm_read_byte(&tick_table[(index_freq_1)]) - 127;
    sample_freq_2 = (pgm_read_byte(&bass_table[(index_freq_2)])) - 127;

    noise_sample = (((pgm_read_byte(&noise_table[(index_noise)])))) - 127;

  }
  else { //reverse
    sample3 = (pgm_read_byte(&snare_table[(snare_length - index3)])) - 127;
    sample4 = (pgm_read_byte(&kick_table[(kick_length - index4)])) - 127;
    sample2 = (pgm_read_byte(&tick_table[(tick_length - index2)])) - 127;
    sample1 = (pgm_read_byte(&bass_table[(bass_length - index1)])) - 127;

    sample_freq_1 = pgm_read_byte(&tick_table[(tick_length - index_freq_1)]) - 127;
    sample_freq_2 = (pgm_read_byte(&bass_table[(bass_length - index_freq_2)])) - 127;

  }

  // TODO: left right assignement at boot or with midi
  sample_sum = sample3;
  sample_sum_b = sample4 + sample2 + sample1 + sample_freq_1 + sample_freq_2;

  if (noise_mode == 1) {

    sample_holder1 = ((sample_sum_b + sample_sum) ^ (noise_sample >> 1)) + 127;


    if (B1_latch == 0 && B2_latch == 0  && B3_latch == 0  && B4_latch == 0 ) {
      sample = 127 ;
      sample_b = 127 ;
    }
    else {
      sample = sample_holder1;
      sample_b = sample_holder1;
    }

  }

  if (noise_mode == 0) {
    sample = (sample_sum) + 127;
    sample_b = (sample_sum_b) + 127;

  }

  byte sample_out = constrain(sample, 0, 255);
  byte sample_out_b = constrain(sample_b, 0, 255);


  uint16_t dac_out = (0 << 15) | (1 << 14) | (1 << 13) | (1 << 12) | ( sample_out << 4 );
  digitalWrite(10, LOW);
  SPI.transfer(dac_out >> 8);
  SPI.transfer(dac_out & 255);
  digitalWrite(10, HIGH);

#ifdef STEREO
  uint16_t dac_outb = (1 << 15) | (1 << 14) | (1 << 13) | (1 << 12) | ( sample_out_b << 4 );
  digitalWrite(10, LOW);
  SPI.transfer(dac_outb >> 8);
  SPI.transfer(dac_outb & 255);
  digitalWrite(10, HIGH);
#endif

  ///////////////////////////////////////////////////////////////////////////////

  if (B1_latch == 1) {
    if (midicc1 > 4) {
      accumulator1 += midicc1;
    }
    else {
      accumulator1 += pot1;
    }
    index2 = (accumulator1 >> (6));
    if (index2 > tick_length) {

      index2 = 0;
      accumulator1 = 0;
      B1_latch = 0;
    }
  }

  if (B2_latch == 1) {
    if (midicc2 > 4) {
      accumulator2 += midicc2;
    }
    else {
      accumulator2 += pot2;
    }
    index1 = (accumulator2 >> (6));
    if (index1 > bass_length) {

      index1 = 0;
      accumulator2 = 0;
      B2_latch = 0;
    }
  }

  if (B3_latch == 1) {
    accumulator3 += (midicc3);
    index3 = (accumulator3 >> 6);
    if (index3 > snare_length) {

      index3 = 0;
      accumulator3 = 0;
      B3_latch = 0;
    }
  }

  if (B4_latch == 1) {
    accumulator4 += (midicc4);
    // index4b=(accumulator4 >> (6));
    index4 = (accumulator4 >> (6));
    if (index4 > kick_length) {

      index4 = 0;
      accumulator4 = 0;
      B4_latch = 0;
    }
  }

  accu_freq_1 += kf;
  index_freq_1 = (accu_freq_1 >> (6));
  if (B1_seq_trigger == 1 && tiggertempo == 0) {
    kf = B1_freq_sequence[loopstepf + banko];
    kfe = kf;
  }

  if (B1_seq_trigger == 1 && tiggertempo == 1) {
    kf = B1_freq_sequence[loopstep + banko];
    kfe = kf;
  }

  if (index_freq_1 > tick_length) {
    kf = 0;
    index_freq_1 = 0;
    accu_freq_1 = 0;
    B1_seq_latch = 0;
  }


  accu_freq_2 += pf;
  index_freq_2 = (accu_freq_2 >> (6));

  if (B2_seq_trigger == 1 && tiggertempo == 0) {
    pf = B2_freq_sequence[loopstepf + banko];
  }

  if (B2_seq_trigger == 1 && tiggertempo == 1) {
    pf = B2_freq_sequence[loopstep + banko];
  }
  if (index_freq_2 > bass_length) {
    pf = 0;
    index_freq_2 = 0;
    accu_freq_2 = 0;
    B2_seq_latch = 0;
  }


  if (noise_mode == 1) {
    accumulator_noise += (pot3);
    // index4b=(accumulator4 >> (6));
    index_noise = (accumulator_noise >> (6));
    if (index_noise > pot4) {
      index_noise = 0;
      accumulator_noise = 0;
    }
  }
}


