COMPILER=g++
CPPFLAGS=-Wall -Wunused -Wshadow -pedantic -O2 -std=c++11 -g

all: master player

player: player.cpp utility.o
	$(COMPILER) $(CPPFLAGS) -o player player.cpp utility.o

master: master.cpp utility.o
	$(COMPILER) $(CPPFLAGS) -o master master.cpp utility.o

utility.o: utility.h utility.cpp
	$(COMPILER) $(CPPFLAGS) -c utility.cpp

clean:
	rm -f master player *.o
