# CAB403Project
CAB403 Major Project - Car park
all: simulator manager firealarm

# Compile systems
simulator:
	gcc -Wall -pedantic simulator.c -o simulator -lrt -pthread

manager:
	gcc -Wall -pedantic manager.c -o manager -lrt -pthread

firealarm:
	gcc -Wall -pedantic fire_alarm_new.c -o firealarm -lrt -pthread

# Run Systems
type ./simulator to run the simulator type ./manager to run the manager type ./firealarm to run the firealarm



