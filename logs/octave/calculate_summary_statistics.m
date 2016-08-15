prompt = ...
sprintf(['\nEnter the group(s) of scenarios you wish to analyze. This can be useful\n', ...
         'to compare a set of scenarios that used cognitive engine A vs. another set\n', ...
         'which used cognitive engine B for example. The scenarios groups should be\n', ...
         'cell arrays within one enclosing cell array. Wildcards are supported. So for\n', ...
         'example, assume you had four scenarios called CE_A_scenario_1, CE_A_scenario_2,\n', ...
         'CE_B_scenario_1, and CE_B_scenario_2. You could compare the statistical\n', ...
         'performance of cognitive engine A and B by providing the following argument:\n', ...
         '{{''*A*''}, {''*B*''}}. This would include both scenarios 1 and 2\n']);
scenarios = input(prompt);

prompt = ...
sprintf(['\nThis tool will calculate statistics for the sum throughput achieved by\n', ...
        'the executed test scenarios. Since the test scenarios may involve multiple\n', ...
        'coexisting networks, it may be useful to look at the sum throughput for\n', ...
        'these networks individually. To do so, enter a cell array where each\n',...
        'element of the array contains the indices of a particular network\n', ...
        'e.g. {[1:2],[3:4]} for a network between nodes 1&2 and another between 3&4.\n', ...
        'You can look at each node individually by assigning it to its own network e.g.\n', ...
        '{1,2,3,4}\n']);

networks = input(prompt);
num_networks = numel(networks);

%%

% iterative through groups of scenarios
for sg_i = 1:numel(scenarios)

  % translate a regular expression into a list of scenarios
  scenarios_in_group = {};
  for i= 1:numel(scenarios{sg_i})
    matching_scenarios_temp = ... 
        regexp(scenario_name, regexptranslate('wildcard', scenarios{sg_i}{i}), 'match');
    num_matched = 0;
    for j = 1:numel(matching_scenarios_temp)
      if (numel(matching_scenarios_temp{j})>0)
        num_matched = num_matched + 1;
        matching_scenarios{num_matched} = matching_scenarios_temp{j}{1};
      end
    end
    if (numel(matching_scenarios)>0)
      scenarios_in_group = [scenarios_in_group, matching_scenarios];
    else
      disp(['No matching scenario was found for ', scenarios{sg_i}{i}]);
      return;
    end
  end

  num_scenarios = numel(matching_scenarios);
  max_num_nodes = 0;

  % determine indices of selected scenarios
  scenario_inds = [];
  for i = 1:num_scenarios
    scenario_inds = [scenario_inds, find(strcmp(scenario_name,matching_scenarios{i}))];
    max_num_nodes = max([max_num_nodes, num_nodes(scenario_inds(i))]);
  end

  % define data for selected scenarios
  _scenario_name = scenario_name(scenario_inds);
  _num_nodes = num_nodes(scenario_inds);
  _run_time = run_time(scenario_inds);
  _bytes_received = bytes_received(scenario_inds,1:max_num_nodes,:);
  _bytes_sent = bytes_sent(scenario_inds,:,:);

  for i = 1:num_scenarios
    throughput(i,:,:) = 8*_bytes_received(i,:,:)/_run_time(i);
  end

  %%%%%%%

  for i = 1:num_networks
    mean_sum_throughput = mean(mean(sum(throughput(:,networks{i},:),2),3),1);
    var_sum_throughput = mean(var(sum(throughput(:,networks{i},:),2),0,3));
    std_dev_sum_throughput = sqrt(var_sum_throughput);
    cov_sum_throughput = std_dev_sum_throughput./mean_sum_throughput;
    output = sprintf('\n=================================================================\n');
    output = sprintf([output,'Summary of scenario group %i, network %i:\n'],sg_i,i);
    output = sprintf([output,'  Scenarios:\n']);
    for j = 1:num_scenarios
      output = sprintf([output,'    %s\n'], matching_scenarios{j});
    end
    output = sprintf([output,'  Nodes: %s\n'], num2str(networks{i}));
    output = sprintf([output,'  --------------------------------------------------\n']);
    output = sprintf([output,'  Mean of sum throughput:                     %.3e\n'], 
                     mean_sum_throughput);
    output = sprintf([output,'  Variance of sum throughput:                 %.3e\n'], 
                     var_sum_throughput);
    output = sprintf([output,'  Standard deviation of sum throughput:       %.3e\n'], 
                     std_dev_sum_throughput);
    output = sprintf([output,'  Coefficient of variation of sum throughput: %.3e'], 
                     cov_sum_throughput);
    disp(output);
  end

end

disp('');
