ISR(TIMER2_COMPA_vect) {
  OCR2A = 40;

  sample_sum[0] = 0;
  sample_sum[1] = 0;
  if(playmode){
      sample_sum[outputs[0]] += pgm_read_byte(&table0[samples[0].getIndex()]) - 127; 
      sample_sum[outputs[1]] += pgm_read_byte(&table1[samples[1].getIndex()]) - 127; 
      sample_sum[outputs[2]] += pgm_read_byte(&table2[samples[2].getIndex()]) - 127; 
      sample_sum[outputs[3]] += pgm_read_byte(&table3[samples[3].getIndex()]) - 127; 

      sample_sum[outputs[0]] += pgm_read_byte(&table0[samples[0].getIndex(1)]) - 127; 
      sample_sum[outputs[1]] += pgm_read_byte(&table1[samples[1].getIndex(1)]) - 127; 
      if (noise_mode == 1) noise_sample = (((pgm_read_byte(&noise_table[(index_noise)])))) - 127;
  }
  else { //reverse

      sample_sum[outputs[0]] += pgm_read_byte(&table0[length0 - samples[0].getIndex()]) - 127; 
      sample_sum[outputs[1]] += pgm_read_byte(&table1[length1 - samples[1].getIndex()]) - 127; 
      sample_sum[outputs[2]] += pgm_read_byte(&table2[length2 - samples[2].getIndex()]) - 127; 
      sample_sum[outputs[3]] += pgm_read_byte(&table3[length3 - samples[3].getIndex()]) - 127; 

      sample_sum[outputs[0]] += pgm_read_byte(&table0[length0 - samples[0].getIndex(1)]) - 127; 
      sample_sum[outputs[1]] += pgm_read_byte(&table1[length1 - samples[1].getIndex(1)]) - 127; 
      // sample_sum[outputs[4]] += pgm_read_byte(&table0[length0 - samples[4].getIndex()]) - 127; 
      // sample_sum[outputs[5]] += pgm_read_byte(&table1[length1 - samples[5].getIndex()]) - 127; 

    // original had no noise in reverse ??
  }

  if (noise_mode == 1) {
    sample_holder1 = ((sample_sum[0] + sample_sum[1]) ^ (noise_sample >> 1)) + 127;

    uint8_t latch = 0;
    for (uint8_t i=0; i<4; i++){
      latch += samples[i].latched();
    }

    if (latch == 0) {
      sample[0] = 127;
    }
    else {
      sample[0] = sample_holder1;
    }
    sample[1] = sample[0];
  }

  else {
    sample[0] = (sample_sum[0]) + 127;
    sample[1] = (sample_sum[1]) + 127;
  }

  uint8_t sample_out[2] = {constrain(sample[0], 0, 255), constrain(sample[1], 0, 255)};

  uint16_t dac_out = (0 << 15) | (1 << 14) | (1 << 13) | (1 << 12) | ( sample_out[0] << 4 );
  digitalWrite(10, LOW);
  SPI.transfer(dac_out >> 8);
  SPI.transfer(dac_out & 255);
  digitalWrite(10, HIGH);

#ifdef STEREO
  uint16_t dac_outb = (1 << 15) | (1 << 14) | (1 << 13) | (1 << 12) | ( sample_out[1] << 4 );
  digitalWrite(10, LOW);
  SPI.transfer(dac_outb >> 8);
  SPI.transfer(dac_outb & 255);
  digitalWrite(10, HIGH);
#endif

  //////////////////////////////////////////////////////////////////////////////


  // update all index
  for(uint8_t i=0; i<N_SAMPLES; i++){
    samples[i].update();
  }


  // Noise mode index update
  if (noise_mode == 1) {
    accumulator_noise += noise_p1;
    index_noise = (accumulator_noise >> (6));
    if (index_noise > noise_p2) {
      index_noise = 0;
      accumulator_noise = 0;
    }
  }
}


