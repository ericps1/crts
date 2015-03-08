FLAGS = -I include -Wall -fPIC -g
LIBS = lib/TUN.o lib/CR.o -lliquid -luhd -lpthread -lm -lc -lconfig

#EDIT START FLAG
CEs = src/CE.cpp cognitive_engines/CE_Example_2.cpp cognitive_engines/CE_Example_1.cpp
#EDIT END FLAG

all: lib/TUN.o TUN lib/read_configs.o config_CEs lib/CR.o CRTS_UE lib/interferer.o CRTS_interferer CRTS_controller post_process_logs
#CRTS_test

lib/TUN.o: src/TUN.cpp
	g++ $(FLAGS) -c -o lib/TUN.o src/TUN.cpp

TUN: test/TUN.cpp
	g++ $(FLAGS) -o TUN test/TUN.cpp lib/TUN.o -lpthread

lib/read_configs.o: src/read_configs.cpp
	g++ $(FLAGS) -c -o lib/read_configs.o src/read_configs.cpp

config_CEs: src/config_CEs.cpp
	g++ $(FLAGS) -o config_CEs src/config_CEs.cpp lib/read_configs.o -lconfig

lib/CR.o: include/CR.hpp src/CR.cpp
	g++ $(FLAGS) -c -o lib/CR.o src/CR.cpp

#CR_test: include/CR.hpp src/TUN.cpp src/CR.cpp test/CR_test.cpp
#	g++ $(FLAGS) -o CR_test test/CR_test.cpp lib/TUN.o lib/CR.o -lliquid -luhd -lpthread -lm -lc 

CRTS_UE: include/CR.hpp src/TUN.cpp src/CR.cpp src/CRTS_UE.cpp
	g++ $(FLAGS) -o CRTS_UE src/CRTS_UE.cpp src/read_configs.cpp lib/TUN.o lib/CR.o -lliquid -luhd -lpthread -lm -lc -lconfig $(CEs)

CRTS_AP: src/CRTS_AP.cpp
	g++ $(FLAGS) -c -o CRTS_AP src/CRTS_AP.cpp lib/CR.o lib/CR.o

lib/interferer.o: src/interferer.cpp
	g++ $(FLAGS) -c -o lib/interferer.o src/interferer.cpp

CRTS_interferer: src/CRTS_interferer.cpp
	g++ $(FLAGS) -o CRTS_interferer src/CRTS_interferer.cpp lib/interferer.o lib/read_configs.o -luhd -lc -lconfig -lliquid

CRTS_controller: include/node_parameters.hpp src/CRTS_controller.cpp src/read_configs.cpp
	g++ $(FLAGS) -o CRTS_controller src/CRTS_controller.cpp lib/read_configs.o -lconfig

post_process_logs: src/post_process_logs.cpp
	g++ $(FLAGS) -o logs/post_process_logs src/post_process_logs.cpp -luhd

clean:
	rm -rf lib/*.o
	rm -rf TUN
	rm -rf CRTS_UE
	rm -rf CRTS_AP
	rm -rf CRTS_interferer
	rm -rf CRTS_controller
	rm -rf post_process_logs
    
