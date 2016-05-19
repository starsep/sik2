COMPILER=g++
CPPFLAGS=-Wall -Wunused -Wshadow -pedantic -O2 -std=c++11 -g#-DDEBUG
LIBSFLAGS=-lboost_regex
OFILES=Player.o Master.o Utility.o Socket.o Epoll.o
all: master player

player: player_main.cpp $(OFILES)
	$(COMPILER) $(CPPFLAGS) -o $@ $^ $(LIBSFLAGS)

master: master_main.cpp $(OFILES)
	$(COMPILER) $(CPPFLAGS) -o $@ $^ $(LIBSFLAGS)

Player.o: Player.h Player.cpp
	$(COMPILER) $(CPPFLAGS) -c Player.cpp $(LIBSFLAGS)

Master.o: Master.h Master.cpp
	$(COMPILER) $(CPPFLAGS) -c Master.cpp $(LIBSFLAGS)

Utility.o: Utility.h Utility.cpp
	$(COMPILER) $(CPPFLAGS) -c Utility.cpp $(LIBSFLAGS)

Socket.o: Socket.h Socket.cpp
	$(COMPILER) $(CPPFLAGS) -c Socket.cpp $(LIBSFLAGS)

Epoll.o: Epoll.h Epoll.cpp
	$(COMPILER) $(CPPFLAGS) -c Epoll.cpp $(LIBSFLAGS)

clean:
	rm -f master player *.o
