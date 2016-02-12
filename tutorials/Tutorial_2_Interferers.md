# Tutorial 2: Interferers

In this tutorial we go over how to use an interferer in a CRTS test 
scenario and the options available in terms of defining the interferers
behavior.

As in the previous tutorial, select a set of three nearby nodes in your
testbed and open ssh terminals to each.

Modify master/_scenario/_file.cfg to run the Interferer_Test scenario.
The contents of the file should then include the following definitions.

  NumberofScenarios = 1;
  reps_all_scenarios = 1;
  scenario_1 = "Interferer_Test";

Now open the Interferer_Test.cfg file to see the scenario definition.
You may also refer to the Scenario_Template.cfg file for a detailed
description of each parameter for an interferer node. For the first
execution let's set the following parameters.

  tx_rate = 1e6;
  interference_type = "RRC";
  period = 4.0;
  duty_cycle = 1.0;
  tx_freq_behavior = "FIXED";

On another node, open uhd_fft so that we can see the interferer's
transmissions; run the following command

  $ uhd_fft -f <tx_freq> -s <tx_rate> -g <gain>

where tx_freq and tx_rate are the parameters defined in the
Interferer_Test.cfg file, and gain should be set based on the physical
separation of the nodes. On CORNET, a gain of 10-20 dB is usually good.

You should now see a plot of the spectrum where the interferer will
transmit. If there is already a signal present you may want to change
to a different band (one which you have a license for of course). To 
do so you'll need to change tx_freq in the scenario file.

Return to the first node and run the CRTS controller

  $ ./CRTS_controller -m

Finally on a third node run

  $ ./CRTS_interferer -a <controller ip>

You should now see a constant signal in the middle of the spectrum.
It should have the root-raised-cosine shape.

Now go back to the scenario file and edit the duty cycle to be 0.5.
Rerun CRTS and you should see the same signal which will alternate 
between being on for duty_cycle*period seconds and then off for
(1-duty_cycle)*period.

Now let's look at dynamic frequency behavior. Go back to the scenario
file and set the following properties

  duty_cycle = 1.0;
  tx_freq_behavior = "SWEEP";
  tx_freq_min = <tx_freq-5e6>;
  tx_freq_max = <tx_freq+5e6>;
  tx_freq_dwell_time = 1.0;
  tx_freq_resolution = 1.0e6;

Also make sure the log flags are set as shown below.

  log_phy_tx = 1;
  generate_octave_logs = 1;

Close uhd_fft and rerun it so we can see the full band the interferer 
will be transmitting in:

  $ uhd_fft -f <tx_freq> -s 10e6 -g <gain>

Rerun CRTS and you should now see a signal which will sweep back and
forth across the viewable spectrum, changing frequencies once every
second.

Now move into the /logs/octave directory. You should see a file called
Interferer_Test_Int_PHY_TX.m. If you do, run the following to see
a plot of the interferer's transmission parameters as a function of
time throughout the scenario's execution.

  $ octave
  >> Interferer_Test_Node1_Int_PHY_TX
  >> Plot_Interferer_PHY_TX

If you'd like, play around with some of the settings. You might try 
changing tx_freq_behavior to "RANDOM", changing the interference_type,
or trying some combination of dynamic frequency behavior and a duty 
cycle.








