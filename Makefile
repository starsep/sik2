COMPILER=g++
CPPFLAGS=-Wall -Wunused -Wshadow -pedantic -O2 -std=c++11 -g#-DDEBUG
LIBSFLAGS=-lboost_regex
all: master player

player: player.cpp utility.o
	$(COMPILER) $(CPPFLAGS) -o player player.cpp utility.o $(LIBSFLAGS)

master: master.cpp utility.o
	$(COMPILER) $(CPPFLAGS) -o master master.cpp utility.o $(LIBSFLAGS)

utility.o: utility.h utility.cpp
	$(COMPILER) $(CPPFLAGS) -c utility.cpp $(LIBSFLAGS)

clean:
	rm -f master player *.o
