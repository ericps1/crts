phy_tx_t = phy_tx_t - phy_tx_t(1);
	
figure;
plot(phy_tx_t, phy_tx_numSubcarriers);
title('Number of subcarriers');	
xlabel('Time (s)');
ylabel('Number of Subcarriers');
xlim([min(phy_tx_t) max(phy_tx_t)]);

figure;
plot(phy_tx_t, phy_tx_cp_len);
title('Length of Cyclic Prefix');	
xlabel('Time (s)');
ylabel('Samples');
xlim([min(phy_tx_t) max(phy_tx_t)]);

figure;
plot(phy_tx_t, phy_tx_taper_len);
title('Taper Length');	
xlabel('Time (s)');
ylabel('Samples');
xlim([min(phy_tx_t) max(phy_tx_t)]);

figure;
plot(phy_tx_t, phy_tx_gain_uhd);
title('Tx USRP Gain');	
xlabel('Time (s)');
ylabel('Gain (dB)');
xlim([min(phy_tx_t) max(phy_tx_t)]);

figure;
plot(phy_tx_t, phy_tx_gain_soft);
title('Tx Soft Gain');	
xlabel('Time (s)');
ylabel('Gain (dB)');
xlim([min(phy_tx_t) max(phy_tx_t)]);

figure;
plot(phy_tx_t, phy_tx_lo_freq);
title('Tx Center Frequency');	
xlabel('Time (s)');
ylabel('Frequency (Hz)');
if(max(phy_tx_freq)-min(phy_tx_freq) > 0)
  ylim([2*min(phy_tx_freq)-max(phy_tx_freq), 2*max(phy_tx_freq)-min(phy_tx_freq)]);
end
xlim([min(phy_tx_t) max(phy_tx_t)]);

figure;
plot(phy_tx_t, phy_tx_lo_freq);
title('Tx LO Frequency');	
xlabel('Time (s)');
ylabel('Frequency (Hz)');
if(max(phy_tx_lo_freq)-min(phy_tx_lo_freq) > 0)
  ylim([2*min(phy_tx_lo_freq)-max(phy_tx_lo_freq), 2*max(phy_tx_lo_freq)-min(phy_tx_lo_freq)]);
end
xlim([min(phy_tx_t) max(phy_tx_t)]);

figure;
plot(phy_tx_t, phy_tx_dsp_freq);
title('Tx DSP Frequency');	
xlabel('Time (s)');
ylabel('Frequency (Hz)');
if(max(phy_tx_dsp_freq)-min(phy_tx_dsp_freq) > 0)
  ylim([2*min(phy_tx_dsp_freq)-max(phy_tx_dsp_freq), 2*max(phy_tx_dsp_freq)-min(phy_tx_dsp_freq)]);
end
xlim([min(phy_tx_t) max(phy_tx_t)]);

figure;
plot(phy_tx_t, phy_tx_rate);
title('Tx Rate');	
xlabel('Time (s)');
ylabel('Tx Rate (Hz)');
xlim([min(phy_tx_t) max(phy_tx_t)]);

figure;
plot(phy_tx_t, phy_tx_mod_scheme);
title('Modulation Scheme');
xlabel('Time (s)');
ylabel('Modulation Scheme');
mod_sets = [0,0; 1,8; 9,16; 17,24; 25,31; 32,38; 39,40; 41,41; 42,43; 44,44; 45,49; 50,50; 51,51];
for i = 1:numel(mod_sets)/2
  if ((mod_sets(i,1)-max(phy_tx_mod_scheme) <= 0) && (mod_sets(i,2)-max(phy_tx_mod_scheme) >= 0))
    ymod_max = mod_sets(i,2)+0.5;
  end
  if ((mod_sets(i,1)-min(phy_tx_mod_scheme) <= 0) && (mod_sets(i,2)-min(phy_tx_mod_scheme) >= 0))
    ymod_min = mod_sets(i,1)-0.5;
  end
end
ylim([ymod_min ymod_max]);
mod_labels = {'Unknown', ...
  'PSK2','PSK4','PSK8','PSK16','PSK32','PSK64','PSK128','PSK256', ...
  'DPSK2','DPSK4','DPSK8','DPSK16','DPSK32','DPSK64','DPSK128','DPSK256', ...
  'ASK2','ASK4','ASK8','ASK16','ASK32','ASK64','ASK128','ASK256', ...
  'QAM4', 'QAM8', 'QAM16', 'QAM32', 'QAM64', 'QAM128', 'QAM256', ...
  'APSK4','APSK8','APSK16','APSK32','APSK64','APSK128','APSK256', ...
  'BPSK','QPSK', ...
  'OOK', ...
  'SQAM32','SQAM128', ...
  'V29', ...
  'Optimal QAM16','Optimal QAM32','Optimal QAM64','Optimal QAM128','Optimal QAM256', ...
  'VT Logo', ...
  'Aribtrary Modulation'};
set(gca, 'YTick', 0:52, 'YTickLabel', mod_labels);
xlim([min(phy_tx_t) max(phy_tx_t)]);

figure;
plot(phy_tx_t,phy_tx_fec0);
title('Inner Forward Error Correction');
xlabel('Time (s)');
ylabel('FEC Scheme');
fec_sets = [0,0; 1,1; 2,3; 4,6; 7,7; 8,10; 11,26; 27,27];
for i = 1:numel(fec_sets)/2
  if ((fec_sets(i,1)-max(phy_tx_fec0) <= 0) && (fec_sets(i,2)-max(phy_tx_fec0) >= 0))
    yfec_max = fec_sets(i,2)+0.5;
  end
  if ((fec_sets(i,1)-min(phy_tx_fec0) <= 0) && (fec_sets(i,2)-min(phy_tx_fec0) >= 0))
    yfec_min = fec_sets(i,1)-0.5;
  end
end
ylim([yfec_min yfec_max]);
fec_labels = {'Unknown', ...
  'None', ...
  'Repeat (1/3)','Repeat(1/5)', ...
  'Hamming (7/4)','Hamming (8/4)','Hamming (12/8)', ...
  'Golay (24/12)', ...
  'SEC-DED (22/16)','SEC-DED (39/32)','SEC-DED72/64)', ...
  'Convultional (1/2,7)','Convolutional (1/2,9)','Convolutional (1/3,9)', 'Convolutional (1/6,15)', ...
  'Convolutional (2/3,7)','Convolutional (3/4,7)', 'Convolutional (4/5,7)','Convolutional (5/6,7)', ...
  'Convolutional(6/7,7)','Convolutional (7/8,7)','Convolutional (2/3,9)','Convolutional (3/4,9)', ...
  'Convolutional (4/5,9)','Convolutional (5/6,9)','Convolutional(6/7,9)','Convolutional(7/8,9)', ...
  'Reed-Solomon (8)'};
set(gca, 'YTick', 0:27, 'YTickLabel', fec_labels);
xlim([min(phy_tx_t) max(phy_tx_t)]);

figure;
plot(phy_tx_t,phy_tx_fec1);
title('Outter Forward Error Correction');
xlabel('Time (s)');
ylabel('FEC Scheme');
for i = 1:numel(fec_sets)/2
  if ((fec_sets(i,1)-max(phy_tx_fec1) <= 0) && (fec_sets(i,2)-max(phy_tx_fec1) >= 0))
    yfec_max = fec_sets(i,2)+0.5;
  end
  if ((fec_sets(i,1)-min(phy_tx_fec1) <= 0) && (fec_sets(i,2)-min(phy_tx_fec1) >= 0))
    yfec_min = fec_sets(i,1)-0.5;
  end
end
ylim([yfec_min yfec_max]);
set(gca, 'YTick', 0:27, 'YTickLabel', fec_labels);
xlim([min(phy_tx_t) max(phy_tx_t)]);



