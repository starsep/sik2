COMPILER=g++
CPPFLAGS=-Wall -Wunused -Wshadow -pedantic -O2 -std=c++11 -g#-DDEBUG
LIBSFLAGS=-lboost_regex -lssh -lpthread
OFILES=Player.o Master.o Utility.o Socket.o Epoll.o
all: master player

player: player_main.cpp $(OFILES)
	$(COMPILER) $(CPPFLAGS) -o $@ $^ $(LIBSFLAGS)

master: master_main.cpp $(OFILES)
	$(COMPILER) $(CPPFLAGS) -o $@ $^ $(LIBSFLAGS)

%.o: %?.h %?.cpp
	$(COMPILER) $(CPPFLAGS) -c %?.cpp

clean:
	rm -f master player *.o
