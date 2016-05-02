#include "utility.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <csignal>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

void syserr(const char *fmt, ...) {
  va_list fmt_args;

  fprintf(stderr, "ERROR: ");
  va_start(fmt_args, fmt);
  vfprintf(stderr, fmt, fmt_args);
  va_end(fmt_args);
  fprintf(stderr, " (%d; %s)\n", errno, strerror(errno));
  _exit(ExitCode::SystemError);
}

unsigned getPort(const char *cPort) {
  std::string sPort(cPort);
  unsigned long port;
  try {
    port = std::stoul(sPort);
  } catch (...) {
    return INVALID_PORT;
  }
  return (port < MIN_PORT || port > MAX_PORT) ? INVALID_PORT : port;
}

unsigned getFreePort() {
  return INVALID_PORT; // TODO
}

void _exit(ExitCode code) { exit(static_cast<int>(code)); }

File _open(const char *filename, int flags) {
  File result = open(filename, flags);
  if (result == -1) {
    syserr("open");
  }
  return result;
}