phy_rx_t = t - t(1);

figure;
stem(phy_rx_t,phy_rx_Control_valid);
title('Valid Control Messages');
xlabel('Time (s)');
ylabel('Valid Control');
ylim([-1 2]);

figure;
stem(phy_rx_t,phy_rx_Payload_valid);
title('Valid Payloads');
xlabel('Time (s)');
ylabel('Valid Payload');
ylim([-1 2]);

figure;
plot(phy_rx_t,phy_rx_EVM);
title('Error Vector Magnitide');
xlabel('Time (s)');
ylabel('EVM (dB)');
	
figure;
plot(phy_rx_t,phy_rx_RSSI);
title('Received Signal Strength Indicator');
xlabel('Time (s)');
ylabel('RSSI (dB)');
	
figure;
plot(phy_rx_t,phy_rx_CFO);
title('Carrier Frequency Offset');
xlabel('Time (s)');
ylabel('CFO (f/fs)');
	
figure;
plot(phy_rx_t,phy_rx_num_syms);
title('Number of Frame Symbols');
xlabel('Time (s)\");');
ylabel('Number of Frame Symbols');
	
figure;
plot(phy_rx_t,phy_rx_mod_scheme);
title('Modulation Scheme');	
xlabel('Time (s)');
ylabel('Modulation Scheme');
ylim([25 30]);
labels = {'Unknown','PSK2','PSK4','PSK8','PSK16','PSK32','PSK64','PSK128','PSK256','DPSK2','DPSK4','DPSK8','DPSK16','DPSK32','DPSK64','DPSK128','DPSK256','ASK2','ASK4','ASK8','ASK16','ASK32','ASK64','ASK128','ASK256','QAM4', 'QAM16', 'QAM32', 'QAM64', 'QAM128', 'QAM256','APSK2','APSK4','APSK8','APSK16','APSK32','APSK64','APSK128','APSK256','BPSK','QPSK','OOK','SQAM32','SQAM128','V29','Optimal QAM16','Optimal QAM32','Optimal QAM64','Optimal QAM128','Optimal QAM256','VT Logo'};
set(gca, 'YTick', 0:52, 'YTickLabel', labels);

figure;
plot(phy_rx_t,phy_rx_BPS);
title('Bits Per Symbol');
xlabel('Time (s)');
ylabel('Bits Per Symbol');
ylim([0, max(phy_rx_BPS)+1]);

figure;
plot(phy_rx_t,phy_rx_fec0);
title('Inner Forward Error Correction');
xlabel('Time (s)');
ylabel('FEC Scheme');
labels = {'Unknown','None','Repeat (1/3)','Repeat(1/5)','Hamming (7/4)','Hamming (8/4)','Hamming (12/8)','Golay (24/12)','SEC-DED (22/16)','SEC-DED (39/32)','SEC-DED72/64)','Convultional (1/2,7)','Convolutional (1/2,9)','Convolutional (1/3,9)','Convolutional (1/6,15)','Convolutional (2/3,7)','Convolutional (3/4,7)','Convolutional (4/5,7)','Convolutional (5/6,7)','Convolutional(6/7,7)','Convolutional (7/8,7)','Convolutional (2/3,9)','Convolutional (3/4,9)','Convolutional (4/5,9)','Convolutional (5/6,9)','Convolutional(6/7,9)','Convolutional(7/8,9)','Reed-Solomon (8)'};
set(gca, 'YTick', 0:28, 'YTickLabel', labels);

figure;
plot(phy_rx_t,phy_rx_fec1);
title('Outter Forward Error Correction');
xlabel('Time (s)');
ylabel('FEC Scheme');
labels = {'Unknown','None','Repeat (1/3)','Repeat(1/5)','Hamming (7/4)','Hamming (8/4)','Hamming (12/8)','Golay (24/12)','SEC-DED (22/16)','SEC-DED (39/32)','SEC-DED72/64)','Convultional (1/2,7)','Convolutional (1/2,9)','Convolutional (1/3,9)','Convolutional (1/6,15)','Convolutional (2/3,7)','Convolutional (3/4,7)','Convolutional (4/5,7)','Convolutional (5/6,7)','Convolutional(6/7,7)','Convolutional (7/8,7)','Convolutional (2/3,9)','Convolutional (3/4,9)','Convolutional (4/5,9)','Convolutional (5/6,9)','Convolutional(6/7,9)','Convolutional(7/8,9)','Reed-Solomon (8)'};
set(gca, 'YTick', 0:28, 'YTickLabel', labels);
	
	
