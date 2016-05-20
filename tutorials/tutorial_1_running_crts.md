# Tutorial 1: Running CRTS

In this tutorial we go over the basic mechanics of how to configure CRTS 
to run a test scenario or a batch of test scenarios and view the results.

Begin by selecting a set of three nodes that you will use to run a basic
scenario using CRTS. Be sure to choose nodes that are close enough for
reliable communication. If you are using CORNET, choose three adjacent
nodes. You can view the floorplan at . 
This floorplan also shows the status for each node. Be sure to choose 
nodes that are bright green, indicating that they have working USRP's.

Once you've selected three nodes, open ssh terminals to them by running
the command below. If you are using CORNET, the username will be your 
CORNET username. Note that the node ports are also displayed on the
floorplan.

\code{.sh}
  $ ssh -XC -p <node port> <username>@128.173.221.40
\endcode

If you didn't check the CORNET floorplan to see that your nodes had 
working USRP's or if you're using a testbed other than CORNET, run the 
following command on each node to double check.

\code{.sh}
  $ uhd_find_devices
\endcode

If a node does not have access to it's USRP either power cycle the USRP if
you have access to it or just try another node. 

Navigate to the crts directory on each node and open up the 
master\_scenario\_file.cfg file. This file defines the number of scenarios
that will be run when CRTS is executed along with their names and optionally
how many times these scenarios should be repeated. In this tutorial we're 
going to run the Two_Node_FDD_Network scenario, which consists of two 
cognitive radio nodes that will communicate with one another. The contents
of the master\scenario\file.cfg should look as shown below.

\code{.cpp}
  NumberofScenarios = 1;
  reps_all_scenarios = 1;
  scenario_1 = "Two_Node_FDD_Network";
\endcode

Now open the scenario configuration file Two\_Node\_FDD\_Network.cfg. As
mentioned earlier, this file defines a basic scenario involving two CR nodes. 
Familiarize yourself with the overall structure, you may also look at the
Scenario_Template.cfg file for a detailed description of all the parameters. 
In this scenario we use the CE_Template cognitive engine which does not make 
any decisions. Check to make sure that all of the print and log flags are
set to 1 so that we can view results during and after the scenario runs.

Now that we've looked at how scenarios are configured in CRTS, lets actually 
run one. First launch the controller. CRTS can be run in a 'manual' or 
'automatic' mode. The default behavior is to run in automatic mode; manual 
mode is specified by a -m flag after the controller command. Manual mode can
be very useful for debug purposes when you develop complex cognitive engines
later on. On the node you want to act as the controller, run:

\code{.sh}
  $ ./CRTS_controller -m
\endcode

Now you can run the CRTS cognitive radio process on the other two nodes.

\code{.sh}
  $ ./CRTS_CR -a <controller ip>
\endcode

The controller IP needs to be specified so the program knows where to connect. 
On CORNET the internal ip will be 192.168.1.<external port number -6990>. 

Observe that  the two nodes have received their operating parameters and will 
begin to exchange frames. Over the air metrics for the received frames should 
be printed out to both terminals. 

Once the scenario has finished running go to the /logs/octave directory. You 
should see several auto-generated .m files starting with Two_Node_FFD_Network_*. 
To view a plot of the network throughput vs. time for each node run:

\code{.sh}
  $ octave
  > Two_Node_FDD_Network_Node<node number>_NET_RX
  > Plot_CR_NET_RX
\endcode

You can also view plots of the physical layer transmitted and received frames.

\code{.sh}
  > Two_Node_FDD_Network_Node<node number>_PHY_TX
  > Plot_CR_PHY_TX

  > Two_Node_FDD_Network_Node<node number>_PHY_RX
  > Plot_CR_PHY_RX
\endcode

Troubleshooting:
- If you are seeing issues with your radio links e.g. no frames are being 
  received or there is a significant number of frames being received in error, 
  a first measure check would be to look at the transmit and receive gains for 
  each node. Depending on the physical placement of the nodes and the environment
  you may need to use higher gains to overcome path loss or in some cases you
  may need to reduce your gain to avoid clipping the ADC of the USRP.
- If you don't see the generated octave log files, return to the scenario file
  and make sure all of the options including the word log are set equal to 1.




