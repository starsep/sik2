COMPILER=g++
CPPFLAGS=-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wswitch-default -Wundef -Werror -Wno-unused -O2 -std=c++11 -g#-DDEBUG
LIBSFLAGS=-lboost_regex -lssh -lpthread
OFILES=Player.o Master.o Utility.o Socket.o Epoll.o TelnetSession.o
all: master player

.PHONY: clean analyze

player: player_main.cpp $(OFILES)
	$(COMPILER) $(CPPFLAGS) -o $@ $^ $(LIBSFLAGS)

master: master_main.cpp $(OFILES)
	$(COMPILER) $(CPPFLAGS) -o $@ $^ $(LIBSFLAGS)

$(OFILES): %.o: %.cpp %.hpp
	$(COMPILER) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f master player *.o *.gch *.plist

analyze:
	clang++ -std=c++11 --analyze *.cpp *.hpp
