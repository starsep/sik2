#ifndef SIK2_UTILITY_H
#define SIK2_UTILITY_H

#include <iostream>
#include <tuple>
#include <fstream>
#include <sstream>

#include <netdb.h>
#include <sys/epoll.h>

using File = int;
using Socket = int;
using Epoll = int;

class ClosedConnectionException {};
class BadNetworkDataException {};

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

void _exit(ExitCode);
void _getaddrinfo(const char *, const char *, addrinfo *, addrinfo **,
                  bool = false);
unsigned getPort(const char *);
unsigned getFreePort();
File _open(const char *, int);
Socket _socket(int, int, int);
void addEpollEvent(Epoll, Socket);
void _connect(Socket, const sockaddr *, socklen_t);
void _write(Socket, const void *, size_t);
Epoll _epoll_create();
void makeSocketNonBlocking(Socket);
std::string receiveShoutcast(Socket, bool);
void sendShoutcastHeader(Socket, const std::string &);

#endif // SIK2_UTILITY_H