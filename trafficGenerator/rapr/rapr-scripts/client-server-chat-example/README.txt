By default the example application scripts expect to be located in
the following path:

/home/rapr/rapr-scripts/<application-scripts>

Each application script will load a default input script located in the
rapr-scripts directory, e.g.:

/home/rapr/rapr-scripts/rapr-defaults.input

This script will load a default dictionary that defines paths used by
all the example applications:

LOAD_DICTIONARY /home/rapr/rapr-scripts/rapr-default-dictionary.xml

The rapr binary will look for the default scripts from its current
directory.  Therefore it is recommended to create a symlink to the
rapr binary in the rapr-scripts directory and invoke the applications
from there such that the applications should work "out of the box"
with no path changes required:

cd /home/rapr/rapr-scripts
ln -s <pathToRaprBinary>/rapr rapr
./rapr input <exampleDir>/<exampleScript>

Change the settings in these files if you deviate from this path
structure.  


GROUP-CHAT-EXAMPLE

The group chat application consists of a N node network, with all nodes
participating in a group chat.

The sample application behaves as follows:

At time 0.0 all nodes start listening for multicast traffic on the
multicast address associated with CHAT_MULTICAST_GROUP in the 
dictionary.

Each client sends a single tcp request every 30-60 seconds to the
server with logic id 1.

The server then choses which client (1,2,3, or 4) to send a message to
that will trigger a UDP flow to the multicast group (via logic ID 2).

The client then randomly chooses a reply pattern for a random duration
and sends a UDP stream to the multicast group.

The logictable can be modified to trigger the chat causality between
nodes.

1.  Modify the dictionary to change the default client addresses.

2.  Start the chat server on node 10.0.0.2

./rapr event "HOSTID 1" input client-server-chat-example/chat-server.input

2. Start the chat clients on nodes 10.0.0.1, 10.0.0.3, 10.0.0.4, 10.0.0.5

./rapr event "HOSTID <n>" input client-server-chat-example/chat.input

To plot all traffic received by the clients (replace the src node ip
address as appropriate):

/trpr/trpr recv flow X,::ffff:10.0.0.1/X,X/X,X flow
X,::ffff:10.0.0.2/X,X/X,X flow X,::ffff:10.0.0.3/X,X/X,X mgen input
client-server-chat-example/mgen-chat-server.log output server.plot

To plot all traffic to the multicast address (replace the src node ip
address as appropriate):

 ~/trpr/trpr recv flow X,::ffff:10.0.0.1/X,239.1.1.1/X,X flow
 X,::ffff:10.0.0.2/X,239.1.1.1/X,X flow
 X,::ffff:10.0.0.3/X,239.1.1.1/X,X mgen input
 client-server-chat-example/mgen-chat-server.log output test-mc.plot
