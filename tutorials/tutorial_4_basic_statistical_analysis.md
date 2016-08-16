# Tutorial 4: Basic Statistical Analysis

In this tutorial we demonstrate how trivial it is to perform some basic
statistical analysis on a set of test scenarios.

To do this, we will write a new scenario master file to define our set
of test scenarios and use the calculate_summary_statistics octave script
to do our basic statistical analysis of the results. 

Let's look at how our FEC adaptation engine works using different windows
to average EVM measurements in the presence of an interferer with different 
temporal behavior. Specifically we'll look at cognitive engines that use 
measurement windows of length 10 s, 500 ms, and 10 ms and interferers 
operating with a 0.5 duty cycle over a period of 10 s, 1 s, and 100 ms.

Before we setup these scenarios, let's double check that the provided
example scenario will do what we want. If you haven't already, choose
3 nodes to run the tests on. Ideally they will be close enough to one
another that each node will hear the others. Now open example_scenarios/fec_adaptation.cfg
and verify that the server_ip parameters match the nodes that you've
selected. Let's also set the run_time to 60. Note that to get statistically
sound results we would probably want to run each scenario for longer than
this, but for the purposes of this tutorial this is fine.

Now let's setup the scenarios for this tutorial. From the base crts directory:

\code{.sh}
  $ cd scenarios
  $ mkdir tutorial_4_scenarios
  $ cp example_scenarios/fec_adaptation.cfg tutorial_4_scenarios/window_10s_interference_period_10s.cfg
  $ cp example_scenarios/fec_adaptation.cfg tutorial_4_scenarios/window_10s_interference_period_1s.cfg
  $ cp example_scenarios/fec_adaptation.cfg tutorial_4_scenarios/window_10s_interference_period_100ms.cfg
  $ cp example_scenarios/fec_adaptation.cfg tutorial_4_scenarios/window_500ms_interference_period_10s.cfg
  $ cp example_scenarios/fec_adaptation.cfg tutorial_4_scenarios/window_500ms_interference_period_1s.cfg
  $ cp example_scenarios/fec_adaptation.cfg tutorial_4_scenarios/window_500ms_interference_period_100ms.cfg
  $ cp example_scenarios/fec_adaptation.cfg tutorial_4_scenarios/window_10ms_interference_period_10s.cfg
  $ cp example_scenarios/fec_adaptation.cfg tutorial_4_scenarios/window_10ms_interference_period_1s.cfg
  $ cp example_scenarios/fec_adaptation.cfg tutorial_4_scenarios/window_10ms_interference_period_100ms.cfg 
\endcode

Now edit each of these scenarios to contain the relevant parameters

\code{.cpp}
  // under nodes 1 & 2
  ce_args = "-w <window in seconds>"

  // under the interferer
  period = <period in seconds>
\endcode

Now let's define a new scenario master configuration file called scenario_master_tutorial_4.cfg
to run these scenarios. Let's run each scenario 3 times. Again note that to get 
sound statistical results we would probably want to run each test more than just 
3 times. For the sake of keeping the tutorial shorter we won't concern ourselves 
with this detail. Also make sure you've set the option to generate a summary log 
in octave.Your new scenario master configuration file should look something like 
below.

/code{.cpp}
num_scenarios = 9;
reps_all_scenarios = 3;
octave_log_summary = 1;

scenario_1 = "tutorial_4_scenarios/window_10s_interference_period_10s";
...
scenario_9 = "tutorial_4_scenarios/window_10ms_interference_period_100ms";
/endcode

Now we should be able to just launch the controller and sit back while it
runs. Note that based on how we've set things up, this will take about
an hour to run. You can of course reduce the number of scenarios or run time
if you are in a rush.

/code{.sh}
  $ ./crts_controller -f scenario_master_tutorial_4 
/endcode

Once it has finished running let's take a look at the statistical results.
We first load the summary data into octave, then run our statistical
analysis script and provide the arguments necessary as prompted. First,
let's compare the performance of the cognitive engines. To do this, we
provide an argument which will group the scenarios with common cognitive
engines. Let's also look at nodes 1 and 2 individually as well as part of
a network. The commands below will accomplish this.

/code{.sh}
  $ cd logs/octave
  $ octave
  >> scenario_master_tutorial_4_summary
  >> calculate_summary_statistics
  >> {{'*10s_*'},{'*500ms_*'},{'*10ms_*'}}
  >> {1,2,[1,2]}
/endcode

At this point you should be able to see some basic statistics for how the
different cognitive engines performed.

Let's run the analysis again looking at how the interference period affected
the performance of all of the cognitive engines overall.

/code{.sh}
  >> calculate_summary_statistics
  >> {{'*10s'},{'*1s'},{'*100ms'}}
  >> {1,2,[1,2]}
/endcode

