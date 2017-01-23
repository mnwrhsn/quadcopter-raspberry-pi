CC=g++

CFLAGS=-std=c++0x
CFLAGS_Raspberry= $(CFLAGS) -lwiringPi

LDFLAGS=-lpthread 

#--------------------------SUR RASBPERRY---------------------------#

#rajouté  -lwiringPi   a la suite de  LDFLAGS_Raspberry= $(LDFLAGS)
LDFLAGS_Raspberry= $(LDFLAGS)
#-lwiringPi 

#------------------------------------------------------------------#

LDFLAGS_ClientRemote= $(LDFLAGS) -lSDL -lSDLmain

SRC=src/concurrent.c src/network.c

SRC_drone = $(SRC) src/controldeVol.c src/motors.c src/serv.c

SRC_client = $(SRC)  src/client.c src/Manette/controller.c src/Manette/manette.c 

OBJdroneMain= $(SRC_drone:.c=.o)

OBJclientRemote= $(SRC_client:.c=.o) 

EXEC = clientRemoteMain droneMain

all: 
	
	echo 'type : make drone   |or type : make client'

#all: $(EXEC)

drone:droneMain

client:clientRemoteMain

src/droneMain.o: src/droneMain.c
	$(CC) $(CFLAGS_Raspberry) -o $@ -c $< 

droneMain: src/droneMain.o $(OBJdroneMain)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS_Raspberry)

src/clientRemoteMain.o: src/clientRemoteMain.c
	$(CC) $(CFLAGS) -o $@ -c $< 

clientRemoteMain: $(OBJclientRemote) src/clientRemoteMain.o 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS_ClientRemote)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf object/*.o
	find . -name '*.o' -delete

mrproper: clean
	rm -rf $(EXEC)