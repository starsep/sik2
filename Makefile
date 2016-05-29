COMPILER=g++
CPPFLAGS=-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Werror -Wsign-promo -Wstrict-null-sentinel -Wswitch-default -Wundef -Wno-unused -O2 -std=c++11 -g#-DDEBUG
LIBSFLAGS=-lboost_regex -lpthread -lssh2
BINARIES=player master
OFILES=Player.o Master.o Utility.o Socket.o Epoll.o TelnetSession.o PlayerExecution.o

all: $(BINARIES)

.PHONY: clean analyze

$(BINARIES): %: %_main.cpp $(OFILES)
	$(COMPILER) $(CPPFLAGS) -o $@ $^ $(LIBSFLAGS)

$(OFILES): %.o: %.cpp %.hpp
	$(COMPILER) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(BINARIES) *.o *.gch *.plist

analyze:
	clang++ -std=c++11 --analyze *.cpp *.hpp
