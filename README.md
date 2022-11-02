# CAB403Project
CAB403 Major Project - Car park
all: simulator manager firealarm

simulator:
	gcc -Wall -pedantic simulator.c -o simulator -lrt -pthread

manager:
	gcc -Wall -pedantic manager.c -o manager -lrt -pthread

firealarm:
	gcc -Wall -pedantic fire_alarm_new.c -o firealarm -lrt -pthread

clean:
	rm firealarm
	rm manager
	rm simulator
