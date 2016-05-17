FLAGS = -I include -Wall -fPIC -std=c++11 -g
LIBS = lib/TUN.o lib/ECR.o -lliquid -luhd -lpthread -lm -lc -lconfig

#EDIT CE OBJECT LIST START FLAG
CEs = src/CE.cpp cognitive_engines/CE_Template.cpp cognitive_engines/CE_Subcarrier_Alloc.cpp cognitive_engines/CE_Mod_Adaptation.cpp cognitive_engines/CE_Network_Loading.cpp cognitive_engines/CE_Two_Channel_DSA_Spectrum_Sensing.cpp cognitive_engines/CE_Two_Channel_DSA_PU.cpp cognitive_engines/CE_FEC_Adaptation.cpp cognitive_engines/CE_Two_Channel_DSA_Link_Reliability.cpp cognitive_engines/CE_Control_and_Feedback_Test.cpp cognitive_engines/CE_Simultaneous_RX_And_Sensing.cpp cognitive_engines/CE_Throughput_Test.cpp
#EDIT CE OBJECT LIST END FLAG

#EDIT SC START FLAG
SCs = src/SC.cpp scenario_controllers/SC_BER_Sweep.cpp scenario_controllers/SC_Control_and_Feedback_Test.cpp scenario_controllers/SC_CORNET_3D.cpp scenario_controllers/SC_Network_Loading.cpp scenario_controllers/SC_Template.cpp
#EDIT SC END FLAG

all: lib/CRTS.o config_CEs config_SCs lib/TUN.o lib/timer.o lib/ECR.o lib/interferer.o logs/logs2python logs/logs2octave CRTS_interferer CRTS_CR CRTS_controller

lib/CRTS.o: include/CRTS.hpp src/CRTS.cpp
	g++ $(FLAGS) -c -o lib/CRTS.o src/CRTS.cpp

config_CEs: src/config_CEs.cpp
	g++ $(FLAGS) -o config_CEs src/config_CEs.cpp lib/CRTS.o -lconfig -lliquid

config_SCs: src/config_SCs.cpp
	g++ $(FLAGS) -o config_SCs src/config_SCs.cpp lib/CRTS.o -lconfig -lliquid

lib/TUN.o: include/TUN.hpp src/TUN.cpp
	g++ $(FLAGS) -c -o lib/TUN.o src/TUN.cpp

lib/timer.o: include/timer.h src/timer.cc
	g++ $(FLAGS) -c -o lib/timer.o src/timer.cc

lib/ECR.o: include/ECR.hpp src/ECR.cpp 
	g++ $(FLAGS) -c -o lib/ECR.o src/ECR.cpp

lib/interferer.o: include/interferer.hpp src/interferer.cpp 
	g++ $(FLAGS) -c -o lib/interferer.o src/interferer.cpp

logs/logs2octave: src/logs2octave.cpp
	g++ $(FLAGS) -o logs/logs2octave src/logs2octave.cpp -luhd

logs/logs2python: src/logs2python.cpp
	g++ $(FLAGS) -o logs/logs2python src/logs2python.cpp -luhd

CRTS_interferer: include/interferer.hpp include/CRTS.hpp src/CRTS_interferer.cpp src/interferer.cpp src/CRTS.cpp 
	g++ $(FLAGS) -o CRTS_interferer src/CRTS_interferer.cpp lib/CRTS.o lib/interferer.o lib/timer.o -luhd -lc -lconfig -lliquid -lpthread

CRTS_CR: include/ECR.hpp src/TUN.cpp src/ECR.cpp src/CRTS_CR.cpp  $(CEs)
	g++ $(FLAGS) -o CRTS_CR src/CRTS_CR.cpp lib/CRTS.o lib/timer.o $(CEs) $(LIBS)

CRTS_controller: include/CRTS.hpp src/CRTS.cpp src/CRTS_controller.cpp $(SCs)
	g++ $(FLAGS) -o CRTS_controller src/CRTS_controller.cpp lib/CRTS.o lib/timer.o -lconfig -lliquid -lpthread $(SCs)

install:
	cp ./.crts_sudoers /etc/sudoers.d/crts # Filename must not have '_' or '.' in name.
	chmod 440 /etc/sudoers.d/crts

uninstall:
	rm -rf /etc/sudoers.d/crts

.PHONY: doc
doc:
	$(MAKE) -C doc all
cleandoc:
	$(MAKE) -C doc clean

clean:
	rm -rf lib/*.o
	rm -rf CRTS_CR
	rm -rf CRTS_interferer
	rm -rf CRTS_controller
	rm -rf logs/logs2octave
	rm -rf logs/logs2python
	rm -rf config_CEs
	rm -rf config_SCs
	$(MAKE) -C doc clean

    

