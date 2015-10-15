# CRTS
##About:

The Cognitive Radio Test System (CRTS) is intended to provide a flexible framework for 
over the air test and evaluation of cognitive radio (CR) networks. 
Users can configure networks of CRs that use intelligent
algorithms defined in a cognitive engine to optimize their performance and that
of the network. 

In time, CRTS will be able to connect with any CR with only 
few modifications. As of now, CRTS can run with any custom cognitive engine developed 
through the provided Extensible Cognitive Radio (ECR) API.

Through the ECR, developers can deploy real cognitive radios built from their custom 
cognitive engines and then evaluate their performance with CRTS. 
By providing accessible and customizable waveforms, the ECR enables developers to focus 
on their cognitive engine algorithms, without being bogged in implementation of 
every aspect of the signal processing.

The waveforms of the ECR are based on the
[OFDM Frame Generator](http://liquidsdr.org/doc/tutorial_ofdmflexframe.html)
of
[liquid-dsp](http://liquidsdr.org/)
and are designed for use with an 
[Ettus](http://www.ettus.com/)
Univeral Software Radio Peripheral (USRP).

CRTS is being developed using the 
[CORNET](http://cornet.wireless.vt.edu/)
testbed under 
Virginia Tech's
[Wireless@VT](https://wireless.vt.edu/)
Research Group.

##Installation:
###Dependencies

CRTS is being developed on 
[Ubuntu 14.04](http://releases.ubuntu.com/14.04/)
but should be compatible with most 
Linux distributions.
To compile and run CRTS and the ECR, your system will need
the following packages. 
If a version is indicated, then it is recommended because it 
is being used in CRTS development.
- [UHD Version 3.8.4](https://github.com/EttusResearch/uhd/releases/tag/release_003_008_004)
- [liquid-dsp commit a4d7c80d3](https://github.com/jgaeddert/liquid-dsp/commit/a4d7c80d3a3510a453c30e02e58b505d07afb920)
- libconfig-dev

CRTS also relies on each node having network synchronized clocks. On CORNET this is accomplished
with NTP. PTP would work as well.

Note to CORNET users: These dependencies are already installed for you on all CORNET nodes.

###Downloading and Configuring CRTS 
Official releases of CRTS can be downloaded from the
[Releases Page](https://github.com/ericps1/crts/releases)
while the latest development version is available on the main 
[Git Page](https://github.com/ericps1/crts/).

Note that because using CRTS involves actively writing and compiling 
cognitive engine code, it is not installed like traditional software.

#### Official Releases
1. Download the Version 2.0 tar.gz from the [Official Releases Page](https://github.com/ericps1/crts/releases):

        $ wget -O crts-v2.0.tar.gz https://github.com/ericps1/crts/archive/v2.0.tar.gz

2. Unzip the archive and move into the main source tree:

        $ tar xzf crts-v2.0.tar.gz
        $ cd crts-v2.0/

3. Compile the code with:

        $ make

4. Then configure the system to allow certain networking commands without a password 
    (CORNET users should skip this step):

        $ sudo make install

The last step should only ever need to be run once. 
It configures the system to allow all users to run
certain very specific networking commands which are necessary for CRTS.
They are required because CRTS creates and 
tears down a virtual network interface upon each run. 
The commands may be found in the .crts\_sudoers file.

To undo these changes, simply run:

	$ sudo make uninstall

#### Latest Development Version
1. Download the git repository:

        $ git clone https://github.com/ericps1/crts.git

2. Move into the main source tree:

        $ cd crts/

3. Compile the code with:

        $ make

4. Then configure the system to allow certain networking commands without a password 
    (CORNET users should skip this step):

        $ sudo make install

The last step should only ever need to be run once. 
It configures the system to allow all users to run
certain very specific networking commands which are necessary for CRTS.
They are required because CRTS creates and 
tears down a virtual network interface upon each run. 
The commands may be found in the .crts\_sudoers file.

To undo these changes, simply run:

	$ sudo make uninstall

## An Overview

CRTS is designed to run on a local network of machines, each 
with their own dedicated USRP 
(though CRTS could also be run on a single machine with multiple USRPs).
Through the main program, `CRTS_controller`,
CRTS facilitates fast and effiecient CR experimentation
by automatically launching each radio node in the emulated environment or scenario.

Each radio node could be 
1. A member of a CR network (controlled by `CRTS_CR`) 
or 
2. An interfering node (controlled by `CRTS_interferer`),
    generating particular noise or interference patterns against which the 
    CR nodes must operate.

### Scenarios

The `master_scenario_file.cfg` specifies which scenario(s) should be run for a
single execution of the `CRTS_controller`. A single scenario can be run multiple
times if desired. The syntax scenario\_<#> and reps\_scenario\_<#> must be used.

Scenarios are defined by configuration files in the scenarios/ directory. Each of 
these files will specify the number of nodes in the experiment and the duration
of the experiment. Each node will have additional parameters that must be specified. 
These parameters include but are not limited to:
- The node's type: CR or interferer.
- The node's local IP address.
- If it is a CR node, it further defines:
    + The type of the CR (e.g. if it uses the ECR or some external CR).
    + The node's virtual IP address in the CR network.
    + The virtual IP address of the node it initially communicates with.
    + If the CR node uses the ECR, it will also specify:
        * Which cognitive engine to use.
        * The initial configuration of CR. 
        * What type of data should be logged.
- If it is an interferer node, it further defines:
    + The type of interferer (e.g. AWGN, OFDM, etc.).
    + The paremeters of the interferer's operation.
    + What type of data should be logged.

In some cases a user may not care about a particular setting e.g. the forward
error correcting scheme. In this case, the setting may be neglected in the
configuration file and the default setting will be used.

Examples of scenario files are provided in the `scenarios/` directory of the
source tree.

### Cognitive Engines

The Extensible Cognitive Radio provides an easy way to implement generic
cognitive engines. This is accomplished through inheritance i.e. a particular
cognitive engine can be implemented as a subclass of the cognitive engine 
base class and seamlessly integrated with the ECR. The general structure is
such that the cognitive engine has access to any information related to the
operation of the ECR via get() function calls as well as metrics passed from
the receiver DSP. It can then control any of the operating parameters of the
radio using set() function calls defined for the ECR.

Variables used to keep track of information from one execution of the cognitive
engine to the next must be declared as static (otherwise they will fall out of
scope after the end of the execute function).

One particular function that users should be aware of is ECR.set\_control\_information().
This provided a generic way for cognitive radios to exchange control information
without impacting the flow of data. The control information is 6 bytes which are
placed in the header of the transmitted frame. It can then be extracted in the
cognitive engine of at the receiving radio.


Examples of cognitive engines are provided in the `cognitive_engines/` directory.

##Tutorial:

Begin by opening four ssh sessions on CORNET using the following command:

	$ ssh -p <node port> <username>@128.173.221.40

Navigate to the crts directory. First open up the master\_scenario\_file.cfg file.
This file simply tells the experiment controller how many tests will be performed
and their names. Now open the default scenario configuration file,
./scenarios/interferer.cfg. This file defines all of the nodes that will be
involved in the scenario along with some parameters that define their behavior and 
initial conditions.

One of the more important features of CRTS is that it allows users to write their
own cognitive engines in C++. Take a look at ./cognitive\_engines/CE\_Example.cpp.
The execute function is what defines the operation of the cognitive engine. Here,
the cognitive engine will be continually updated with information about what the
radio is doing which it can use to adjust the radios behavior.

A user can create as many custom cognitive engines he wants by adding files that
follow the structure of the examples provided. The name of the class used in the
file must match the file's name. Once the cognitive engine is defined, run:
./config\_CEs from the crts directory. This will actually modify some of the code
in CRTS to allow the cognitive engine to be used. Now you can modify or create a
scenario configuration file to have a node that uses the new cognitive engine.
You will want to create static variables within the cognitive engine execute
function so that they will persist from one execution to the next. You can look
at the examples to see how this is done. You can also define other functions that
can be called from within the cognitive engine execute function.

Now we'll actually run CRTS. On the node you want to use as the controller execute:

	$ ./CRTS_controller -m

The -m option tells the controller that you want to run the experiment manually
by launching the processes on the other nodes yourself. The controller can do this
for you by using the following command.

	$ ./CRTS_controller -a <controller internal ip>

In this case you need to make sure that the ips are set up correctly in the scenario
config file being used. Assuming you've launched CRTS manually, on two of the other 
nodes run:

	$ ./CRTS_CR -a <controller internal ip>

The internal ip will be 192.168.1.<external port number -6990>. Observe that 
the two nodes have received their operating parameters and will begin to 
exchange frames over the air. On the last node:

	$ ./CRTS_interferer -a <controller internal ip>

Observe that the interferer will turn off and on according to the duty cycle that
was specified in the scenario configuration file. If you look at the EVM
statistic being printed to the screen by the two CR nodes you can
see that it degrades (becomes less negative) when the interferer is on.

CRTS and the ECR will log all events to a binary file as the scenario runs.
These logs will be converted to octave and/or python files which can be used
to visualize or calculate various performance metrics for the radios. This behavior
can be controlled through various flags found in the scenario files. We've provided
some standard octave scripts to plot the logs as a function of time.

