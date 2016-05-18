#include "../include/FEC_Select.hpp"

int fec_select(float SNR_dB) {
  if (SNR_dB > 24.4)
    return LIQUID_FEC_CONV_V27P78;
  else if (SNR_dB > 22.7)
    return LIQUID_FEC_CONV_V27P67;
  else if (SNR_dB > 18.2)
    return LIQUID_FEC_CONV_V27P56;
  else if (SNR_dB > 16.4)
    return LIQUID_FEC_CONV_V27P45;
  else if (SNR_dB > 11.2)
    return LIQUID_FEC_CONV_V27P34;
  else if (SNR_dB > 9.4)
    return LIQUID_FEC_CONV_V27P23;
  else
    return LIQUID_FEC_CONV_V27;
}
