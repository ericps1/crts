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


Situational Awareness EXAMPLE

The SA app is a very simple application designed to listen to and send
"situational awareness" multicast packets.  It uses a dictionary to
perform some simple translations such as multicast address and port.


1. Run the sa.input script on each node participating in the
application.

./rapr event "HOSTID <n>" input sa-example/sa.input

