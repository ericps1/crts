# Tutorial 1: Running CRTS

In this tutorial we go over the basic mechanics of how to configure CRTS 
to run a test scenario or a batch of test scenarios and view the results.

Begin by selecting a set of three nodes that you will use to run a basic
scenario using CRTS. Be sure to choose nodes that are close enough for
reliable communication. If you are using CORNET, choose three adjacent
nodes. You can view the floorplan 
\href{http://www.cornet.wireless.vt.edu/CORNET3D/CORNET3D/cornet_3d_full/CORNET3D/index.html}{here}. 
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
following command on each node to double check that the USRP's are available.

\code{.sh}
  $ uhd_find_devices
\endcode

If a node does not have access to it's USRP either power cycle the USRP if
you have access to it or just try another node. 

Navigate to the crts directory on each node (this is assuming you've already
followed the installation instructions) and open up the 
scenario\_master\_template.cfg file. This file defines the number of scenarios
that will be run when CRTS is executed along with their names and optionally
how many times these scenarios should be repeated. In this tutorial we're 
going to run the basic_two_node_network scenario , which consists of two 
cognitive radio nodes that will communicate with one another. The 
scenario_master_template file should already be setup to run this scenario.
A minimalist version of this would look like the following.

\code{.cpp}
  num_scenarios = 1;
  reps_all_scenarios = 1;
  scenario_1 = "example_scenarios/basic_two_node_network";
\endcode

Now open the scenario configuration file scenarios/example\_scenarios/basic\_two\_node\_network.
As mentioned earlier, this file defines a basic scenario involving two CR nodes. 
Familiarize yourself with the overall structure; there are some general scenario
parameters at the top followed by node declarations which have their own parameters.
You may also want to look at the scenarios/scenario_template.cfg file for a more 
detailed description of all the parameters. In this scenario we use the CE_Template 
cognitive engine which is basically a placeholder i.e. it does not make any decisions. 
Check to make sure that all of the print and log flags are set to 1 so that we can 
view results during and after the scenario runs.

Now that we've looked at how scenarios are configured in CRTS, lets actually 
run one. First launch the controller. CRTS can be run in a 'manual' or 
'automatic' mode. The default behavior is to run in automatic mode; manual 
mode is specified by a -m flag after the controller command. Manual mode can
be very useful for debug purposes when you develop complex cognitive engines
later on. If you want to run in automatic mode, make sure the server_ip
parameters for both nodes point to the nodes you want to use. On the node you 
want to act as the controller, run:

\code{.sh}
  $ ./crts_controller -m
\endcode

Now you can start the CRTS cognitive radio processes on the other two nodes.

\code{.sh}
  $ ./crts_cognitive_radio -a <controller ip>
\endcode

The controller IP needs to be specified so the program knows where to connect. 
On CORNET the internal ip will be 192.168.1.<external port number -6990>. You
can double check the ip by running ifconfig on the controller node.

Once you've started the two CR nodes, observe that  they have received their 
operating parameters and will shortly begin to exchange frames over the air. 
Metrics for the received frames should be printed out to both terminals. When
you run in automatic mode this output will be stored in the logs/stdout directory.

Once the scenario has finished running go to the /logs/octave directory. You 
should see several auto-generated .m files starting with basic\_two\_node\_network\_*. 
To view a plot of the network throughput vs. time for each node run:

\code{.sh}
  $ octave
  > basic_two_node_network_node_<node number>_net_rx
  > plot_cognitive_radio_net_rx
\endcode

You can also view plots of the physical layer transmitted and received frames.

\code{.sh}
  > basic_two_node_network_node_<node number>_phy_tx
  > plot_cognitive_radio_phy_tx

  > basic_two_node_network_node_<node number>_phy_rx
  > plot_cognitive_radio_phy_rx
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




