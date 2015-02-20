FLAGS = -I include -Wall -fPIC -g
LIBS = lib/TUN.o lib/CR.o -lliquid -luhd -lpthread -lm -lc -lconfig

#EDIT START FLAG
CEs = src/CE.cpp cognitive_engines/CE_Example_1.cpp cognitive_engines/CE_Example_2.cpp
#EDIT END FLAG

all: TUN TUN_test read_configs config_CEs CR CRTS_UE interferer CRTS_interferer CRTS_controller post_process_logs
#CRTS_test

TUN: src/TUN.cpp
	g++ $(FLAGS) -c -o lib/TUN.o src/TUN.cpp

TUN_test: test/TUN.cpp
	g++ $(FLAGS) -o TUN test/TUN.cpp lib/TUN.o -lpthread

read_configs: src/read_configs.cpp
	g++ $(FLAGS) -c -o lib/read_configs.o src/read_configs.cpp

config_CEs: src/config_CEs.cpp
	g++ $(FLAGS) -o config_CEs src/config_CEs.cpp lib/read_configs.o -lconfig

CR: include/CR.hpp src/CR.cpp
	g++ $(FLAGS) -c -o lib/CR.o src/CR.cpp

#CR_test: include/CR.hpp src/TUN.cpp src/CR.cpp test/CR_test.cpp
#	g++ $(FLAGS) -o CR_test test/CR_test.cpp lib/TUN.o lib/CR.o -lliquid -luhd -lpthread -lm -lc 

CRTS_UE: include/CR.hpp src/TUN.cpp src/CR.cpp src/CRTS_UE.cpp
	g++ $(FLAGS) -o CRTS_UE src/CRTS_UE.cpp src/read_configs.cpp lib/TUN.o lib/CR.o -lliquid -luhd -lpthread -lm -lc -lconfig $(CEs)

CRTS_AP: src/CRTS_AP.cpp
	g++ $(FLAGS) -c -o CRTS_AP src/CRTS_AP.cpp lib/CR.o lib/CR.o

interferer: src/interferer.cpp
	g++ $(FLAGS) -c -o lib/interferer.o src/interferer.cpp

CRTS_interferer: src/CRTS_interferer.cpp
	g++ $(FLAGS) -o CRTS_interferer src/CRTS_interferer.cpp lib/interferer.o lib/read_configs.o -luhd -lc -lconfig

CRTS_controller: include/node_parameters.hpp src/CRTS_controller.cpp src/read_configs.cpp
	g++ $(FLAGS) -o CRTS_controller src/CRTS_controller.cpp lib/read_configs.o -lconfig

post_process_logs: src/post_process_logs.cpp
	g++ $(FLAGS) -o post_process_logs src/post_process_logs.cpp -luhd
