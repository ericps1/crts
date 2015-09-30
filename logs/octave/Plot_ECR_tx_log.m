ECR_tx_t = ECR_tx_t - ECR_tx_t(1);
	
figure;
plot(ECR_tx_t, ECR_tx_M);
title('Number of subcarriers');	
xlabel('Time (s)');
ylabel('Number of Subcarriers');
	
figure;
plot(ECR_tx_t, ECR_tx_cp_len);
title('Length of Cyclic Prefix');	
xlabel('Time (s)');
ylabel('Samples');
	
figure;
plot(ECR_tx_t, ECR_tx_taper_len);
title('Taper Length');	
xlabel('Time (s)');
ylabel('Samples');
	
figure;
plot(ECR_tx_t, ECR_tx_gain_uhd);
title('Tx USRP Gain');	
xlabel('Time (s)');
ylabel('Gain (dB)');
	
figure;
plot(ECR_tx_t, ECR_tx_gain_soft);
title('Tx Soft Gain');	
xlabel('Time (s)');
ylabel('Gain (dB)');
	
figure;
plot(ECR_tx_t, ECR_tx_freq);
title('Tx Center Frequency');	
xlabel('Time (s)');
ylabel('Frequency (Hz)');
ylim([2*min(ECR_tx_freq)-max(ECR_tx_freq), 2*max(ECR_tx_freq)-min(ECR_tx_freq)]);

figure;
plot(ECR_tx_t, ECR_tx_rate);
title('Tx Rate');	
xlabel('Time (s)');
ylabel('Tx Rate (Hz)');
