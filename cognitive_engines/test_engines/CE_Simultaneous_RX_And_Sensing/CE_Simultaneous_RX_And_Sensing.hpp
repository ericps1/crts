#ifndef _CE_SIMULTANEOUS_RX_AND_SENSING__
#define _CE_SIMULTANEOUS_RX_AND_SENSING_

#include <liquid/liquid.h>
#include <sys/time.h>
#include "extensible_cognitive_radio.hpp"
#include "cognitive_engine.hpp"
#include "timer.h"
#include <complex.h>
#include <complex>

class CE_Simultaneous_RX_And_Sensing : public CognitiveEngine {

private:
  // sensing parameters
  static constexpr float sensing_delay_ms = 1e3;
  static constexpr int fft_length = 512;
  static constexpr int fft_averaging = 10;

  // timer to start and stop sensing
  timer t;
  long int sense_time_s;
  long int sense_time_us;

  // counter for fft averaging
  int fft_counter;

  // USRP, FFT output, and average FFT  sample buffers
  float _Complex buffer[fft_length];
  float _Complex buffer_F[fft_length];
  float fft_avg[fft_length];

  // fft plan for spectrum sensing
  fftplan fft;

public:
  CE_Simultaneous_RX_And_Sensing(int argc, char ** argv, ExtensibleCognitiveRadio *_ECR);
  ~CE_Simultaneous_RX_And_Sensing();
  virtual void execute();
};

#endif
