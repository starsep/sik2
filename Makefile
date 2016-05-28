COMPILER=g++
CPPFLAGS=-Wall -Wunused -Wshadow -pedantic -O2 -std=c++11 -g#-DDEBUG
LIBSFLAGS=-lboost_regex -lssh -lpthread
OFILES=Player.o Master.o Utility.o Socket.o Epoll.o TelnetSession.o
all: master player

player: player_main.cpp $(OFILES)
	$(COMPILER) $(CPPFLAGS) -o $@ $^ $(LIBSFLAGS)

master: master_main.cpp $(OFILES)
	$(COMPILER) $(CPPFLAGS) -o $@ $^ $(LIBSFLAGS)

$(OFILES): %.o: %.cpp
	$(COMPILER) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f master player *.o
