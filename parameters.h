#pragma once

enum ControlParameters {
  // C_frequency, detune, level, octave, waveshape (button)
  OSC_1_PARAMS = 0,
  
  OSC_1_ADSR,
  //// Detune, level, semitone offset, waveshape (button)
  //OSC_2_PARAMS,
  
  //OSC_2_ADSR,

  //// intensity, nosie enable (button)
  //ADDITIVE_NOISE,

  //// C_freq, Q, filter type (button)
  //FILTER_PARAMS,

  //// Waveform, rate, intensity, DC offset. Mapping through GUI
  //LFO_PARAMS,

  //// delay length, feedback, delay enable (button)
  //DELAY_PARAMS,
  
  ////intensity, frequency, waveshape (button)
  //AM_PARAMS,

  ////intensity, frequency, waveshape (button)
  //FM_PARAMS,

  ARP,

  //GLOBAL_CONTROLS
};

// TODO: decide whether to use a class