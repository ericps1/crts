# This includes a rapr log parser that is used to store time series rapr events
#
# by Joe Macker 2014
# 

import time
from datetime import datetime
import re
import matplotlib.pyplot as plt

def secs_from_datetime(timetuple):
    """Convert a time tuple stored as
    %H:%M:%S.%f
    Returns a seconds float value.
    """
    secs = timetuple.hour*3600
    secs += timetuple.minute*60
    secs += timetuple.second
    secs = str(secs) + '.' + str(timetuple.microsecond)
    return float(secs)

def parse_mgen_events(filename='test',eventType='SEND'):
    """Parse an associated RAPR mgen log file and store events
   Each log line is stored as a dictionary
   Event values can be accessed via dictionary keys 
   Keys represent event type fields
    """     
    f = open(filename)
    # readlines() will return the data as a list of strings, one for each line
    contents = f.readlines()
    # close the log file
    f.close()

    # Call the parse function
    mgen_data=[]
    start_time = contents[0].partition(" ")[0]
    start_time = datetime.strptime(start_time, "%H:%M:%S.%f")
    start_secs = secs_from_datetime(start_time)
    for line in contents:
        logtime = line.partition(" ")[0]
        if eventType not in line: continue
# Convert the date string format into a tuple then convert to seconds
        logtime = datetime.strptime(logtime, "%H:%M:%S.%f")
        secs = secs_from_datetime(logtime)
        d = {}
        d['time'] = float(secs)
        rapr_log_line = line.partition(" ")[2:][0]
        s=rapr_log_line
        match = re.findall(r'([\w-]+)>([A-Za-z0-9-.:]+)',s,re.IGNORECASE)
        for element in match:
            d[element[0]] = element[1]      
        mgen_data.append(d)    
    return mgen_data,start_secs

def parse_rapr_events(filename='test'):
    """Parse an associated RAPR log file and store events
   Each log line is stored as a dictionary
   Event values can be accessed via dictionary keys 
   Keys represent event type fields
    """      
    f = open(filename)
    # readlines() will return the data as a list of strings, one for each line
    contents = f.readlines()
    # close the log file
    f.close()

    # Call the parse function
    rapr_data=[]
    start_time = contents[0].partition(" ")[0]
    start_time = datetime.strptime(start_time, "%H:%M:%S.%f")
    start_secs = secs_from_datetime(start_time)
    for line in contents:
        logtime = line.partition(" ")[0]
# Convert the date string format into a tuple then convert to seconds
        logtime = datetime.strptime(logtime, "%H:%M:%S.%f")
        secs = secs_from_datetime(logtime)
        d = {}
#        d['time'] = float(secs-start_secs)
        d['time'] = float(secs)
        rapr_log_line = line.partition(" ")[2:][0]
        s=rapr_log_line
#        match = re.findall(r'([\w-]+)>([\w-]+)',s,re.IGNORECASE)
        match = re.findall(r'([\w-]+)>([\w-]+)',s,re.IGNORECASE)
        for element in match:
            d[element[0]] = element[1]
        match = re.findall(r'([mgenCmd]+)>("[^"]*")',s,re.IGNORECASE)
#        match = re.findall(r'([mgenCmd]+)>(?<=")[^"]*(?=")',s,re.IGNORECASE)
# trying to remove the quotes from mgen sequence not a regex expert
# getting mgen command
        for element in match:
            d[element[0]] = element[1]
        rapr_data.append(d)
    
    return rapr_data,start_secs

def get_logicid_events(rapr_data):
    """Get Events and return as a series for plotting purposes
    """ 
    import matplotlib as plt
    
# Stored as list of dictionaries
    series=[]
    for event in rapr_data:
# Test for doLogicID key in dictionary
        if event.get('app') == 'RAPR':
            action = event.get('action')
            if action == 'doLogicID':
                etype = event.get('result')
                if etype == 'parsingEvent':
                    time_of_event = event.get('time')
                    logicID = event.get('logicID')
                    #logicID = '1.' + logicIDtmp
                    series.append((time_of_event,int(logicID)))
                elif etype == 'randTestFailure':
                    time_of_event = event.get('time')
                    series.append((time_of_event,10.0))
                else:
                    type = event.get('type')
                    if type == 'Error':
                        time_of_event = event.get('time')
                        series.append((time_of_event,0))
                   # pass
    return series
    
def get_mgen_dst_events(rapr_data):
    """Get Events and return as a series for plotting purposes
    """ 
    import matplotlib as plt
# Stored as list of dictionaries
    series=[]
    for event in rapr_data:
# Test for doLogicID key in dictionary
           
            dst = event.get('dst')
            time_of_event = event.get('time')
            if dst == '10.0.0.1':
                series.append((time_of_event,1))
            elif dst == '10.0.0.2':
                series.append((time_of_event,2))
            elif dst == '10.0.0.3':
                series.append((time_of_event,3))
            else:
                 pass
    return series

def get_mgen_size_events(rapr_data):
    """Get Events and return as a series for plotting purposes
    """ 
    import matplotlib as plt
# Stored as list of dictionaries
    series=[]
    for event in rapr_data:
        size = event.get('size')
        time_of_event = event.get('time')
        series.append((time_of_event,int(size)))
    return series

def get_rapr_events(rapr_data):
    """Get Many Events and return as a series for plotting purposes
    """ 
    series=[]
    for event in rapr_data:
# Test for eventSource key in dictionary
        if event.get('app') == 'RAPR':
            type = event.get('type')
            if type == 'Application':
                time_of_event = event.get('time')
                series.append((time_of_event,1.0))                
            elif type == 'RaprEvent':
                time_of_event = event.get('time')
                series.append((time_of_event,2.0))                
            elif type == 'Reception':
                time_of_event = event.get('time')
                series.append((time_of_event,3.0))                
            elif type == 'logicTable':
                time_of_event = event.get('time')
                series.append((time_of_event,4.0))                
            elif type == 'Declarative':   
                time_of_event = event.get('time')
                series.append((time_of_event,5.0))
            elif type == 'Interrogative':
                if 'action' in event:
                    if event.get('action') == 'start':
                        time_of_event = event.get('time')
                        series.append((time_of_event,6.0))
                    elif event.get('action') == 'success':
                        time_of_event = event.get('time')
                        series.append((time_of_event,6.2))
                    elif event.get('action') == 'failure':
                        time_of_event = event.get('time')
                        series.append((time_of_event,6.4))
                    elif event.get('action') == 'timeout':
                        time_of_event = event.get('time')
                        series.append((time_of_event,6.6))
                    else:
                        pass
            elif type == 'mgenMsgRecv':                
                time_of_event = event.get('time')
                series.append((time_of_event,7.0))
            elif type == 'Error':                
                time_of_event = event.get('time')
                series.append((time_of_event,8.0))
            else:
                pass
    return series

def plot_rapr_events(series,name,series2='null',name2='null'):
#Plotting Section
    plt.title('RAPR Events')
# Set fictitious y axis to plot different event types
    plt.ylim(0,11)
# Set timelimits for desired plotting
#plt.xlim(59400,59700)

    plt.yticks([1,2,3,4,5,6.0,6.2,6.4,6.6,7,8],
      [r'Application', r'RAPR Event',\
        r'Reception', r'Logic Table',\
          r'Declarative', r'Interrogative Start',\
           r'Success',r'Failure',r'Timeout',\
          r'MGEN Message Rcv', r'Error']\
      )
    plt.scatter(*zip(*series),label=name,marker='x',color='b')
    if name2 != 'null':
        plt.scatter(*zip(*series2),label=name2,marker='x',color='r')
    plt.legend()
    plt.show()

def plot_logicid_events(series1,name,series2='null',name2='null'):
    plt.title('RAPR LogicID Events')

# Set fictitious y axis to plot different event types
    plt.ylim(0,11)
# Set timelimits for desired plotting
#plt.xlim(59400,59700)
# Set figure size
#    figsize(10,3)

    plt.yticks([0,1,2,3,4,5,6,7,8,9,10],
          [r'Error',r'Logic ID 1',r'Logic ID 2',r'Logic ID 3',
           r'Logic ID 4',r'Logic ID 5', r'Logic ID 6',r'Logic ID 7',
           r'Logic ID 8',r'Logic ID 9',r'Random Test Failure'])
    plt.scatter(*zip(*series1),label=name,marker='x',color='b')
    if series2 != 'null':
        plt.scatter(*zip(*series2),label=name2,marker='x',color='r')

    plt.legend()
    plt.show()

def plot_mgen_size_events(series,name,series2='null',name2='null',ylim=38192):
    plt.title('Mgen Send Events')

# Set fictitious y axis to plot different event types
    plt.ylim(0,ylim)
# Set timelimits for desired plotting
#plt.xlim(59400,59700)

    plt.scatter(*zip(*series),label=name,marker='x',color='b')
    if series2 != 'null':
        plt.scatter(*zip(*series2),label=name2,marker='x',color='r')
    plt.legend()
    plt.show()

#not used right now
def parse_rapr(logger, line):
    # Split the line into fields
    date, metric_name, metric_value, attrs = line.split('|')

    # Convert the iso8601 date into a unix timestamp, assuming the timestamp
    # string is in the same timezone as the machine that's parsing it.
    date = datetime.strptime(date, "%H:%M:%S.%MS")
    date = time.mktime(date.timetuple())

    # Remove surrounding whitespace from the metric name
    metric_name = metric_name.strip()

    # Convert the metric value into a float
    metric_value = float(metric_value.strip())

    # Convert the attribute string field into a dictionary
    attr_dict = {}
    for attr_pair in attrs.split(','):
        attr_name, attr_val = attr_pair.split('=')
        attr_name = attr_name.strip()
        attr_val = attr_val.strip()
        attr_dict[attr_name] = attr_val

    # Return the output as a tuple
    return (metric_name, date, metric_value, attr_dict)

from optparse import OptionParser

def main():
    rapr_logfile=""
    parser = OptionParser(usage="usage: %prog [options]",
                          version="%prog 0.9")
    parser.add_option("-r", "--raprlogfile",
                      type="string",
                      dest="raprlogfile",
                      default="rapr-server-http.log",
                      help="input file for rapr tracker rapr log parser")
    parser.add_option("-m", "--mgenlogfile",
                      type="string",
                      dest="mgenlogfile",
                      default="mgen-server-http.log",
                      help="input file for rapr tracker mgen log parser")
    (options, args) = parser.parse_args()



# Parse an associated RAPR log file and storelogfile
    rapr_data,start_time1 = parse_rapr_events(filename=options.raprlogfile)

# Get rapr events from rapr_data & plot
    series = get_rapr_events(rapr_data)


# Get logicids from rapr_data & plot
    series = get_logicid_events(rapr_data)
    plot_logicid_events(series,options.raprlogfile)

# Parse and associate MGEN log file and store
    mgen_data,start_time1 = parse_mgen_events(filename=options.mgenlogfile,eventType='SEND')

# Get mgen size data from mgen_data & plot
    series = get_mgen_size_events(mgen_data)
    plot_mgen_size_events(series,options.mgenlogfile)

# Note that you can plot multiple input files, e.g.
#    mgen_data,start_time1 = parse_mgen_events('./mgen-client-http.log')
#    series2 = get_mgen_size_events(mgen_data)
#    plot_mgen_size_events(series,'server',series2,'client')

if __name__ == '__main__':
    main()






