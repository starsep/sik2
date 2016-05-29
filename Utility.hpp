#ifndef SIK2_UTILITY_HPP
#define SIK2_UTILITY_HPP

#include <boost/regex.hpp>
#include <cctype>
#include <iostream>
#include <sstream>
#include <ostream>
#include <algorithm>
#include <vector>
#include <map>
#include <tuple>
#include <iomanip>
#include <exception>

#include <iostream>
#include <tuple>
#include <fstream>
#include <sstream>

#include <netdb.h>
#include <sys/epoll.h>

#include "Socket.hpp"
#include "Epoll.hpp"

using File = int;

class ClosedConnectionException {
};

class BadNetworkDataException {
};

class TooMuchDataException {
};

const unsigned INVALID_PORT = 0;
const unsigned MIN_PORT = 1;
const unsigned MAX_PORT = (1 << 16) - 1;
const std::string stdOut = "-";
const std::string stdOutFilename = "/dev/stdout";
const std::string YES = "yes";
const std::string NO = "no";
const std::string INVALID_DATA = "";

const std::string PLAY = "PLAY";
const std::string PAUSE = "PAUSE";
const std::string TITLE = "TITLE";
const std::string QUIT = "QUIT";
const std::string AT = "AT";
const std::string START = "START";

const std::string OK = "OK\n";

const int HEADER_EVENTS = 2;

const int BUFFER_LEN = 10000;

const int INF = -1;
const int MAX_TIME_S = 4;
const int MAX_TIME_MS = MAX_TIME_S * 1000;
const int MAX_SOCKETS_PLAYER = 2;
const int MAX_EVENTS_TELNET = 10;
const int MAX_HEADER_SHOUTCAST_SIZE = 10000000; //10MB

const File stdOutFile = 1;

enum class ExitCode {
  Ok = 0,
  InvalidArguments = 1,
  SystemError = 1,
  BadData = 1,
};

class Utility {
private:
  static void setAddrinfo(addrinfo *, bool);

public:
  static void _exit(ExitCode);

  static void info(const char *fmt, ...);

  static bool _getaddrinfo(const char *, const char *, addrinfo *, addrinfo **,
                           bool = false);

  static unsigned getPort(const char *);

  static bool equalExceptWhitespaceOnEnd(const std::string &msg, const std::string &pat);

  static bool startsWith(const std::string &s, const std::string &pref);

  static int currentMinutes();
};

#endif // SIK2_UTILITY_HPP