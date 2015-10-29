The rapr_tracker tools are templates for plotting rapr data.

By default they will plot the sample log files included in this
directory but can easily be modified to plot multiple input files and
other interesting data.

rapr_parser.py can be run from the command line and will popup plots
on the display.  A few command line options are available to change
the default input files or the script itself may be easily modified.

Usage:

python rapr_parser.py [-r raprLogFile] [-m mgenLogFile]

rapr_tracker.ipynb is a python notebook designed to be run from within
a web browser.  To start the notebook (after installing any required
python modules and starting a web browser):

ipython notebook --pylab=inline

Next, access the IP[y]: Notebook that should be launched within your
browser.  Click on rapr_tracker to start the default notebook.  

By default it will plot the sample client and server log files
included in the rapr_tracker directory but can be easily modified to
plot other data and files.

Sample notebooks are included to illustrate how to plot data for some
included rapr "applications", e.g. plot_server_muc_example.ipynb and
plot_simple_http_example.ipynb
