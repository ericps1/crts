phy_tx_t = phy_tx_t - phy_tx_t(1);
	
figure;
plot(phy_tx_t, phy_tx_numSubcarriers);
title('Number of subcarriers');	
xlabel('Time (s)');
ylabel('Number of Subcarriers');
	
figure;
plot(phy_tx_t, phy_tx_cp_len);
title('Length of Cyclic Prefix');	
xlabel('Time (s)');
ylabel('Samples');
	
figure;
plot(phy_tx_t, phy_tx_taper_len);
title('Taper Length');	
xlabel('Time (s)');
ylabel('Samples');
	
figure;
plot(phy_tx_t, phy_tx_gain_uhd);
title('Tx USRP Gain');	
xlabel('Time (s)');
ylabel('Gain (dB)');
	
figure;
plot(phy_tx_t, phy_tx_gain_soft);
title('Tx Soft Gain');	
xlabel('Time (s)');
ylabel('Gain (dB)');
	
figure;
plot(phy_tx_t, phy_tx_lo_freq);
title('Tx Center Frequency');	
xlabel('Time (s)');
ylabel('Frequency (Hz)');
if(max(phy_tx_freq)-min(phy_tx_freq) > 0)
  ylim([2*min(phy_tx_freq)-max(phy_tx_freq), 2*max(phy_tx_freq)-min(phy_tx_freq)]);
end

figure;
plot(phy_tx_t, phy_tx_lo_freq);
title('Tx LO Frequency');	
xlabel('Time (s)');
ylabel('Frequency (Hz)');
if(max(phy_tx_lo_freq)-min(phy_tx_lo_freq) > 0)
  ylim([2*min(phy_tx_lo_freq)-max(phy_tx_lo_freq), 2*max(phy_tx_lo_freq)-min(phy_tx_lo_freq)]);
end

figure;
plot(phy_tx_t, phy_tx_dsp_freq);
title('Tx DSP Frequency');	
xlabel('Time (s)');
ylabel('Frequency (Hz)');
if(max(phy_tx_dsp_freq)-min(phy_tx_dsp_freq) > 0)
  ylim([2*min(phy_tx_dsp_freq)-max(phy_tx_dsp_freq), 2*max(phy_tx_dsp_freq)-min(phy_tx_dsp_freq)]);
end

figure;
plot(phy_tx_t, phy_tx_rate);
title('Tx Rate');	
xlabel('Time (s)');
ylabel('Tx Rate (Hz)');

figure;
plot(phy_tx_t, phy_tx_mod_scheme);
title('Modulation Scheme');
xlabel('Time (s)');
ylabel('Modulation Scheme');
labels = {'Unknown','PSK2','PSK4','PSK8','PSK16','PSK32','PSK64','PSK128','PSK256','DPSK2','DPSK4','DPSK8','DPSK16','DPSK32','DPSK64','DPSK128','DPSK256','ASK2','ASK4','ASK8','ASK16','ASK32','ASK64','ASK128','ASK256','QAM4', 'QAM8', 'QAM16', 'QAM32', 'QAM64', 'QAM128', 'QAM256','APSK2','APSK4','APSK8','APSK16','APSK32','APSK64','APSK128','APSK256','BPSK','QPSK','OOK','SQAM32','SQAM128','V29','Optimal QAM16','Optimal QAM32','Optimal QAM64','Optimal QAM128','Optimal QAM256','VT Logo'};
set(gca, 'YTick', 0:52, 'YTickLabel', labels);

figure;
plot(phy_tx_t,phy_tx_fec0);
title('Inner Forward Error Correction');
xlabel('Time (s)');
ylabel('FEC Scheme');
labels = {'Unknown','None','Repeat (1/3)','Repeat(1/5)','Hamming (7/4)','Hamming (8/4)','Hamming (12/8)','Golay (24/12)','SEC-DED (22/16)','SEC-DED (39/32)','SEC-DED72/64)','Convultional (1/2,7)','Convolutional (1/2,9)','Convolutional (1/3,9)','Convolutional (1/6,15)','Convolutional (2/3,7)','Convolutional (3/4,7)','Convolutional (4/5,7)','Convolutional (5/6,7)','Convolutional(6/7,7)','Convolutional (7/8,7)','Convolutional (2/3,9)','Convolutional (3/4,9)','Convolutional (4/5,9)','Convolutional (5/6,9)','Convolutional(6/7,9)','Convolutional(7/8,9)','Reed-Solomon (8)'};
set(gca, 'YTick', 0:28, 'YTickLabel', labels);
ylim([0 28]);

figure;
plot(phy_tx_t,phy_tx_fec1);
title('Outter Forward Error Correction');
xlabel('Time (s)');
ylabel('FEC Scheme');
labels = {'Unknown','None','Repeat (1/3)','Repeat(1/5)','Hamming (7/4)','Hamming (8/4)','Hamming (12/8)','Golay (24/12)','SEC-DED (22/16)','SEC-DED (39/32)','SEC-DED (72/64)','Convultional (1/2,7)','Convolutional (1/2,9)','Convolutional (1/3,9)','Convolutional (1/6,15)','Convolutional (2/3,7)','Convolutional (3/4,7)','Convolutional (4/5,7)','Convolutional (5/6,7)','Convolutional(6/7,7)','Convolutional (7/8,7)','Convolutional (2/3,9)','Convolutional (3/4,9)','Convolutional (4/5,9)','Convolutional (5/6,9)','Convolutional(6/7,9)','Convolutional(7/8,9)','Reed-Solomon (8)'};
set(gca, 'YTick', 0:28, 'YTickLabel', labels);
ylim([0 28]);



