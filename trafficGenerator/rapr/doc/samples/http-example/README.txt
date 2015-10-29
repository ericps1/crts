This sample http "client-server" application is described in the 
rapr tutorial in the docs directory.

1. Modify the dictionary-http.xml file to reflect the correct
IP addres for the node you wish to use as the http server.

Modify the http-client.input and http-server.input scripts to
point to the correct paths for your environment.

2. To run the application, first start the "http server" on the
server node:

./rapr input http-server.input

On the client node start the client:

./rapr input http-client.input.

(You may modify the test stop times and the periodicity 
interval to shorten or lengthen the test.  See the 
examples in the input scripts.)

Note that if the rapr application is started from within the
http-example directory the scripts should work without requiring any
path changes.

3. If you have chosen to log the mgen output, you may use NRL's 
TRPR application to create a gnuplot with the following command:

./trpr drec input mgen-server-http.log auto X output mgen-server-http.plot

Display the plot file via:

gnuplot -persist mgen-server-http.plot

