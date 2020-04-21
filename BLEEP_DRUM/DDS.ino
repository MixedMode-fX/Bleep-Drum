ISR(TIMER2_COMPA_vect) {
  dds_time++;
  OCR2A = sample_rate; // 40

  prevloopstep = loopstep;
  if (recordmode == 1 && miditempo == 0) {
    if (dds_time - prev > (taptempof) ) {
      prev = dds_time;
      loopstep++;
      if (loopstep > 31)
      {
        loopstep = 0;
      }
    }
  }

  if (shift == 1) {
    if (bft == 1) {
      bft = 0;
      t++;
      t %= 2;
      tapbank[t] = (dds_time - prevtap) >> 2;
      taptempo = ((tapbank[0] + tapbank[1]) >> 1);
      prevtap = dds_time;
    }
  }

  taptempof = taptempo;

  recordoffsettimer = dds_time - prev ;
  offsetamount = taptempof - (taptempof >> 2 );

  if ((recordoffsettimer) > (offsetamount))
  {
    loopstepf = loopstep + 1;
    if (loopstepf > 31) {
      loopstepf = 0;
    }
  }

  if (loopstep % 4 == 0) {
    click_pitch = 220;
  }
  if (loopstep % 8 == 0) {
    click_pitch = 293;
  }

  if (loopstep == 0) {
    click_pitch = 440;
  }


  if (play == 1) {
    click_play = 1;
    samples[0].setLoopTrigger(loopstep + banko);
    samples[1].setLoopTrigger(loopstep + banko);
    samples[2].setLoopTrigger(loopstep + banko);
    samples[3].setLoopTrigger(loopstep + banko);
  }

  if (play == 0) {
    loopstep = 31;
    prev = 0;
    click_play = 0;
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
      samples[i].trigger(1);
      samples[i].setSpeed(samples[i].getSpeedStep(loopstepf + banko), 1);
    }

  }

  if (loopstep % 4 == 0 && prevloopstep % 4 != 0) {
    click_amp = 64;
  }



  sine_sample = (((pgm_read_byte(&sine_table[index_sine]) - 127) * click_amp) >> 8) * click_play * click_en;

  sample_sum[0] = sine_sample;
  sample_sum[1] = sine_sample;

  if(playmode){
      sample_sum[OUTPUT_0] += pgm_read_byte(&table0[samples[0].getIndex()]) + pgm_read_byte(&table0[samples[0].getIndex(1)]) - 255; 
      sample_sum[OUTPUT_1] += pgm_read_byte(&table1[samples[1].getIndex()]) + pgm_read_byte(&table1[samples[1].getIndex(1)]) - 255; 
      sample_sum[OUTPUT_2] += pgm_read_byte(&table2[samples[2].getIndex()]) + pgm_read_byte(&table2[samples[2].getIndex(1)]) - 255; 
      sample_sum[OUTPUT_3] += pgm_read_byte(&table3[samples[3].getIndex()]) + pgm_read_byte(&table3[samples[3].getIndex(1)]) - 255; 

      if (noise_mode == 1) sample_noise = (((pgm_read_byte(&noise_table[(index_noise)])))) - 127;
  }
  else { //reverse

      sample_sum[OUTPUT_0] += pgm_read_byte(&table0[length0 - samples[0].getIndex()]) + pgm_read_byte(&table0[length0 - samples[0].getIndex(1)]) - 255; 
      sample_sum[OUTPUT_1] += pgm_read_byte(&table1[length1 - samples[1].getIndex()]) + pgm_read_byte(&table1[length1 - samples[1].getIndex(1)]) - 255; 
      sample_sum[OUTPUT_2] += pgm_read_byte(&table2[length2 - samples[2].getIndex()]) + pgm_read_byte(&table2[length2 - samples[2].getIndex(1)]) - 255; 
      sample_sum[OUTPUT_3] += pgm_read_byte(&table3[length3 - samples[3].getIndex()]) + pgm_read_byte(&table3[length3 - samples[3].getIndex(1)]) - 255; 
    // original had no noise in reverse ??
  }

  if (noise_mode == 1) {
    sample_holder1 = ((sample_sum[0] + sample_sum[1]) ^ (sample_noise >> 1)) + 127;

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
    sample[1] = sample[0]; // output in mono when in noise mode
  }

  else {
    sample[0] = sample_sum[0] + 127;
    sample[1] = sample_sum[1] + 127;
  }


  // DAC Writes

  // uint8_t sample_out[2] = {constrain(sample[0], 0, 255), constrain(sample[1], 0, 255)};

  uint16_t dac_out = (0 << 15) | (1 << 14) | (0 << 13) | (1 << 12) | ( sample[0] << 4 ); // v16 only does a shift by 2 and doesn't constrain
  digitalWrite(DAC_CS, LOW);
  SPI.transfer(dac_out >> 8);
  SPI.transfer(dac_out & 255);
  digitalWrite(DAC_CS, HIGH);

#ifdef STEREO
  uint16_t dac_outb = (1 << 15) | (1 << 14) | (0 << 13) | (1 << 12) | ( sample[1] << 4 );
  digitalWrite(DAC_CS, LOW);
  SPI.transfer(dac_outb >> 8);
  SPI.transfer(dac_outb & 255);
  digitalWrite(DAC_CS, HIGH);
#endif
  


  //////////////////////////////////////////////////////////////////////////////

  // CLICK
  acc_sine += click_pitch << 2 ;
  index_sine = (dds_tune * acc_sine) >> (32 - 8);
  //  index_sine = (acc_sine << 15) >> (32 - 8); //fast and the pitches arent critical
  click_wait++;
  if (click_wait > 4) {
    click_wait = 0;

    if (click_amp >= 4) {
      click_amp -= 1;
    }
    if (click_amp < 4) {
      click_amp = 0;
    }
  }



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


