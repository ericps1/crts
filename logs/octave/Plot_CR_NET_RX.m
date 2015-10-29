% redefine time based on first time instant
net_rx_t = net_rx_t - net_rx_t(1);

% define parameters used to calculate instantaneous throughput
t_step = 0.01;
steps = ceil(net_rx_t(end)/t_step);
t = linspace(t_step, t_step*steps, steps);
net_throughput = zeros(1,steps);
j = 1;

% step through time calculating average throughput over the current step
for i = 1:steps
  while(net_rx_t(j) < t(i))
    net_throughput(i) = net_throughput(i) + 8*net_rx_bytes(j)/t_step;
    j = j+1;
    if(j == length(net_rx_t))
      break;
    end
  end
  if(j == length(net_rx_t))
    break
  end
end

% taking a moving average
MA_span = 1.0;
taps = round(MA_span/t_step);
net_throughput = filter(ones(1,taps)/taps, 1, [net_throughput zeros(1,taps)]);

% normalize samples at either end of the filter
for i=1:taps
  net_throughput(i) = net_throughput(i)*(taps/i);
  net_throughput(end-i+1) = net_throughput(end-i+1)*(taps/i);
end

% remove filter delay
net_throughput = net_throughput(1+floor(taps/2):end-ceil(taps/2));

% plot
figure;
plot(t,net_throughput);
title('Throughput vs. Time');
xlabel('Time (s)');
ylabel('Throughput (bps)');

