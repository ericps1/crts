This documentation was formatted to create the crts-manual.pdf file using Doxygen which
doesn't translate super neatly to the github page.

> Please see the `crts-manual.pdf` in this repo for the complete CRTS documentation

\tableofcontents
# About {#About}

The Cognitive Radio Test System (CRTS) provides a flexible framework for over the 
air test and evaluation of cognitive radio (CR) networks. Users can rapidly define
new testing scenarios involving a large number of CR's and interferers while customizing
the behavior of each node individually. Execution of these scenarios is simple
and the results can be quickly visualized using octave/matlab logs that are kept
throughout the experiment.

CRTS evaluates the performance of CR networks by generating network layer traffic at
each CR node and logging metrics based on the received packets. Each CR node will
create a virtual network interface so that CRTS can treat it as a standard network
device. Part of the motivation for this is to enable development of upper layer
protocols. The CR object/process can be anything with such an interface. We are
currently working on examples of this in standard SDR frameworks e.g. GNU Radio. A block
diagram depicting the test process run on a CR node by CRTS is depicted below.
\image latex Cognitive_Radio_Test_Process_Block_Diagram.eps "Cogntive Radio Test Process" width=12cm

In addition to this CR test framework, a particular CR has been developed with the 
goal of providing a flexible, generic structure to enable rapid development and 
evaluation of cognitive engine (CE) algorithms. This CR has been named the Extensible 
Cognitive Radio (ECR). The basic idea of the ECR is rather simple, a CE is fed data 
and metrics relating to the current operating point of the radio. It can then make 
decisions and exert control over the radio to improve its performance. A block diagram
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

# Installation {#Installation}
## Dependencies {#Dependencies}

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
You will still need to download the CRTS source repository and compile it however.

##Downloading and Configuring CRTS {#Downloading_and_Configuring_CRTS}
Official releases of CRTS can be downloaded from the
[Releases Page](https://github.com/ericps1/crts/releases)
while the latest development version is available on the main 
[Git Page](https://github.com/ericps1/crts/).

Note that because using CRTS involves actively writing and compiling 
cognitive engine code, it is not installed like traditional software.

### Latest Development Version
1. Download the git repository:

        $ git clone git@github.com:ericps1/crts.git

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

# An Overview {#An_Overview}

CRTS is designed to run on a local network of machines, each 
with their own dedicated USRP. A single node, the `crts_controller`, 
will automatically launch each radio node for a given scenario and
communicate with it as the scenario progresses.

Each radio node in a test scenario could be 
1. A member of a CR network (controlled by `crts_cognitive_radio`) 
or 
2. An interfering node (controlled by `crts_interferer`),
    generating particular noise or interference patterns against which the 
    CR nodes must operate.

In the next sections we provide an overview of the high level components
used by CRTS to enable flexible, scalable testing of CR's. If this is your
first time using CRTS, we recommend that you take the time to run through
the tutorials.

## Scenarios {#Scenarios}

The crts\_controller will run the tests specified by a scenario master configuration
file. The default configuration file is scenario\_master\_template.cfg. A different
configuration file may be used by providing a -f option to crts\_controller. The
configuration file specifies the number of scenarios to be run, their names, and
the number of times each scenario will be run which can be specified once for all
scenarios, or for each individual scenario. If both methods are used, the number
provided for the specific scenario will take precedence. Syntax for specifying
these parameters must align with the provided configuration files.

If you just want to run a single scenario, you may simply provide a -s argument
to crts\_controller to avoid having to edit a configuration file.

Scenarios are defined by configuration files in the scenarios/ directory. Each of 
these files will specify the number of nodes in the experiment and the duration
of the experiment. Each node will have additional parameters that must be specified. 
These parameters include but are not limited to:
- The node's type: cognitive radio or interferer.
- The node's local IP address.
- If it is a CR node, it further defines:
    + The type of the CR (e.g. if it uses the ECR or some external CR).
    + The node's virtual IP address in the CR network.
    + The virtual IP address of the node it initially communicates with.
    + The network traffic pattern (stream, burst, or Poisson)
	+ If the CR node uses the ECR, it will also specify:
        * Which cognitive engine to use.
        * The initial configuration of the CR. 
        * What logs should be kept during the experiment.
- If it is an interferer node, it further defines:
    + The type of interference (e.g. OFDM, GMSK, RRC, etc.).
    + The paremeters of the interferer's operation.
    + What type of data should be logged.

In some cases a user may not care about a particular setting e.g. the forward
error correcting scheme. In this case, the setting may be neglected in the
configuration file and the default setting will be used.

We've provided a number of scenario files useful for orientation to CRTS
and for software testing and performance testing of the radios

## Scenario Controllers {#Scenario_Controllers}

Scenario controllers provide a centralized and customizable way to receive 
feedback and exert control over a scenario's operation in real time. A
simple API can be used to enable or disable specific types of feedback from
each node involved in the scenario, receive said feedback, and even directly
control the scenario test parameters e.g. the network throughput as well as
the operating parameters of the radio e.g. its transmit power.

We have found several significant use cases for this functionality. It 
provides a nice way to automate performance testing of the radios, it
provides an easy way to create dynamic test conditions such as network
loads, and it creates a central hub through which other applications 
can interface e.g. CORNET 3D can now be used to visualize tests as they 
happen and allows for human control of the radios which can be useful for 
tutorials.

The behavior of the scenario controller is defined by two functions. The
initialize node feedback function is called at the beginning of the scenario
so that feedback can be setup once since this will often be a static setting.
The execute function implements the behavior of the scenario controller. It 
is triggered whenever feedback is received or after a certain period of time 
has passed, specified by the sc\_timeout\_ms parameter in scenario config files.

To make a new scenario controller a user needs to define a new scenario controller
subclass. The SC\_Template.cpp and SC\_Template.hpp can be used as a guide in terms
of the structure and API. Once the SC has been defined it can be integrated into CRTS
by running $ ./config\_scenario\_controllers and $ make in the top directory.

Note: the node number scheme used by the scenario controller's API matches that
of the scenario configuration files i.e. numbering starts at 1.

## The Extensible Cognitive Radio {#The_Extensible_Cognitive_Radio}

As mentioned above, the ECR uses an OFDM based waveform defined by liquid-dsp. The
cognitive engine will be able to control the parameters of this waveform such as
the number of subcarriers, subcarrier allocation, cyclic prefix length, modulation
scheme, and more. The cognitive engine will also be able to control the settings 
of the RF front-end USRP including its gains, sampling rate, center frequency,
and digital mixing frequency. See the code documentation for more details.

Currently the ECR does not support much in the way of MAC layer functionality,
e.g. there is no ARQ or packet segmentation/concatenation. This is planned for
future development.

## Cognitive Engines in the ECR {#Cognitive_Engines_in_the_ECR}

The Extensible Cognitive Radio provides an easy way to implement cognitive 
engines. This is accomplished through inheritance i.e. a particular cognitive 
engine can be implemented as a subclass of the cognitive engine base class 
and seamlessly integrated with the ECR. The general structure is such that the 
cognitive engine has access to any information related to the operation of the 
ECR via get() function calls as well as metrics passed from the receiver DSP. 
It can then control any of the operating parameters of the radio using set() 
function calls defined for the ECR.

The cognitive engine is defined by an execute function which can be triggered by
several events. The engine will need to respond accordingly depending on the type
of event that occurred. The event types include the reception of a physical layer
frame, a timeout, USRP overflows and underruns, and transmission complete events.

To make a new cognitive engine a user needs to define a new cognitive engine
subclass. The CE\_Template.cpp and CE\_Template.hpp can be used as a guide in terms
of the structure, and some of the other examples show how the CE can interact
with the ECR. Once the CE has been defined it can be integrated into CRTS
by running $ ./config\_cognitive\_engines and $ make in the top directory.

Other source files in the cognitive\_engine directory will be automatically
linked into the build process. This way you can define other classes that your
CE could instantiate. 

A couple of notes on syntax for cognitive engines:
- The header and source files of a cognitive engine must reside in a directory which
names the cognitive engine. 
- The name of the directoy, the header and source files, and the class itself must all match.
- The name used must begin with CE\_
- The header must be a .hpp file and the source must be a .cpp file
- Other sources used can be .c, .cc, or .cpp files

If you just copy the CE\_Template directory and replace Template with your desired
name, everything should work out.

Installed libraries can also be used by a CE. For this to work you'll need to manually
edit the makefile by adding the library to the variable LIBS which is located at the
top of the makefile and defines a list of all libraries being linked in the final
compilation.

Examples of cognitive engines are provided in the `cognitive_engines/` directory.

### Exchanging control information using between two ECR's

Most likely your radios will need to exchange some control information over the
air to adapt effectively to channel and interference conditions. The ECR provides 
two possible ways to exchange such information.

The first method is to call the ECR.transmit\_control\_frame() function. This will
transmit a dedicated control frame which allows you to send arbitrarily large 
amounts of control information using its payload. You can check for control frames
in the receiving cognitive engine, and process it whenever one is found.

The alternative is to call the ECR.set\_control\_information() function at the
transmitter and the ECR.get\_control\_information() at the receiver. This allows
you to send 6 bytes of control information in the header of the next data frame.
This way your data flow is not interrupted.

The tradeoffs between these two methods include the following. A dedicated control
frame can support more control information. A dedicated control frame can have
lower latency assuming its payload is smaller than the payload of a data frame
(a combination of the two methods can be used, allowing you to transfer 6 bytes
of control information with a payload of 0 bytes for example). For small amounts
of control information that can afford slightly higher latency, using the
control information of a data frame will be more efficient.

## Cognitive Radios in Python {#Cognitive_Radios_in_Python}

Along with cognitive engines defined by the ECR, CRTS also supports cognitive radios written in python. An
example of a very simple python cognitive radio can be found in cognitive\_radios/python\_txrx.py.  When using a python radio, the scenario file
must specify the cr\_type ("python") and a python\_file (python\_txrx.py in the example scenario). The python file must be
in the top-level cognitive\_radios folder. In addition, you can supply arguments to pass to your radio by using the
arguments field. An example scenario file for a python radio can be found in scenarios/example\_scenarios/python\_flowgraph\_example.cfg.

## Interferers {#Interferers}

The testing scenarios for CRTS may involve generic interferers. There are a number
of parameters that can be set to define the behavior of these interferers. They
may generate CW, GMSK, RRC, OFDM, or AWGN waveforms. Their behavior can be defined
in terms of when they turn off and on by the period and duty cycle settings, and 
there frequency behavior can be defined based on its type, range, dwell time, and
increment.

## Logs and Basic Result Analysis {#Logs_and_Basic_Result_Analysis}

There are a number of log types that may be kept during the execution of CRTS. These
logs are written as binary files in the /logs/bin directory to reduce overhead, and
are automatically converted to Octave/Matlab scripts and placed in the /logs/octave  
directory after each scenario has finished. These scripts provide the user with an 
easy way to import data from experiments. Other scripts can then be written to analyze
the test results. We've provided some basic scripts to plot the contents of the logs 
as a function of time and display some basic statistics.

### Summary logs

This log type will contain summary information from a batch of test scenarios that were
launched using a single scenario master configuration file. The log is kept if the
parameter octave\_log\_summary is set to 1 in the scenario master configuration file.

You can use the statistics\_tool.m script to look at statistics over the set of tests
that were run in a number of dimensions.

### CRTS packet logs

CRTS will log packet transmission and reception details at the network layer if the
appropriate flags are set in the scenario configuration file. Each entry will include
the number of bytes sent or received, the packet number, and a timestamp. These may
be used to look at network layer metrics such as dropped packets or latency. Note that
latency calculations can only be as accurate as the synchronization between the server
nodes.

### Sysout logs

When CRTS is run in automatic mode, the print statements that would show up for each 
node in manual mode are redirected to log files in /logs/sysout.

### ECR logs
The ECR will also log frame transmission and reception parameters and metrics at the
physical layer if the appropriate flags are set in the scenario configuration file.

In the case where a scenario is run more than once (using the scenario\_reps field in
the master\_scenario\_file.cfg), the data from all repetitions will be held in a single
Octave script. Rather than a single array for each parameter there will be a cell array
for each parameter, each element of the cell array is an array which comprises the results
from a particular repetition. This is done to facilitate analysis across the repetitions.

## Debug {#Debug}

In the event that you experience issues running CRTS there are some simple things you can
try to help you debug. The first is reading the sysout logs if you've been running CRTS
in automatic mode. The main processes/classes in CRTS also have a debug compile time
option at the top of the source files. Setting this to 1 will provide additional information
to be printed out which may help you identify the issue.

## Physical Layer Performance Evaluation {#Physical_Layer_Performance_Evaluation}

Although the main purpose of CRTS is to facilitate cognitive engine test and development,
it can also be useful to look at the radios physical layer performance in different
operating regimes. The SC\_Performance\_Sweep scenario controller was written to address
this need. You can very easily create a new sweep configuration to look at a particular
parameter or set of parameters. The radio's performance will be output to a CSV file
in logs/csv/. Below are several examples of performance curves obtained using this
scenarios controller.

\image latex CRTS_Throughput_vs_Tx_Gain.png "Throughput vs. Tx Gain" width=12cm

\image latex CRTS_EVM_vs_Tx_Gain.png "EVM vs. Tx Gain" width=12cm

\image latex CRTS_PER_vs_Estimated_EVM.png "PER vs. Estimated EVM" width=12cm

# Notes to Developers {#Notes_to_Developers}

When this project was first started there weren't any specific coding styles or
practices that were strictly followed. As the project has grown and with more
people working with the code, it's become more important to make it readable to
a general audience. To that end, the following list of coding style should be
followed as much as possible. These rules are fairly common, and were taken
directly from Google's coding style guide.

## Naming Conventions {#Naming_Conventions}

More explicit names are preferred over short names that make it more difficult
to grasp the purpose of the variable.

All directories and files should be named using all lowercase letters with underscores
between words e.g. an_example_file.cpp.

General purpose variables should be named using all lowercase letters with underscores
between words e.g. an_example_variable.

Defined constants should be named with all capital letters and underscores between
words e.g. AN_EXAMPLE_DEFINED_CONSTANT.

Classes should be named with capital first letters and no underscores between words
e.g. AnExampleClass.

Exceptions to the above rules include the scenario controller and cognitive engine subclasses.
This is because the directory/file names need to match the class names. We therefore use the 
convention of naming them with the required SC or CE prefix followed by the unique name which 
uses capital first letters and underscores between words e.g. CE_An_Example_Cognitive_Engine.cpp.

## Formatting {#Formatting}

We try to stick to LLVM style formatting to keep things consistent. If you are unfamiliar
it's pretty easy to run clang-format which will take care of this for you.

Regardless, it'd probably be best if you set the preferences of your editor to insert 2
spaces for each tab. This will keep whitespace consistent for everyone. No one likes
opening up a file and having things shifted all over the place.





