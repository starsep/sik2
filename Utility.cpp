#include "Utility.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <csignal>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

void Utility::Utility::syserr(const char *fmt, ...) {
  va_list fmt_args;

  fprintf(stderr, "ERROR: ");
  va_start(fmt_args, fmt);
  vfprintf(stderr, fmt, fmt_args);
  va_end(fmt_args);
  fprintf(stderr, " (%d; %s)\n", errno, strerror(errno));
  Utility::_exit(ExitCode::SystemError);
}

unsigned Utility::getPort(const char *cPort) {
  std::string sPort(cPort);
  unsigned long port;
  try {
    port = std::stoul(sPort);
  } catch (...) {
    return INVALID_PORT;
  }
  return (port < MIN_PORT || port > MAX_PORT) ? INVALID_PORT : port;
}

unsigned Utility::getFreePort() {
  return INVALID_PORT; // TODO
}

void Utility::_exit(ExitCode code) { exit(static_cast<int>(code)); }

static void setAddrinfo(addrinfo *addr, bool passive) {
  memset(addr, 0, sizeof(addrinfo));
  addr->ai_family = AF_INET; // IPv4
  addr->ai_socktype = SOCK_STREAM;
  addr->ai_protocol = IPPROTO_TCP;
  addr->ai_flags |= AI_PASSIVE & passive;
}

void Utility::_getaddrinfo(const char *node, const char *service, addrinfo *hints,
                  addrinfo **res, bool passive) {
  setAddrinfo(hints, passive);
  int err = getaddrinfo(node, service, hints, res);
  if (err == EAI_SYSTEM) {
    Utility::syserr("getaddrinfo: %s", gai_strerror(err));
  } else if (err != 0) {
    Utility::syserr("getaddrinfo: %s", gai_strerror(err));
  }
}

std::string Utility::receiveShoutcast(Socket from, bool) {
  static char buffer[BUFFER_LEN];
  std::string result = INVALID_DATA;
  while (true) {
    ssize_t count = from.Read(buffer, BUFFER_LEN);
    if (count == -1 && errno == EAGAIN) {
      break;
    }
    if (count == 0) {
      throw ClosedConnectionException();
    }
    result += std::string(buffer, count);
  }
  return result;
}

void Utility::sendShoutcastHeader(Socket sock, const std::string &path, bool md) {
  std::string request;
  request += "GET " + path + " HTTP/1.0\r\n"; // http header
  request += "Accept: */*\r\n";               // accept header
  request += std::string("Icy-MetaData: ") + (md ? "1" : "0") +
             "\r\n";                  // whether we want metadata
  request += "Connection: close\r\n"; // connection header
  request += "\r\n";                  // empty line
  sock.Write(request.c_str(), request.size());
}