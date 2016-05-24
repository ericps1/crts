plot(ECR_tx_t, ECR_tx_freq/1e6, 'b', Int_tx_t, Int_tx_freq/1e6, 'r', 'LineWidth', 2);
title('Cognitive Radio and Interferer Transmit Frequencies', 'FontSize', 12);
xlabel('Time (s)', 'FontSize', 12);
ylabel('Frequency (MHz)', 'FontSize', 12);
legend('Cognitive Radio', 'Interferer', 'FontSize', 12);
ylim([2*min(ECR_tx_freq)-max(ECR_tx_freq), 2*max(ECR_tx_freq)-min(ECR_tx_freq)]/1e6);

% calculate and plot evacuation time
j = 1;
Int_tx_freq_list(1) = Int_tx_freq(1);
for i=1:length(Int_tx_freq)-1
    if Int_tx_freq(i+1) ~= Int_tx_freq(i)
        Int_tx_freq_list(j+1) = Int_tx_freq(i+1);
        Int_switch_time(j) = Int_tx_t(i+1);
        j = j+1;
    end
end

Int_switch_time = [Int_switch_time 1e10];

j = 1;
Int_current_tx_freq = Int_tx_freq_list(1);
Int_switch_count = 1;
right_decisions = 0;
wrong_decisions = 0;
first_switch = 0;
evac_time(1) = 0;
for i=1:length(ECR_tx_freq)-1
    
    % update current situation
    if(ECR_tx_t(i) > Int_switch_time(Int_switch_count))
        Int_switch_count = Int_switch_count + 1
        Int_current_tx_freq = Int_tx_freq_list(Int_switch_count);
        first_switch = 0
    end
    
    % when the CR made the right decision
    if ECR_tx_freq(i) == Int_tx_freq_list(Int_switch_count) &   ...
       ECR_tx_freq(i+1) ~= Int_tx_freq_list(Int_switch_count)
        
        right_decisions = right_decisions + 1;
        if first_switch == 0
            Int_switch_count
            ECR_tx_t(i+1)
            Int_switch_time(Int_switch_count-1)
            evac_time(Int_switch_count) = ECR_tx_t(i+1) - Int_switch_time(Int_switch_count-1);
            first_switch = 1
        end
    end
    
    % when the CR made the wrong decision
    if ECR_tx_freq(i) ~= Int_tx_freq_list(Int_switch_count) &   ...
       ECR_tx_freq(i+1) == Int_tx_freq_list(Int_switch_count)
        
        wrong_decisions = wrong_decisions + 1;
    end
end

% p = 1;
% for i=1:length(evac_time)
%     if evac_time(i) > 0
%         evac_time_p(p) = evac_time(i);
%     end
% end

hist(evac_time,20);
% xlim([0, 3]);

% figure;
% hist(ECR_switch_time - Int_switch_time)
% title('Evacuation Time Histogram');
% xlabel('Time (s)');





