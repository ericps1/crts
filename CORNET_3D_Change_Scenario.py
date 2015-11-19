#!/usr/bin/env python
import fileinput
import sys
from subprocess import call

# Replace text for setting transmitter and receiver addresses in the scenario file
def replaceTextInScenarioFile(txIPAddress,rxIPAddress):
        parsedFirstNode = False
        parsedSecondNode = False

        strNode1 = "node1: {"
        strNode2 = "node2: {"

        strCORNETIP = '    CORNET_IP = '
        for line in fileinput.input("scenarios/CORNET_3D.cfg", inplace=True):
                # First parse until "node1: {" is found
                if line.startswith("node1: {"):
                        parsedFirstNode = True
                # Then parse until "node1: {" is found
                if line.startswith("node2: {"):
                        parsedSecondNode = True
                # Modify the Transmitter IP address
                if line.startswith(strCORNETIP) and parsedFirstNode == True and parsedSecondNode == False:
                        replacedText = strCORNETIP + '"' + txIPAddress + '";'
                        print replacedText
                # Modify the Receiver IP address
                elif line.startswith(strCORNETIP) and parsedSecondNode == True:
                        replacedText = strCORNETIP + '"' + rxIPAddress + '";'
                        print replacedText
                else:
                        print line,

replaceTextInScenarioFile(sys.argv[1], sys.argv[2])
call(["./CRTS_controller"])
