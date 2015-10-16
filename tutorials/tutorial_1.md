#Tutorials
## Tutorial 1: Running CRTS

Begin by opening three ssh sessions on CORNET using the following command:

    $ ssh -p <node port> <CORNET username>@128.173.221.40

Choose three nodes that are adjacent to one another. Make sure that the nodes
have access to their USRPs by running:

    $ uhd_find_devices

If not, try other nodes. Navigate to the crts directory on each node. Open up 
the master\_scenario\_file.cfg file. This file tells the experiment controller 
how many scenarios to run and their names. Make the values match:

    NumberofScenarios = 1;
	scenario\_1 = "2_Node_FDD_Network.cfg";
    reps\_scenario\_1 = 1;

Now open the scenario configuration file 2\_Node\_FDD\_Network.cfg This file 
defines the most basic scenario involving two CR nodes. The default cognitive
engine is transparent (does not make any decisions). If you haven't already,
take a look at the definition of the cognitive engine being used by this
scenario just to see the general structure, more details are provided in the
Cognitive Engine section of the documentation.

Now lets actually run CRTS. First launch the controller. CRTS can be run in a 
'manual' or 'automatic' mode. The default is to run in automatic mode; manual 
mode is specified by a -m flag on the controller command. On the node you want
to act as the controller, run:

    $ ./CRTS_controller -m

Now you can run the CRTS CR process on the other two nodes.

    $ ./CRTS_CR -a <controller internal ip>

The controller IP needs to be specified so the program knows where to connect.

The internal ip will be 192.168.1.<external port number -6990>. Observe that
the two nodes have received their operating parameters and will begin to
exchange frames over the air metrics for the received frames should be printed
out to both terminals. 

Go to /logs/octave and you should see several auto-generated .m files. To view
a plot of the network throughput vs. time for each node run:

    $ octave
    > 2_Node_FDD_Network_N1_NET_RX
	> Plot_CR_NET_RX
