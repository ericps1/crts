# Tutorial 5: Physical Layer Testing

In this tutorial we demonstrate how you can execute some basic physical layers tests
to gain insight into how this might affect the design of your cognitive engines. To
do this we'll be using the SC_Performance_Sweep_Utility scenario controller.

Let's begin by taking a brief look at how the performance sweep scenario controller
works. Open scenario_controllers/SC_Performance_Sweep_Utility/SC_Performance_Sweep_Utility.cpp 
and take a look at the constructor. You can see that it takes arguments for a sweep
configuration file, a dwell time, settle time, and log file name. The dwell time
specifies the period of time over which each measurement point is taken. The settle
time specifies the period of time for which measurement is suspended so that the
radio operation will be in steady state for the next measurement period.

Open some of the provided sweep configurations in 
scenario_controllers/SC_Performance_Sweep_Utility/ and take a look at the general structure.
You can define multiple sweep parameters with either specified values, or a range
of values specified by a starting value, a step value, and a final value. The 
sweep structure can be either nested, which will iterate through every combination of
the provided sweeping parameters, or it can be linear, which will step through each
parameter simultaneously and only once. A linear sweep should have the same number of
values specified for each parameter.

Let's make a new sweep configuration that will perform a nested sweep of both
transmit and receive gain.

\code{.sh}
  $ cd scenario_controllers/SC_Performance_Sweep_Utility
  $ cp tx_gain_sweep.cfg tx_and_rx_gain_sweep.cfg
\endcode

Open up the new config and edit it to also sweep the receive gain.
Let's also reduce the range and increase the step size to keep the
required runtime reasonable.

\code{.cpp}
  num_sweep_params = 2;
  sweep_mode = "nested";

  param_1 : {
    param_type = "tx gain";
    
    initial_val = 5.0;
    final_val = 20.0;
    step_val = 3.0;
  }; 
  param_2 : {
    param_type = "rx gain";
    
    initial_val = 5.0;
    final_val = 20.0;
    step_val = 3.0;
  }; 
\endcode

Now let's define a new test scenario which will run the sweep:

\code{.sh}
  $ cd scenarios
  $ cp example_scenarios/basic_two_node_network.cfg tutorial_5.cfg
\endcode

Let's edit the new scenario file to use the performance sweep scenario controller.
We also need to give it the proper arguments so that it will use our new sweep
congiguration, use the correct dwell and settle times, and write the results to
the log file that we want.

We need to set the runtime to ensure that the sweep will finish. The sweep
will go through a total of 36 combinations of tx/rx gains. Each sweep point
will take 31 seconds between the dwell and settle times. So the runtime should
be atleast 36x31=1116 seconds, let's make it a round 1120 just to be sure we
get the last data point. By the way, the sweep will loop back to the beginning
if the runtime is long enough, but any measurements taken after the initial
pass will be discarded. The following changes should be made to the new
scenario file.

\code{.cpp}
  scenario_controller = "SC_Performance_Sweep_Utility";
  sc_args = "-d 30 -s 1 -c tx_and_rx_gain_sweep -l tx_and_rx_gain_sweep_log";
\endcode

You'll also want to make sure you've set the server_ip's to use the nodes you
want, but hopefully you're pros by now and have these details on lock. Now we 
should be ready to run the test.

\code{.sh}
  $ ./crts_controller -s tutorial_5
\endcode

As the test runs you should see printouts showing the current sweep values and 
the metrics reported from each cognitive radio. Once the test finishes the results
will be written to logs/csv/tx_and_rx_gain_sweep_log.csv. You can open it in
libreoffice --calc, though I prefer to actually transfer the file to a windows
machine and view it in excel. In this case you could create a pivot chart and
show a family of curves for things like Estimated EVM vs. Transmit Gain with
a unique curve for each receive gain value.




