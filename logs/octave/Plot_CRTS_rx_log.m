CRTS_rx_t = CRTS_rx_t - CRTS_rx_t(1);
t_step = 0.01;
steps = ceil(CRTS_rx_t(end)/t_step);
t = linspace(t_step, t_step*steps, steps);
CRTS_throughput = zeros(1,steps);
j = 1;

for i = 1:steps
  while(CRTS_rx_t(j) < t(i))
    CRTS_throughput(i) = CRTS_throughput(i) + 8*CRTS_rx_bytes(j)/t_step;
    j = j+1;
    if(j == length(CRTS_rx_t))
      break;
    end
  end
  if(j == length(CRTS_rx_t))
    break
  end
end

MA_span = 2.0;
taps = round(MA_span/t_step);
CRTS_throughput = filter(ones(1,taps)/taps, 1, [CRTS_throughput zeros(1,taps)]);
CRTS_throughput = CRTS_throughput(taps+1:end);

figure;
plot(t,CRTS_throughput);
title('Throughput vs. Time');
xlabel('Time (s)');
ylabel('Throughput (bps)');

