# Provided Cognitive Radio

## 1. python\_txrx.py
This is a simple python flowgraph that was developed using GNURadio-companion. It reads data from the tunCRTS virtual interface
using the TUNTAP\_PDU block, and passes it along to a second node via a Socket\_PDU block operating in server mode. The second node
receives the data from a Socket PDU block operating in client mode, then passes it to the TUNTAP PDU block to write the data to
the tunCRTS virtual interface. This radio can be used as a template for other python radios, by replacing the Socket\_PDU
blocks with USRP blocks to send the data over the air, rather than over the network.
