#!/usr/bin/env python
#python 2.7
#rapr parse - parse rapr dictionary and input files
#@author Brandon Katz - 6/12/2014

from xml.etree import ElementTree as ElemTree
from xml.etree.ElementTree import Element, SubElement, Comment
from xml.dom import minidom
from optparse import OptionParser

import string
import os
import ConfigParser
import json

#hard-coded xml generation
def main():

    parser = OptionParser()
    parser.add_option("-f", "--file", dest="filename", help="Input File - required", metavar="FILE", default=None)
    parser.add_option("-l", "--log", dest="logfile", help="Log Filename - optional", default=None)
    parser.add_option("-i", "--input", dest="infile", help="Input Script Filename - optional", default=None)

    (options, args) = parser.parse_args()

    if (options.filename == None) :
        print "Must give input file with: -f <filename>"
        return

    configParser(options)

    #print prettify(top)

#read config and split up dict and input file parsing
def configParser(options):

    filename = options.filename

    config = ConfigParser.RawConfigParser()
    config.read(filename)

    if options.logfile == None :
        if config.has_option('paths', 'logname') :
            options.logfile = config.get('paths', 'logname')
    if options.infile == None :
        if config.has_option('paths', 'inputname') :
	    options.infile = config.get('paths', 'inputname')

    logfile = None
    if options.logfile == None :
        logfile = filename
    else :
        logfile = options.logfile
    infile = None
    if options.infile == None :
        infile = filename
    else :
        infile = options.infile

    dictParser(config, options)
    inputParser(config, options)

def dictParser(ConfigParser, options):

    filename = options.filename
    logfile = None
    if options.logfile == None :
        logfile = filename
    else :
        logfile = options.logfile
    infile = None
    if options.infile == None :
        infile = filename
    else :
        infile = options.infile
    
    os.chdir(ConfigParser.get('paths', 'scripts'))    

    top = Element('RaprDictionary')
    sysdefaults = SubElement(top, 'namespace')
    sysDefLabel = SubElement(sysdefaults, 'label')
    sysDefLabel.text = 'SystemDefaults'

    for k,v in ConfigParser.items('SystemDefaults') :
        item = SubElement(sysdefaults, 'item')
        tempf = SubElement(item, 'field')
        tempf.text = k.upper()
        tempv = SubElement(item, 'value')
        tempv.text = v.upper()

    default = SubElement(top, 'namespace')
    defLabel = SubElement(default, 'label')
    defLabel.text = 'DEFAULT'

    for k,v in ConfigParser.items('dict-defaults') :
        item = SubElement(default, 'item')
        tempf = SubElement(item, 'field')
        tempf.text = k.upper()
        tempv = SubElement(item, 'value')
        tempv.text = v.upper()

    for k,v in ConfigParser.items('nodes') :
        item = SubElement(default, 'item')
        tempf = SubElement(item, 'field')
        tempf.text = k.upper()
        tempv = SubElement(item, 'value')
        tempv.text = v.upper()

    #print prettify(top)
    filesplit = string.split(filename, '.')
    filename = filesplit[0]
    f = open(filename+'-dictionary.xml', 'w+')
    f.write(prettify(top))
    f.close()

def inputParser(configParser, options):

    filename = options.filename
    filesplit = string.split(filename, '.')
    filename = filesplit[0]
    logfile = None
    if options.logfile == None :
        logfile = filename
    else :
        logfile = options.logfile
    infile = None
    if options.infile == None :
        infile = filename
    else :
        infile = options.infile

    #os.chdir(configParser.get('paths', 'scripts'))
    for k,v in configParser.items('nodes') :
        f = open(k.upper()+'-'+infile+'.input', 'w+')
        f.write('#Auto-generated rapr input script\n')
        f.write('#Use for '+k.upper()+' with expected ip '+v+'\n')
        f.write('\n#DEFAULTS\n\n')
        for l,b in configParser.items('input-defaults') :
            f.write(b+'\n')
        f.write('\n')
        f.write('LOAD_DICTIONARY '+os.getcwd()+'/'+filename+'-dictionary.xml\n')
        f.write('\n')
        f.write('OVERWRITE_MGENLOG '+os.getcwd()+'/'+k.upper()+'-'+logfile+'-mgen.log\n')
        f.write('OVERWRITE_RAPRLOG '+os.getcwd()+'/'+k.upper()+'-'+logfile+'-rapr.log\n')
        f.write('\n')
        f.close()

    for k,v in configParser.items('networks') :

        net = json.loads(v)
        if net['topology'].find('-') >= 0:
            topSplit = string.split(net['topology'], "-")
            netParse(configParser, options, net, topSplit[0], topSplit[1])
            netParse(configParser, options, net, topSplit[1], topSplit[0])
            
        else :
            topSplit = string.split(net['topology'], ">")
            netParse(configParser, options, net, topSplit[0], topSplit[1])  

#parses networks and writes to input files, takes in a dict representation of a network 
def netParse(configParser, options, net, send, recv): 

    filename = options.filename
    filesplit = string.split(filename, '.')
    filename = filesplit[0]
    logfile = None
    if options.logfile == None :
        logfile = filename
    else :
        logfile = options.logfile
    infile = None
    if options.infile == None :
        infile = filename
    else :
        infile = options.infile
 
    sndPt = "%SEND_PORT%"
    rxPt = "%LISTEN_PORT%"
    if "send_port" in net :
        sndPt = net["send_port"]
    if "listen_port" in net :
        rxPt = net["listen_port"]

    bitsPerPacket = float(net["filesize"])*8
    pps = float(net["data_rate"])/bitsPerPacket
    pps = str(pps)


    f = open(send+'-'+infile+'.input', 'a+')
    sendCmd = "0.0 DURATION "+net["duration"]+" DECLARATIVE "+net["mode"]+" SRC "+sndPt+" DST "+configParser.get("nodes", recv.upper())+"/"+rxPt+" PERIODIC ["+pps+" "+net["filesize"]+"]"

    f.write(sendCmd)
    f.write("\n\n")
    f.close()

    f = open(recv+'-'+infile+'.input', 'a+')
    recvCmd = "0.0 DURATION "+net["duration"]+" LISTEN "+net["mode"]+" "+rxPt
    f.write(recvCmd)
    f.write("\n\n")
    f.close()




#Found online at google code by spatialguru
def prettify(elem):
    """Return a pretty-printed XML string for the Element.
    """
    rough_string = ElemTree.tostring(elem, 'utf-8')
    reparsed = minidom.parseString(rough_string)
    return reparsed.toprettyxml(indent="  ")


if __name__ == "__main__":
    main()

