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



SIMPLE-HTTP-EXAMPLE

This very simplistic "web server" sample application is also described
in the rapr documentation in the docs directory.

The web server is modeled to be a simple stateless server that simply
responds to various client requests. The web client drives the
behavior by sending logic ids to the server that trigger its response,
for example to return an "HTML web page". The traffic patterns are
randomized such that the emulated behavior reflects the randomness of
real traffic patterns but is repeatable.

The sample application models two behavior "patterns":

- The client requests an HTML web page. After the server returns a
"web page", the client requests between one and three gif files.

- The client requests an HTML web page with a link to a file. The
server responds with a file between 512 and 38192 bytes.

1. Modify the dictionary-http.xml file in the http-examples directory
to reflect the correct IP address for the node if you wish to change the
default address.

2. To run the application, first start the "http server" on the server
node (see above notes on directory structures):

./rapr event "HOSTID <n>" input http-example/http-server.input

On the client node start the client:

./rapr event "HOSTID <n>" input http-example/http-client.input.

(You may modify the periodicty duration and interval to shorten or
lengthen the test and traffic frequency.  See the examples in the
input scripts.)

3. If you have chosen to log the mgen output, you may use NRL's TRPR
application to create a gnuplot with the following command:

By default trpr will log send data only.  Add the "send" and "recv"
data options to log both SEND and RECV data:

./trpr mgen input mgen-server-http.log auto X output mgen-server-http.plot

Display the plot file via:

gnuplot -persist mgen-server-http.plot

4. A http "validation" application is available for testing that
removes randomness from the application.  It can be used to verify
that a simple implementation of the application is working.

A plot of server SEND and RECV traffic is available for comparison.
