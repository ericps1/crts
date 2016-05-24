prompt = ...
sprintf(['\nEnter either the name(s) of the specific scenario(s) you are interested\n', ...
         'in or ''all'' as a cell array of strings. e.g. {''scenario_1''} OR\n', ...
         '{''scenario_1'', ''scenario_2'', ''scenario_3''} OR {''all''}\n\n']);
scenarios = input(prompt);

prompt = ...
sprintf(['\nThis tool will calculate statistics for the sum throughput achieved by\n', ...
        'the executed test scenarios. Since the test scenarios may involve multiple\n', ...
        'coexisting networks, it may be useful to look at the sum throughput for\n', ...
        'these networks individually. To do so, enter a cell array where each\n',...
        'element of the array contains the indices of a particular network\n', ...
        'e.g. {[1:2],[3:4]} for a network between nodes 1&2 and another between 3&4.\n\n']);

networks = input(prompt);
num_networks = numel(networks);

%%

if (strcmp(scenarios{1},'all'))
  num_scenarios = numel(scenario_name);
  max_num_nodes = max(num_nodes);

  scenario_name_ = scenario_name;
  num_nodes_ = num_nodes;
  run_time_ = run_time;
  bytes_received_ = bytes_received;
  bytes_sent_ = bytes_sent;
else
  num_scenarios = numel(scenarios);
  max_num_nodes = 0;

  % determine indices of selected scenarios
  for i = 1:num_scenarios
    scenario_inds(i) = find(strcmp(scenario_name,scenarios{i}));
    max_num_nodes = max([max_num_nodes, num_nodes(scenario_inds(i))]);
  end

  % define data for selected scenarios
  scenario_name_ = scenario_name(scenario_inds);
  num_nodes_ = num_nodes(scenario_inds);
  run_time_ = run_time(scenario_inds);
  bytes_received_ = bytes_received(scenario_inds,1:max_num_nodes,:);
  bytes_sent_ = bytes_sent(scenario_inds,:,:);
end

for i = 1:num_scenarios
  throughput(i,:,:) = 8*bytes_received_(i,:,:)/run_time_(i);
end

%%
for i = 1:num_networks
  scenario_mean_sum_throughput{i} = mean(sum(throughput(:,networks{i},:),2),3);
  scenario_var_sum_throughput{i} = var(sum(throughput(:,networks{i},:),2),0,3);
  scenario_std_dev_sum_throughput{i} = sqrt(scenario_var_sum_throughput{i});
  scenario_cov_sum_throughput{i} = scenario_std_dev_sum_throughput{i}./scenario_mean_sum_throughput{i};
end

output = sprintf('\n=================================================================\n');
output = sprintf([output,'Per Scenario Summary (taken over all repetitions):\n']);
disp(output);
for i = 1:num_scenarios
  output = sprintf('scenario %i: %s\n', i, scenario_name_{i});
  for j = 1:num_networks
    if (sum((networks{j} <= num_nodes_(i))))
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
  end
  output = sprintf([output, '\n']);
  disp(output);
end

%%
for i = 1:max_num_nodes
  scenario_inds = find(num_nodes_ >= i);
  node_mean_throughput = mean(mean(throughput(scenario_inds,:,:),1),3);
  node_var_throughput = mean(mean(throughput(scenario_inds,:,:).^2,1),3) - ...
                        node_mean_throughput.^2;
  node_std_dev_throughput = sqrt(node_var_throughput);
  node_cov_throughput = node_std_dev_throughput./node_mean_throughput;
end

output = sprintf('=================================================================\n');
output = sprintf([output,'Per Node Summary\n']); 
output = sprintf([output,'(taken over all repetitions of scenarios for which the node was active):\n']);
disp(output);
for i = 1:max_num_nodes
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

