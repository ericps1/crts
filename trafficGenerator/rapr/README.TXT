RAPR Version 1.0.1

This directory contains source code for the NRL Real-Time Application
Representative (RAPR) Test Tool Version 1.0.1

RAPR can be used to generate and respond to network traffic in order
to emulate generic application behaviors, or "representative
applications".  It uses the Mgen toolkit to generate network traffic
and supports most of the traffic shaping functionality available in
Mgen Version 5.01b (see http://www.nrl.navy.mil/itd/ncs/products/mgen
for more details).

Rapr behavior is defined by a RAPR input script, a behavior table (or
logictable), and RAPR commands received over a run time interface.
The RAPR input script is used to schedule emulated application
behavior over a time line.  The run time interface can be used to
inject and schedule additional RAPR behaviors.  The behavior table
defines behavior in response to network or internally generated
events.  Additionally a dictionary can be used to define name-value
pairs used by the application.


FILES AND DIRECTORIES:

README.TXT   - this file

include      - include files

common       - Directory containing RAPR source code files

doc	     - rapr user's guide (rapr.html) and various readme files

rapr-scripts - sample simple rapr "applications"

rapr-scripts/rapr-tracker - beta rapr plotting python code

makefiles    - Directory with Unix-specific Makefiles

Required Distributions:

mgen-5.02* - NRL MGEN source tree (See
             <http://www.nrl.navy.mil/itd/ncs/mgen)

	     At the current time the nightly build is required.

protolib   - NRL Protolib source tree (See
             <http://www.nrl.navy.mil/itd/ncs/protolib)

sprng 4.4  - <http://sprng.cs.fsu.edu/>

Quick Start Installation:

The RAPR makefiles expect the mgen and protolib src trees to be on the
same level as the RAPR root directory.  Modify PROTOLIB, MGEN_COMMON,
and MGEN_BUILD in Makefile.common to change the default location.

By default, the sprng distribution is expected to be relative to the
RAPR root directory.  Modify SPRNG_INC and SPRNG_LIB in Makefile.linux
to change the sprng header and library locations.

Feel free to email with questions and comments.

John Schlorff <schlorff@itd.nrl.navy.mil>

