# Tutorial 2: Interferers

In this tutorial we go over how to use an interferer in a CRTS test 
scenario and the options available in terms of defining the interferers
behavior.

As in the previous tutorial, select a set of three nearby nodes in your
testbed and open ssh terminals to each.

Modify master/_scenario/_file.cfg to run the Interferer_Test scenario.
The contents of the file should then include the following definitions.

\code{.cpp}
  NumberofScenarios = 1;
  reps_all_scenarios = 1;
  scenario_1 = "Interferer_Test";
\endcode

Now open the Interferer_Test.cfg file to see the scenario definition.
You may also refer to the Scenario_Template.cfg file for a detailed
description of each parameter for an interferer node. For the first
execution let's set the following parameters.

\code{.cpp}
  tx_rate = 1e6;
  interference_type = "RRC";
  period = 4.0;
  duty_cycle = 1.0;
  tx_freq_behavior = "FIXED";
\endcode

On another node, open uhd_fft so that we can see the interferer's
transmissions; run the following command

\code{.sh}
  $ uhd_fft -f <freq> -s <rate> -g <gain>
\endcode

where freq should match the tx_freq defined in the Interferer_Test.cfg 
file, rate should be greater than or equal to the tx_rate defined in
the Interferer_Test.cfg file, and gain should be set based on the physical
separation of the nodes. On CORNET, a gain of 10-20 dB is usually good.

You should now see a plot of the spectrum where the interferer will
transmit. If there is already a signal present you may want to change
to a different band (one which you have a license for of course). Remember
to also change the tx_freq parameter in the scenario file.

Return to the first node and run the CRTS controller

\code{.sh}
  $ ./CRTS_controller -m
\endcode

Finally on a third node run

\code{.sh}
  $ ./CRTS_interferer -a <controller ip>
\endcode

You should now see a constant signal in the middle of the spectrum.
It should have the root-raised-cosine shape.

Now go back to the scenario file and edit the duty cycle to be 0.5.
Rerun CRTS and you should see the same signal which will alternate 
between being on for duty_cycle*period seconds and then off for
(1-duty_cycle)*period.

Now let's look at dynamic frequency behavior. Go back to the scenario
file and set the following properties

\code{.cpp}
  duty_cycle = 1.0;
  tx_freq_behavior = "SWEEP";
  tx_freq_min = <tx_freq-5e6>;
  tx_freq_max = <tx_freq+5e6>;
  tx_freq_dwell_time = 1.0;
  tx_freq_resolution = 1.0e6;
\endcode

Also make sure the log flags are set as shown below.

\code{.cpp}
  log_phy_tx = 1;
  generate_octave_logs = 1;
\endcode

Close uhd_fft and rerun it so we can see the full band the interferer 
will be transmitting in:

\code{.sh}
  $ uhd_fft -f <tx_freq> -s 10e6 -g <gain>
\endcode

Rerun CRTS and you should now see a signal which will sweep back and
forth across the viewable spectrum, changing frequencies once every
second.

Now move into the /logs/octave directory. You should see a file called
Interferer_Test_Int_PHY_TX.m. If you do, run the following to see
a plot of the interferer's transmission parameters as a function of
time throughout the scenario's execution.

\code{.sh}
  $ octave
  >> Interferer_Test_Node1_Int_PHY_TX
  >> Plot_Interferer_PHY_TX
\endcode

If you'd like, play around with some of the settings. You might try 
changing tx_freq_behavior to "RANDOM", changing the interference_type,
or trying some combination of dynamic frequency behavior and a duty 
cycle.








