
int mcs_select(float SNR_dB) {
  if (SNR_dB > 24.4)
    return 6;
  else if (SNR_dB > 22.7)
    return 5;
  else if (SNR_dB > 18.2)
    return 4;
  else if (SNR_dB > 16.4)
    return 3;
  else if (SNR_dB > 11.2)
    return 2;
  else if (SNR_dB > 9.4)
    return 1;
  else
    return 0;
}
