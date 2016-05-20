#ifndef SIK2_UTILITY_H
#define SIK2_UTILITY_H

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

#include "Socket.h"
#include "Epoll.h"

using File = int;

class ClosedConnectionException {
};

class BadNetworkDataException {
};

const unsigned INVALID_PORT = 0;
const unsigned MIN_PORT = 1;
const unsigned MAX_PORT = (1 << 16) - 1;
const std::string stdOut = "-";
const std::string stdOutFilename = "/dev/stdout";
const std::string YES = "yes";
const std::string NO = "no";
const std::string INVALID_DATA = "";
const int HEADER_EVENTS = 2;

const int BUFFER_LEN = 10000;

const int MAX_TIME = 5000; //ms
const int MAX_SOCKETS_PLAYER = 2;

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

  static void syserr(const char *fmt, ...);

  static void _getaddrinfo(const char *, const char *, addrinfo *, addrinfo **,
                           bool = false);

  static unsigned getPort(const char *);

  static unsigned getFreePort();
};

#endif // SIK2_UTILITY_H