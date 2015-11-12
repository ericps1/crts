FLAGS = -I include -Wall -fPIC -g
LIBS = lib/TUN.o lib/ECR.o -lliquid -luhd -lpthread -lm -lc -lconfig

#EDIT START FLAG
CEs = src/CE.cpp cognitive_engines/CE_Template.cpp cognitive_engines/CE_Subcarrier_Alloc.cpp cognitive_engines/CE_Mod_Adaptation.cpp cognitive_engines/CE_Two_Channel_DSA_Spectrum_Sensing.cpp cognitive_engines/CE_Two_Channel_DSA_PU.cpp cognitive_engines/CE_FEC_Adaptation.cpp cognitive_engines/CE_Two_Channel_DSA_Link_Reliability.cpp cognitive_engines/CE_uhd_msg.cpp
#EDIT END FLAG

all: lib/TUN.o lib/read_configs.o config_CEs lib/ECR.o logs/logs2python logs/logs2octave CRTS_CR lib/interferer.o CRTS_interferer CRTS_controller

lib/TUN.o: src/TUN.cpp
	g++ $(FLAGS) -c -o lib/TUN.o src/TUN.cpp

lib/read_configs.o: src/read_configs.cpp
	g++ $(FLAGS) -c -o lib/read_configs.o src/read_configs.cpp

config_CEs: src/config_CEs.cpp
	g++ $(FLAGS) -o config_CEs src/config_CEs.cpp lib/read_configs.o -lconfig -lliquid

lib/ECR.o: include/ECR.hpp src/ECR.cpp 
	g++ $(FLAGS) -c -o lib/ECR.o src/ECR.cpp

CRTS_CR: include/ECR.hpp src/TUN.cpp src/ECR.cpp src/CRTS_CR.cpp  $(CEs)
	g++ $(FLAGS) -o CRTS_CR src/CRTS_CR.cpp src/read_configs.cpp src/timer.cc $(CEs) $(LIBS)

lib/interferer.o: src/interferer.cpp 
	g++ $(FLAGS) -c -o lib/interferer.o src/interferer.cpp

CRTS_interferer: src/CRTS_interferer.cpp 
	g++ $(FLAGS) -o CRTS_interferer src/CRTS_interferer.cpp src/timer.cc lib/interferer.o lib/read_configs.o -luhd -lc -lconfig -lliquid

CRTS_controller: include/node_parameters.hpp src/CRTS_controller.cpp src/read_configs.cpp
	g++ $(FLAGS) -o CRTS_controller src/CRTS_controller.cpp lib/read_configs.o -lconfig -lliquid

logs/logs2octave: src/logs2octave.cpp
	g++ $(FLAGS) -o logs/logs2octave src/logs2octave.cpp -luhd

logs/logs2python: src/logs2python.cpp
	g++ $(FLAGS) -o logs/logs2python src/logs2python.cpp -luhd

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
	$(MAKE) -C doc clean

    

