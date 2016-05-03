#!/usr/bin/env python
import fileinput
import sys
from subprocess import call

# Replace text for setting transmitter and receiver addresses in the scenario file
def replaceTextInScenarioFile(txIPAddress,rxIPAddress, scenarioFile='CORNET_3D.cfg'):
        parsedFirstNode = False
        parsedSecondNode = False

        strNode1 = "node1: {"
        strNode2 = "node2: {"

        strCORNETIP = '    CORNET_IP = '
        pathToScenario = 'scenarios/' + scenarioFile
        print 'path:', pathToScenario
        for line in fileinput.input(pathToScenario, inplace=True):
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

def replaceIPs(scenarioFile, IPs):
    pathToScenario = 'scenarios/' + scenarioFile
    strCORNETIP = 'CORNET_IP = '
    changed = 0
    foundNode = False
    for i in range(len(IPs)):
        node = 'node' + str(i + 1)
        for line in fileinput.input(pathToScenario, inplace=True):
            if line.startswith(node):
                foundNode = True
            if strCORNETIP in line and foundNode == True:
                replacedText = '    ' + strCORNETIP + '"' + IPs[i] + '";'
                print replacedText
                foundNode = False
                changed = changed + 1
            else:
                print line,
    print 'changed', changed, 'lines'

i = 2
ips = []
while i < len(sys.argv):
    ips.append(sys.argv[i])
    i = i + 1
replaceIPs(sys.argv[1], ips)
call(["./CRTS_controller"])
