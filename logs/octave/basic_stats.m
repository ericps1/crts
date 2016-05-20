function logStats = basic_stats(logName)

%% Taking the name of log file
disp(["The name of log file that you entered is <", logName, ">."]);
disp("------------------------------------------------------------------")
eval(logName);


%% Setting the names of variables using the name of log file
clear argn
clear ans
clear logName
varNames = who


%% Computation of the statistics
logStats = {};
for ii = 1 : size(varNames,1)     % Loop going through "variables"
    
    logStats{ii,1} = ["<", varNames{ii}, "> for ", num2str(size(eval(varNames{ii}),2)), " times:"];    

    for jj = 1 : size(eval(varNames{ii}),2)     % Loop going through "repetitions" within a variable
        strEval = strcat(varNames{ii}, "{", num2str(jj), "}");
        logStats{ii,jj+1} = [mean(eval(strEval)), var(eval(strEval))];
         
    end

end


%% Displaying the results
disp(["------------------------------------------------------------------\nThe number of trials is ", num2str(jj), "."])

logStats = transpose(logStats);
disp("------------------------------------------------------------------\n[ii,jj]:\nii: Repetition\njj: Variable\n")
logStats
disp("------------------------------------------------------------------")



end     % end of function
