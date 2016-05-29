#include "Utility.hpp"

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <csignal>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <ctime>


void Utility::info(const char *fmt, ...) {
  va_list fmt_args;

  fprintf(stderr, "ERROR: ");
  va_start(fmt_args, fmt);
  vfprintf(stderr, fmt, fmt_args);
  va_end(fmt_args);
  fprintf(stderr, " (%d; %s)\n", errno, strerror(errno));
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

void Utility::_exit(ExitCode code) { exit(static_cast<int>(code)); }

void Utility::setAddrinfo(addrinfo *addr, bool passive) {
  memset(addr, 0, sizeof(addrinfo));
  addr->ai_family = AF_INET; // IPv4
  addr->ai_socktype = SOCK_STREAM;
  addr->ai_protocol = IPPROTO_TCP;
  addr->ai_flags |= AI_PASSIVE & passive;
}

bool Utility::_getaddrinfo(const char *node, const char *service, addrinfo *hints,
                           addrinfo **res, bool passive) {
  setAddrinfo(hints, passive);
  int err = getaddrinfo(node, service, hints, res);
  if (err == EAI_SYSTEM || err != 0) {
    Utility::info("getaddrinfo: %s", gai_strerror(err));
    return false;
  }
  return true;
}

bool Utility::equalExceptWhitespaceOnEnd(const std::string &msg, const std::string &pat) {
  boost::smatch result;
  const boost::regex pattern(pat + R"(\s*)");
  try {
    return boost::regex_match(msg, result, pattern);
  }
  catch (...) {
    return false;
  }
}

bool Utility::startsWith(const std::string &s, const std::string &pref) {
  return s.substr(0, pref.size()) == pref;
}

int Utility::currentMinutes() {
  char buf[3];
  time_t now = time(nullptr);
  tm tstruct;
  tstruct = *localtime(&now);

  strftime(buf, sizeof(buf), "%H", &tstruct);
  buf[2] = '\0';
  int result = 60 * std::stoi(buf);

  strftime(buf, sizeof(buf), "%M", &tstruct);
  buf[2] = '\0';
  result += std::stoi(buf);

  return result;
}