This directory contains five sample rapr "applications".

NOTE: If the rapr binary is invoked from within the apps directory,
the paths in the scripts and dictionaries will not need to be changed.


1.  Situational Awareness Application

This is a very simple application designed to listen to and send
"situational awareness" multicast packets.  It uses a dictionary to
perform some simple translations such as multicast address and port.

Files:

a.  raprScript-SA.rapr is the input script to be run by each node
participating in the application.

b.  dictionary-sa.xml modify to change multicast addresses, ports etc.

Usage:

On each node participating in a scenario, run:

./rapr input raprScript-SA.rapr


2. Intel Application

This application consist of a three node network.  One node acts as
the "intel server", the other two as "intel clients".  A common
dictionary and logictable is used by all nodes in the application.

Key files in the intel application are:

dictionary-intel.xml
logictable-intel.xml
raprScript-intelServer.rapr
raprScript-intelClient.rapr-62
raprScript-intelClient.rapr-64

Modify the "Intel Server IP" dictionary entry as appropriate for your
network to set the server ip address.

Every 10 minutes the client nodes will query the server for
information.  The server will immediately confirm reception of the
query packet, and after an interval of time, return a second burst of
messages.  Upon receipt of the second traffic burst, the clients will
ask the server for more information.  The server will respond with a
third burst of traffic of random duration and pattern.

Some rapr commands illustrated in this example:

The clients use PERIODIC objects to spawn off INTERROGATIVE objects at
10 minute intervals.  These interrogative objects will send a request
for data (5 total requests) every 15 seconds until a response from the
server satifies the query.

The example uses state changes to toggle between UDP and TCP
communications.  It also uses the CHANGE_UBI_STATE rapr command that
is designed to handle multiple logic id requests from a behavior event
that should not result in multiple responses. See the rapr
documentation for more information on CHANGE_STATE and
CHANGE_UBI_STATE.

To run the sample application (after updating the dictionary xml file
to update the server ip address) issue the following commands on the
test nodes:

# Start the server first:

rapr input raprScript-intelServer.rapr

# First client:

rapr input raprScript-intelClient.rapr-62

# Second client:

rapr input raprScript-intelClient.rapr-64


3. Logistics Application

The logistics application consists of a five node network, two nodes
act as logistic servers, two as logistic clients, and five as nodes
acting as logistic sources.

This application illustrates changing the identity of the logistics
server via the run time interface during the application run.

3a. Files: 

dictionary-logistics.xml            raprScript-logisticsServer.rapr-63
logictable-logisticsClient.xml      raprScript-logisticsSource.rapr-61
logictable-logisticsServer.xml      raprScript-logisticsSource.rapr-62
logictable-logisticsSource.xml      raprScript-logisticsSource.rapr-63
raprScript-logisticsClient.rapr-61  raprScript-logisticsSource.rapr-64
raprScript-logisticsClient.rapr-64  raprScript-logisticsSource.rapr-65
raprScript-logisticsServer.rapr-62


3b. Logistic source:

The logistic source scripts should be run on all five nodes
participating in the application.  At time 0, each source will listen
on a udp update port and join and send traffic to the logistics
multicast group.
 
At time 0, each source will start a PERIODICTY object that will create
DECLARATIVE objects at a rate of 1 every five minutes.  Each
DECLARATIVE object will send a single UDP message of 1125 bytes with
logicId = 0 to the active server at the logistics server update port.

When a logistic source receives a message with a logic id 1, it will
respond with a 1125 byte UDP message to the packet's sender address. 

3c. Logistics client:

The logistics uses a PERIODIC object to spawn an INTEROGATIVE object
every 3 minutes.  It will send a 250 byte UDP message with logic id 2
to the active logistics server directing it to "serve" logistics
content.

3d. Changing the source/client's active server via the run time interface:

Commands to change the logistics server at the client and source nodes
are as follows:

- Set the LOG_ACTIVE_SVR dictionary value to $LOG_SRV_Y, where Y={1,2}

- Stop the current flow associated with RAPRFLOWID 1 going to
  LOG_ACTIVE_SRV

- Restart a new flow associated with RAPRFLOWID 1 going to
  LOG_ACTIVE_SRV (now set to %LOG_SRV_Y%

It is recommended that a script be used to send these commands to the
nodes.  The definition of the commands and script is left as an
exercise for the reader.

3e. Logistics server:

At time 0, the server designated as the active server (LOG_SVR_1),
will set its behavior table state to 1 (ACTIVE), the backup server
(LOG_SVR_2), will set its behavior table state to 0 (INACTIVE).

At time 0, both servers will send bursts of traffic to each other.

3e1. Logistics server BEHAVIOR TABLE:

- If a message of logic ID=2 arrives, and if the behavior table state
is 0 (INACTIVE), do nothing.

- If a message of logic ID=2 arrives, and if the behavior table state
is 1 (ACTIVE), respond with a declarative that sends a periodic UDP
flow of 2 second duration [10 1400] with logicID=0 to sender_address,
and destination port %LOG_CLI_LISTEN_PORT%.  ALSO, with 30%
probability, send a single 250 byte declarative UDP message to
multicast group %LOG_GRP_FWD_1%, destination port
$LOG_SRC_UPDATE_PORT, and with logicID=1.

3e2. Logistics Server RUN-TIME INTERFACE:

Commands to reprovision the logistics server are as follows:

- set the behavior table state to 1 (ACTIVE) 

- set the behavior table state to 0 (INACTIVE) 


3f. Logistics application dictionary:

All application component use the same dictionary:
dictionary-logistics.xml.

# Roles 

$CMDR = 61 (running logistics_source and logistics_client) 

$LOG_SVR_1 = 62 (running logistics_source and logistics_server) 

$LOG_SVR_2 = 63 (running logistics_source and logistics_server)

$LOG_ACTIVE_SVR = $LOG_SVR_1 (this is the starting value, but can be
changed via the run-time interface for re-provisioning) 

$CMDR_FWD_1 = 64 (running logistics_source and logistics_client)

$PRIV_FWD_1_SQD_1_1 = 65 (running logistics_source)

By default the ports/ip addresses are set as follows.  Change these as
appropriate for your environment.

$LOG_SRV_UPDATE_PORT = 1200
$LOG_SRC_UPDATE_PORT = 1201
$LOG_CLI_LISTEN_PORT = 1203
$LOG_SRV_LISTEN_PORT = 1204
$LOG_SRV_SVR_PORT = 1204
$LOG_SRV_SRV_PORT = 1205
LOG_SRV_SEND_PORT = 1206
LOG_CLI_SEND_PORT = 1207
LOG_SRC_SEND_PORT = 1208 
LOG_GRP_FWD_1 = 228.2.2.1 
LOG_SRV_1 = 192.168.2.62
LOG_SRV_2 = 192.168.2.63
LOG_ACTIVE_SRV = %LOG_SRV_1%


3g. Running the logistics application:

Here is an example of how to start and reprovision a logistics
application using the sample configuration on a testbed with nodes
61-65:

Note that errors will go to standard out with this configuration.
Redirect stdout to the output file in order to trap these errors.
(Some of these errors may be important)

Start the servers on nodes 62 and 63

./rapr input raprScript-logisticsServer.rapr-62 >
raprScript-logisticsServer.rapr-62.log

./rapr input raprScript-logisticsServer.rapr-63 >
raprScript-logisticsServer.rapr-63.log

Start the clients on 61 and 64

./rapr input raprScript-logisticsClient.rapr-61 >
raprScript-logisticsClient.rapr-61.log

./rapr input raprScript-logisticsClient.rapr-64 >
raprScript-logisticsClient.rapr-64.log

Start the sources on all the other nodes:

./rapr input raprScript-logisticsSource.rapr-61 >
raprScript-logisticsSource.rapr-61.log

./rapr input raprScript-logisticsSource.rapr-62 >
raprScript-logisticsSource.rapr-62.log

./rapr input raprScript-logisticsSource.rapr-63 >
raprScript-logisticsSource.rapr-63.log

./rapr input raprScript-logisticsSource.rapr-64 >
raprScript-logisticsSource.rapr-64.log

./rapr input raprScript-logisticsSource.rapr-65 >
raprScript-logisticsSource.rapr-65.log


Assuming a "set_logistics_server" script has been written invoke the
following commands to change the active server.  Otherwise enter the
run time command as described above.

Deactivate the server on 62:

set_logistics_server logisticsServer %LOG_SRV_2%
set_logistics_server_state INACTIVE 

Activate the server on 63:

set_logistics_server logisticsServer %LOG_SRV_2%
set_logistics_server_state ACTIVE

Change the active server for clients on 61 and 64:

set_logistics_server logisticsClient %LOG_SRV_2%
set_logistics_server logisticsClient %LOG_SRV_2%

Change the active server on all the sources 61 - 65

set_logistics_server logisticsSource %LOG_SRV_2%
set_logistics_server logisticsSource %LOG_SRV_2%
set_logistics_server logisticsSource %LOG_SRV_2%
set_logistics_server logisticsSource %LOG_SRV_2%
set_logistics_server logisticsSource %LOG_SRV_2%

4. VOIP Application

The voip application consists of a four node network, with all nodes
participating in a group chat.

Key files in the voip application distribution are:

dictionary-voip.xml     dictionary xml file
logictable-voip-61.xml  logic table xml file node 61
logictable-voip-62.xml  logic table xml file node 62
logictable-voip-63.xml  logic table xml file node 63
logictable-voip-64.xml  logic table xml file node 64
raprScript-voip.rapr-61 input script for node 61
raprScript-voip.rapr-62 input script for node 62
raprScript-voip.rapr-63 input script for node 63
raprScript-voip.rapr-64 input script for node 64

The sample application behaves as follows:

At time 0.0 all nodes start listening for multicast traffic on the
multicast address associated with GROUP_FWD1_SQUAD_1 in the voip
dictionary.

At time 2.0, node 61 starts a group conversation with the other nodes
in the scenario.  It starts a multicast packet stream from
VOIP_SEND_PORT to VOIP_LISTEN_PORT with the following pattern:

PERIODIC [25 100]

The packet stream sends a payload id of 1 to the target nodes,
directing them to join the group conversation (as dictated by the
definition of a stream object in their logic tables associated with
logic id 1).  The nodes are each associated with a probability of
response.  Using a seed transmitted in the initiating stream's
payload, each node determines whether it should be the next node to
respond to the conversation.  The probability of reply is determined
as follows:

node    probability of reply
----    --------------------
TN-61         0  -  15
TN-62        16  -  50
TN-63        51  -  85
TN-64        86  - 100

The initiating node also defined the "length" of the conversation (in
terms of "bursts of conversation") and embedded this count
(burstcount) in the initiating payload stream.  Each node that replies
will decrement this count when it determines it is eligible to respond
and will return this count in the payload it transmits.  When this
count reaches 0, the conversation ends.

Other variables that impact the behavior of the conversation are
defined in the stream object definitions in the input script and/or
behavior table and are as follows: (Note that not all these options
are defined in the sample scenario)

BURSTDURATION <duration in seconds>  

The duration of the "burst" in terms of seconds.  May be superceded
by BURSTRANGE.

BURSTDELAY <low><high>

The delay in seconds before the next burst should begin.  Using the
incoming seed, the stream objects will pick a random value within this
range.  

BURSTRANGE <low><high>

The range in seconds that the burst should last.  Using the incoming
seed, the stream objects will pick a random value within this range.

BURSTPRIORITY <priority>

The priority of the stream vis-a-vis other "conversations" the node
may be participating in.  Higher priority conversations take
precedence over lower, and will stop any lower priority active
conversations.  (e.g.  when the node is "talking").  Equal priority
conversations are handled on a first come first served basis.  Lower
priority conversations will not reply, if a higher conversation is
taking place.

TIMEOUTINTERVAL <interval>

A conversation will end if no responses to the conversation have been
received from any of the nodes within the timeout interval.

As the scenario procedes, the other nodes will trigger additional
"group conversations", TN-62 at 100.0 seconds, TN-63 at 160.0 seconds,
TN-64 at 260.0 seconds.


5. Video Application

The video application consists of a four node network, one node acting
as the video_server, and the other three nodes acting as
video_clients.

Key files in the video application distribution are:

dictionary-videoApp.xml       the dictionary xml file
logictable-videoApp.xml       the logic table xml file 
raprScript-videoServer.rapr   input script for the server
raprScript-videoClient-1.rapr input script for client-1
raprScript-videoClient-2.rapr input script for client-2
raprScript-videoClient-3.rapr input script for client-3


The sample application behaves as follows:

Server:

At time 0.0, the server starts a multicast packet stream to 228.1.1.1
from port 2000 to port 2001 with the following pattern:

PERIODIC [10 1000]
PERIODIC [1 1000]
POISSON [2 1000]

-At time 600, the server stops multicasting the current stream, and
starts multicasting a similar packet stream to multicast address
228.1.1.2.

-At time 1200, the server stops multicasting the current stream and
starts multicasting a similar stream to multicast address 228.1.1.3

-At time 1800, the server stops multicasting the current stream and
starts multicasting a similar stream to multicast address 228.1.1.1

-At time 2400, the server stops multicasting the current stream, and
starts multicasting a similar packet stream to multicast address
228.1.1.2.

-At time 3000, the server stops multicasting the current stream and
starts multicasting a similar stream to multicast address 228.1.1.3

-At time 3600, the server stops multicasting the current stream and
starts multicasting a similar stream to multicast address 228.1.1.1

Client:

For a 3 node network, there are 3 "clients". 

The first client node at time 0 joins the 228.1.1.1 multicast group to
listen on port 2001

The second client node at time 0 joins the 228.1.1.2 multicast group
to listen on port 2001

The third client node at time 0 joins the 228.1.1.3 multicast group to
listen on port 2001

To run the sample application (after updating the dictionary xml file
and rap~/samples/rScript-<appName>.rapr files as appropriate) issue the
following commands on the test nodes:

Server:

rapr input raprScript-videoServer.rapr

First Client:

rapr input raprScript-videoClient-1.rapr

Second Client:

video_client input raprScript-videoClient-2.rapr

Third Client:

video_client input raprScript-videoClient-3.rapr

