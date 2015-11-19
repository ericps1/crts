# CRTS
##About:

The Cognitive Radio Test System (CRTS) provides a flexible framework for over the 
air test and evaluation of cognitive radio (CR) networks. Users can rapidly define
new testing scenarios involving a large number of CR's and interferers while customizing
the behavior of each node individually. Execution of these scenarios is simple
and the results can be quickly visualized using octave/matlab logs that are kept
throughout the experiment.

CRTS evaluates the performance of CR networks by generating network layer traffic at
each CR node and logging metrics based on the received packets. Each CR node will
create a virtual network interface so that CRTS can treat it as a standard network
device. Part of the motivation for this is to enable evaluation of UDP and TCP network
connections. The CR object/process can be anything with such an interface. We are
currently working on examples of this in standard SDR frameworks e.g. GNU Radio. A block
diagram depicting the test process run on a CR node by CRTS is depicted below.
\image latex Cognitive_Radio_Test_Process_Block_Diagram.eps "Cogntive Radio Test Process" width=12cm

A particular CR has been developed with the goal of providing a flexible generic structure
to enable rapid development and evaluation of cognitive engine (CE) algorithms. This
CR is being called the Extensible Cognitive Radio (ECR). In this structure, a CE is
fed data and metrics relating to the current operating point of the radio. It can then
make decisions and exert control over the radio to improve its performance. A block diagram
of the ECR is shown below.
\image latex Extensible_Cognitive_Radio_Block_Diagram.eps "The Extensible Cognitive Radio" width=12cm

The ECR uses the  
[OFDM Frame Generator](http://liquidsdr.org/doc/tutorial_ofdmflexframe.html)
of
[liquid-dsp](http://liquidsdr.org/)
and uses an 
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
with Network Time Protocol (NTP). Precision Time Protocol (PTP) would work as well.

Note to CORNET users: These dependencies are already installed for you on all CORNET nodes.

###Downloading and Configuring CRTS 
Official releases of CRTS can be downloaded from the
[Releases Page](https://github.com/ericps1/crts/releases)
while the latest development version is available on the main 
[Git Page](https://github.com/ericps1/crts/).

Note that because using CRTS involves actively writing and compiling 
cognitive engine code, it is not installed like traditional software.

#### Official Releases
1. Download the Version 1.0 tar.gz from the [Official Releases Page](https://github.com/ericps1/crts/releases):

        $ wget -O crts-v1.0.tar.gz https://github.com/ericps1/crts/archive/v1.0.tar.gz

2. Unzip the archive and move into the main source tree:

        $ tar xzf crts-v1.0.tar.gz
        $ cd crts-v1.0/

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
with their own dedicated USRP. A single node, the `CRTS_controller`, 
will automatically launch each radio node for a given scenario and
communicate with it as the scenario progresses.

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
    + The type of interference (e.g. OFDM, GMSK, RRC, etc.).
    + The paremeters of the interferer's operation.
    + What type of data should be logged.

In some cases a user may not care about a particular setting e.g. the forward
error correcting scheme. In this case, the setting may be neglected in the
configuration file and the default setting will be used.

Examples of scenario files are provided in the `scenarios/` directory of the
source tree.

### The Extensible Cognitive Radio

As mentioned above the ECR uses an OFDM based waveform defined by liquid-dsp. The
cognitive engine will be able to control the parameters of this waveform such as
number of subcarriers, subcarrier allocation, cyclic prefix length, modulation
scheme, and more. The cognitive engine will also be able to control the settings 
of the RF front-end USRP including its gains, sampling rate, center frequency,
and digital mixing frequency. See the code documentation for more details.

Currently the ECR does not support much in the way of MAC layer functionality,
e.g. there is no ARQ or packet segmentation/concatenation. This is planned for
future development.

### Cognitive Engines in the ECR

The Extensible Cognitive Radio provides an easy way to implement generic
cognitive engines. This is accomplished through inheritance i.e. a particular
cognitive engine can be implemented as a subclass of the cognitive engine 
base class and seamlessly integrated with the ECR. The general structure is
such that the cognitive engine has access to any information related to the
operation of the ECR via get() function calls as well as metrics passed from
the receiver DSP. It can then control any of the operating parameters of the
radio using set() function calls defined for the ECR.

The cognitive engine is defined by an execute function which can be triggered by
several events. The engine will need to respond accordingly depending on the type
of event that occurred. The event types include the reception of a physical layer
frame, a timeout, or USRP overflows and underruns.

To make a new cognitive engine a user needs to define a new cognitive engine
subclass. The CE\_Template.cpp and CE\_Template.hpp can be used as a guide in terms
of the structure, and some of the other examples show how the CE can interact
with the ECR. Once the CE has been defined it can be integrated into CRTS
by running $ ./config\_CEs in the top directory.

Other source files in the cognitive\_engine directory will be automatically
linked into the build process. This way you can define other classes that your
CE could instantiate. To make this work, a cpp file that defines a CE must be named
beginning with "CE_" as in the examples.

* Any cpp files defining a cognitive engine must begin with "CE_" as in the examples! *

Installed libraries can also be used by a CE. For this to work you'll need to manually
edit the makefile by adding the library to the variable LIBS which is located at the
top of the makefile and defines a list of all libraries being linked in the final
compilation.

One particular function that users should be aware of is ECR.set\_control\_information().
This provides a generic way for cognitive radios to exchange control information
without impacting the flow of data. The control information is 6 bytes which are
placed in the header of the transmitted frame. It can then be extracted in the
cognitive engine at the receiving radio. A similar function can be performed by 
transmitting a dedicated control packet from the CE.

Examples of cognitive engines are provided in the `cognitive_engines/` directory.

### Cognitive Radios in Python

Along with cognitive engines defined by the ECR, CRTS also supports cognitive radios written in python. An
example of a very simple python cognitive radio can be found in cognitive_radios/python_txrx.py.  When using a python radio, the scenario file
must specify the cr\_type ("python") and a python\_file (python_txrx.py in the example scenario). The python file must be
in the top-level cognitive_radios folder. In addition, you can supply arguments to pass to your radio by using the
arguments field. An example scenario file for a python radio can be found in scenarios/Python\_Flowgraph_Example.cfg.

### Interferers

The testing scenarios for CRTS may involve generic interferers. There are a number
of parameters that can be set to define the behavior of these interferers. They
may generate CW, GMSK, RRC, OFDM, or Noise waveforms. Their behavior can be defined
in terms of when they turn off and on by the period and duty cycle settings, and 
there frequency behavior can be defined based on its type, range, dwell time, and
increment.

### Logs

CRTS will automatically log data from several points in the system (provided that
the log file options are appropriately specified in the scenario configuration
file). These logs include the parameters used by the ECR to create each phyiscal
layer frame that is transmitted, the metrics and parameters of each of the physical
layer frames received by the ECR, as well as the data received at the network
layer (the data read from the virtual network interface by CRTS\_CR). Each log
entry will include a timestamp to keep events referenced to a common timeline
The logs are written as raw binary files in the /logs/bin directory, but will 
automatically be converted to either Octave/Matlab or Python scripts after the 
scenario has finished and placed in the logs/Octave or logs/Python directories 
respectively. This again assumes that the appropriate options were set in the 
scenario configuration file. These scripts provide the user with an easy way to 
analyze the results of the experiment. There are some basic Octave/Matlab scripts 
provided to plot the contents of the logs as a function of time.

