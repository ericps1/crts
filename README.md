# CRTS
##About:

The Cognitive Radio Test System (CRTS) is intended to provide a flexible framework for 
over the air test and evaluation of cognitive radio networks. 
Users can configure networks of cognitive radios that use intelligent
algorithms defined in a cognitive engine to optimize their performance and that
of the network. 

In time, CRTS will be able to connect with any cognitive radio with only 
few modifications. As of now, CRTS can run with any custom cognitive engine developed 
through the provided Extensible Cognitive Radio (ECR) API.
Through the ECR, developers can deploy real cognitive radios built from their custom cognitive engines 
and then evaluate their performance with CRTS. 
By providing accessible and customizable waveforms, the ECR enables developers to focus on their 
true interests, cognitive engine algorithms, without being bogged in implementation of 
every option they wish to be made available to their cognitive engines.
The waveforms of the ECR are based on the
[OFDM Frame Generator](http://liquidsdr.org/doc/tutorial_ofdmflexframe.html)
of
[liquid-dsp](http://liquidsdr.org/)
and are designed for use with an 
[Ettus](http://www.ettus.com/)
Univeral Software Radio Peripheral (USRP).

CRTS is being developed using the CORNET testbed under 
Virginia Tech's
[Wireless@VT](https://wireless.vt.edu/)
Research Group.

##Installation:
###Dependencies
	These should already be installed on most CORNET nodes.
	-uhd
	-liquid-dsp
	-libconfig

###CRTS 
	$ git clone https://github.com/ericps1/crts.git
	$ cd crts
	$ make
	$ make setup_env

	NOTE: do no run the last command as sudo
	
	The last command sets up an environment variable for the CRTS path and allows
	the user to launch CRTS_CR as sudo without a password. This is important for
	the automatic operation so that the user doesn't need to enter his/her 
	password for every node. Sudo is required by CRTS_CR because it creates and 
	tears down a virtual network interface upon each run. To undo these changes
	run:

	$ make teardown_env

##Tutorial:

Begin by opening four ssh sessions on CORNET using the following command:

	$ ssh -p <node port> <username>@128.173.221.40

Navigate to the crts directory. First open up the master\_scenario\_file.cfg file.
This file simply tells the experiment controller how many tests will be performed
and their names. Now open the default scenario configuration file,
./scenarios/interferer.cfg. This file defines all of the nodes that will be
involved in the scenario along with some parameters that define their behavior.

One of the more important features of CRTS is that it allows users to write their
own cognitive engines in C++. Take a look at ./cognitive\_engines/CE\_Example.cpp.
The execute function is what defines the operation of the cognitive engine. Here,
the cognitive engine will be continually updated with information about what the
radio is doing which it can use to adjust the radios behavior.

A user can create as many custom cognitive engines he wants by adding files that
follow the structure of the examples provided. The name of the class used in the
file must match the file's name. Once the cognitive engine is defined, run:
./config\_CEs in the crts directory. This will actually modify some of the code
in CRTS to allow the cognitive engine to be used. Now you can modify or create a
scenario configuration file to have a node that uses the new cognitive engine.
Variables can be added to the custom member struct which can be accessed in the
execute function. Users can define other functions that the CE needs as well.

Now we'll actually run CRTS. On the node you want to use as the controller execute:

	$ ./CRTS_controller -m

The -m option tells the controller that you want to run the experiment manually
by launching the processes on the other nodes yourself. The controller can do this
for you by using the following command.

	$ ./CRTS_controller -a <controller internal ip>

In this case you need to make sure that the ips are set up correctly in the scenario
config file being used. Assuming you've launched CRTS manually, on two of the other 
nodes run:

	$ sudo ./CRTS_CR -a <controller internal ip>

The internal ip will be 192.168.1.<external port number -6990>. Observe that 
the two nodes have received their operating parameters and will begin to 
exchange frames over the air. On the last node:

	$ ./CRTS_interferer -a <controller internal ip>

Observe that the interferer will turn off and on according to the duty cycle that
was specified in the scenario configuration file. If you look at the EVM
statistic being printed to the screen by the two cognitive radio nodes you can
see that it degrades (becomes less negative) when the interferer is on.

While the experiment was running, each cognitive radio kept a log of its 
performance metrics stored as a binary file in the ./logs directory. These logs
can be post processed into octave scripts which will plot the behavior and
performance of the cognitive radio throughout the experiment. On one of the
nodes:

	$ cd logs
	$ ./post_process_logs -l <binary log file name> -o <octave script name>.m
	$ octave
	>> <octave script name>

This will generate a number of plots showing what the cognitive radio was doing
for the duration of the experiment. \<binary log file name\> should be specified in the 
scenario configuration file (scenarios/interferer.cfg in our example) with the log_file 
parameter. \<octave script name\> can be any convenient file name; 
it should be appended with a .m extension for post_process_logs, but omit the extension 
when being called from octave. Some of these statistics may be more meaningful 
than others. On one of the nodes (whichever used the same frequency
as the interferer) the EVM should follow the same duty cycle as the interferer.
There may be packet errors that follow this pattern as well.
