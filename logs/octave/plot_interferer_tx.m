Int_tx_t = Int_tx_t - Int_tx_t(1);
	
figure;
plot(Int_tx_t,Int_tx_freq);
title('Transmit Frequency');	
xlabel('Time (s)');
ylabel('Frequency (Hz)');
ylim([2*min(Int_tx_freq)-max(Int_tx_freq), 2*max(Int_tx_freq)-min(Int_tx_freq)]);
	

