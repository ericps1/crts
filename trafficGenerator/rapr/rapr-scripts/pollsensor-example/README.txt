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

POLL-SERVER-EXAMPLE

A sample poll sensor application is contained in this directory

Every 1 to 3 minutes the "poller" will query the "sensors", sending
logic id 1.  The sensors will replay with a randomly sized tcp stream.

1. Modify the dictionary file to reflect the correct IP address for the
node you wish to use as the poller.

2. To run the application, first start the "poll sensors" on the nodes
you wish to be sensors

./rapr event "HOSTID <n>" input pollsensor-example/sensor.input

Then start the "poller":

./rapr event "HOSTID <n>" input pollsensor-example/poller.input

(You may modify the duration of the periodicity interval to shorten or
lengthen the test)

