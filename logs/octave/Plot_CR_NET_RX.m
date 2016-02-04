% redefine time based on first time instant
net_rx_t = net_rx_t - net_rx_t(1);

% define parameters used to calculate instantaneous throughput
t_step = (net_rx_t(end)-net_rx_t(1))/1e4;
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

% create a normal moving average filter
b = [0.5, 0.5];
h = conv(b,b);
for i = 1:3999
  h = conv(h,b);
end
taps = length(h);

% filter throughput
net_throughput = filter(h, 1, [net_throughput zeros(1,taps)]);

% remove filter delay
net_throughput = net_throughput(1+floor(taps/2):end-ceil(taps/2));

% plot
figure;
plot(t,net_throughput);
title('Throughput vs. Time');
xlabel('Time (s)');
ylabel('Throughput (bps)');
xlim([net_rx_t(1), net_rx_t(end)]);
