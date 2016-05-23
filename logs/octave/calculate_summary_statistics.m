num_scenarios = numel(scenario_name);

throughput = 8*bytes_received./run_time;

%%
scenario_mean_sum_throughput = mean(sum(throughput,2),3);
scenario_var_sum_throughput = var(sum(throughput,2),0,3);
scenario_std_dev_sum_throughput = sqrt(scenario_var_sum_throughput);
scenario_cov_sum_throughput = scenario_std_dev_sum_throughput/scenario_mean_sum_throughput;

output = sprintf('\nPer Scenario Summary (taken over multiple repetitions):\n');
disp(output);
for i = 1:num_scenarios
  output = sprintf('%s:\n', scenario_name{i});
  output = sprintf([output, '  mean of sum throughput:                     %e\n'], 
                   scenario_mean_sum_throughput(i));
  output = sprintf([output, '  variance of sum throughput:                 %e\n'], 
                   scenario_var_sum_throughput(i));
  output = sprintf([output, '  standard deviation of sum throughput:       %e\n'], 
                   scenario_std_dev_sum_throughput(i));
  output = sprintf([output, '  coefficient of variation of sum throughput: %e\n'], 
                   scenario_cov_sum_throughput(i));
  disp(output);
end

%%
node_mean_throughput = mean(mean(throughput,1),3);
node_var_throughput = mean(mean(throughput.^2,1),3)-node_mean_throughput.^2;
node_std_dev_throughput = sqrt(node_var_throughput);
node_cov_throughput = node_std_dev_throughput./node_mean_throughput;

output = sprintf('\nPer Node Summary (taken over all scenarios and repetitions):\n');
disp(output);
for i = 1:max(num_nodes)
  output = sprintf('node %i:\n', i);
  output = sprintf([output, '  mean of node throughput:                     %e\n'], 
                   node_mean_throughput(i));
  output = sprintf([output, '  variance of node throughput:                 %e\n'], 
                   node_var_throughput(i));
  output = sprintf([output, '  standard deviation of node throughput:       %e\n'], 
                   node_std_dev_throughput(i));
  output = sprintf([output, '  coefficient of variation of node throughput: %e\n'], 
                   node_cov_throughput(i));
  disp(output);
end

