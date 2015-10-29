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

NOTE: This example will be started automatically when the core imn is
started.  After log files are created the plot_server_muc_example
in rapr-tracker can be used to view some sample plots of the default
log files created.

A sample server based unicast multiuser application is contained in
this directory.  The behavior is as follows:

Every 30 seconds to two minutes each client has a 50% probability of
doing something.  If yes:
     
     - with 10% probability send a "query packet" to the server
     - with 90% probability send a "chat request" to the server

1. Modify the dictionary file to change the default IP address for the
node you wish to use as the server.

2.  Modify the dictionary to change the client list from the default

2. To run the application, first start the "chat server" 

./rapr event "HOSTID <n>" input server-muc-example/muc-server.input

Then start the "clients" on each client node:

./rapr event "HOSTID <n>" input server-muc-example/muc-client.input

(You may modify the duration of the periodicity interval to shorten or
lengthen the test)

