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

def replaceIPsInScenarioFile(scenarioFile, IPs):
    pathToScenario = 'scenarios/' + scenarioFile
    strCORNETIP = 'server_ip = '
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


def rewriteMasterScenarioFile(scenarioFile):
    f = open("cornet_master_scenario_file.cfg", 'w')
    line = 'scenario_1 = "' + scenarioFile[:-4] + '";\n'
    f.write('num_scenarios = 1;\n')
    f.write('reps_all_scenarios = 1;\n')
    f.write(line)
    f.write('reps_scenario_1 = 1;')
    f.close()



i = 2
ips = []
while i < len(sys.argv):
    ips.append(sys.argv[i])
    i = i + 1
replaceIPsInScenarioFile(sys.argv[1], ips)
rewriteMasterScenarioFile(sys.argv[1])
call(["./crts_controller", "-f", "cornet_master_scenario_file"])
