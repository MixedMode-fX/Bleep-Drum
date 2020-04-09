ISR(TIMER2_COMPA_vect) {
  OCR2A = 40;


  sample_sum = 0;
  uint16_t index;
  if(playmode){
    for(uint8_t i=0; i<N_SAMPLES; i++){
      index = samples[i].getIndex();
      switch (i){
        case 0:
          sample_sum += pgm_read_byte(&table0[index]) - 127; 
          break;
        case 1:
          sample_sum += pgm_read_byte(&table1[index]) - 127; 
          break;
        case 2:
          sample_sum += pgm_read_byte(&table2[index]) - 127; 
          break;
        case 3:
          sample_sum += pgm_read_byte(&table3[index]) - 127; 
          break;
      };
      // sample_sum += samples[i].getSample();
    }
    // noise_sample = (((pgm_read_byte(&noise_table[(index_noise)])))) - 127;
  }
  else { //reverse

    for(uint8_t i=0; i<N_SAMPLES; i++){
      index = samples[i].getIndex();
      switch (i){
        case 0:
          sample_sum += pgm_read_byte(&table0[length0 - index]) - 127; 
          break;
        case 1:
          sample_sum += pgm_read_byte(&table1[length1 - index]) - 127; 
          break;
        case 2:
          sample_sum += pgm_read_byte(&table2[length2 - index]) - 127; 
          break;
        case 3:
          sample_sum += pgm_read_byte(&table3[length3 - index]) - 127; 
          break;
      };
      // sample_sum += samples[i].getSample();
    }


    // original had no noise in reverse ??
  }

  // TODO: left right assignement at boot or with mi
  sample_sum_b = sample_sum;

  // if (noise_mode == 1) {

  //   sample_holder1 = ((sample_sum_b + sample_sum) ^ (noise_sample >> 1)) + 127;


  //   if (B1_latch == 0 && B2_latch == 0  && B3_latch == 0  && B4_latch == 0 ) {
  //     sample = 127 ;
  //     sample_b = 127 ;
  //   }
  //   else {
  //     sample = sample_holder1;
  //     sample_b = sample_holder1;
  //   }

  // }

  // if (noise_mode == 0) {
    sample = (sample_sum) + 127;
    sample_b = (sample_sum_b) + 127;

  // }

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

  for(uint8_t i=0; i<N_SAMPLES; i++){
    samples[i].update();
  }


  // Sequencer pitch shift stuff
  // accu_freq_1 += kf;
  // index_freq_1 = (accu_freq_1 >> (6));
  // if (B1_seq_trigger == 1 && tiggertempo == 0) {
  //   kf = B1_freq_sequence[loopstepf + banko];
  //   kfe = kf;
  // }

  // if (B1_seq_trigger == 1 && tiggertempo == 1) {
  //   kf = B1_freq_sequence[loopstep + banko];
  //   kfe = kf;
  // }

  // if (index_freq_1 > tick_length) {
  //   kf = 0;
  //   index_freq_1 = 0;
  //   accu_freq_1 = 0;
  //   B1_seq_latch = 0;
  // }


  // accu_freq_2 += pf;
  // index_freq_2 = (accu_freq_2 >> (6));

  // if (B2_seq_trigger == 1 && tiggertempo == 0) {
  //   pf = B2_freq_sequence[loopstepf + banko];
  // }

  // if (B2_seq_trigger == 1 && tiggertempo == 1) {
  //   pf = B2_freq_sequence[loopstep + banko];
  // }
  // if (index_freq_2 > bass_length) {
  //   pf = 0;
  //   index_freq_2 = 0;
  //   accu_freq_2 = 0;
  //   B2_seq_latch = 0;
  // }



  // Noise mode
  // if (noise_mode == 1) {
  //   accumulator_noise += (pot3);
  //   index_noise = (accumulator_noise >> (6));
  //   if (index_noise > pot4) {
  //     index_noise = 0;
  //     accumulator_noise = 0;
  //   }
  // }
}


