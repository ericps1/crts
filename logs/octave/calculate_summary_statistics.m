prompt = ...
sprintf(['Enter either the name(s) of the specific scenario(s) you are interested\n', ...
         'in or ''all'' as a cell array of strings. e.g. {''scenario_1''} OR\n', ...
         '{''scenario_1'', ''scenario_2'', ''scenario_3''} OR {''all''}\n\n']);
scenarios = input(prompt);

prompt = ...
sprintf(['\nThis tool will calculate statistics for the sum throughput achieved by\n', ...
        'the executed test scenarios. Since the test scenarios may involve multiple\n', ...
        'coexisting networks, it may be useful to look at the sum throughput for\n', ...
        'these networks individually. To do so, enter a cell array where each\n',...
        'element of the array contains the indices of a particular network\n', ...
        'e.g. {1:2,3:4} for a network between nodes 1&2 and another between 3&4.\n\n']);

networks = input(prompt);
num_networks = numel(networks);

%%
num_scenarios = numel(scenario_name);

throughput = 8*bytes_received./run_time;

%%
for i = 1:num_networks
  scenario_mean_sum_throughput{i} = mean(sum(throughput(:,networks{i},:),2),3);
  scenario_var_sum_throughput{i} = var(sum(throughput(:,networks{i},:),2),0,3);
  scenario_std_dev_sum_throughput{i} = sqrt(scenario_var_sum_throughput{i});
  scenario_cov_sum_throughput{i} = scenario_std_dev_sum_throughput{i}/scenario_mean_sum_throughput{i};
end

output = sprintf('\n=================================================================\n');
output = sprintf([output,'Per Scenario Summary (taken over all repetitions):\n']);
disp(output);
for i = 1:num_scenarios
  output = sprintf('%s:\n', scenario_name{i});
  for j = 1:num_networks
    output = sprintf([output, '  network %i: nodes %s\n'], j, num2str(networks{j}));
    output = sprintf([output, '    mean of sum throughput:                     %.3e\n'], 
                     scenario_mean_sum_throughput{j}(i));
    output = sprintf([output, '    variance of sum throughput:                 %.3e\n'], 
                     scenario_var_sum_throughput{j}(i));
    output = sprintf([output, '    standard deviation of sum throughput:       %.3e\n'], 
                     scenario_std_dev_sum_throughput{j}(i));
    output = sprintf([output, '    coefficient of variation of sum throughput: %.3e\n'], 
                     scenario_cov_sum_throughput{j}(i));
  end
  output = sprintf([output, '\n']);
end
disp(output);

%%
node_mean_throughput = mean(mean(throughput,1),3);
node_var_throughput = mean(mean(throughput.^2,1),3)-node_mean_throughput.^2;
node_std_dev_throughput = sqrt(node_var_throughput);
node_cov_throughput = node_std_dev_throughput./node_mean_throughput;

output = sprintf('=================================================================\n');
output = sprintf([output,'Per Node Summary (taken over all scenarios and repetitions):\n']);
disp(output);
for i = 1:max(num_nodes)
  output = sprintf('node %i:\n', i);
  output = sprintf([output, '  mean of node throughput:                     %.3e\n'], 
                   node_mean_throughput(i));
  output = sprintf([output, '  variance of node throughput:                 %.3e\n'], 
                   node_var_throughput(i));
  output = sprintf([output, '  standard deviation of node throughput:       %.3e\n'], 
                   node_std_dev_throughput(i));
  output = sprintf([output, '  coefficient of variation of node throughput: %.3e\n'], 
                   node_cov_throughput(i));
  disp(output);
end

