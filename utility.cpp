#include "utility.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

static void setAddrinfo(addrinfo *addr, bool passive) {
  memset(addr, 0, sizeof(addrinfo));
  addr->ai_family = AF_INET; // IPv4
  addr->ai_socktype = SOCK_STREAM;
  addr->ai_protocol = IPPROTO_TCP;
  addr->ai_flags |= AI_PASSIVE & passive;
}

void _getaddrinfo(const char *node, const char *service, addrinfo *hints,
                  addrinfo **res, bool passive) {
  setAddrinfo(hints, passive);
  int err = getaddrinfo(node, service, hints, res);
  if (err == EAI_SYSTEM) {
    syserr("getaddrinfo: %s", gai_strerror(err));
  } else if (err != 0) {
    syserr("getaddrinfo: %s", gai_strerror(err));
  }
}

Socket _socket(int domain, int type, int protocol) {
  Socket result = socket(domain, type, protocol);
  if (result < 0) {
    syserr("socket");
  }
  return result;
}

void addEpollEvent(Epoll efd, Socket sock) {
  epoll_event event;
  event.data.fd = sock;
  event.events = EPOLLIN | EPOLLET;
  if (epoll_ctl(efd, EPOLL_CTL_ADD, sock, &event) == -1) {
    syserr("epoll_ctl");
  }
}

void _connect(Socket sock, const sockaddr *addr, socklen_t addrlen) {
  int err = connect(sock, addr, addrlen);
  if (err < 0) {
    syserr("connect");
  }
}

void _write(Socket sock, const void *buf, size_t count) {
  size_t err = write(sock, buf, count);
  if (err != count) {
    syserr("partial / failed write");
  }
}

Epoll _epoll_create() {
  Epoll efd = epoll_create1(0);
  if (efd == -1) {
    syserr("epoll_create");
  }
  return efd;
}

void makeSocketNonBlocking(Socket sock) {
  int flags, s;

  flags = fcntl(sock, F_GETFL, 0);
  if (flags == -1) {
    syserr("fcntl");
  }

  flags |= O_NONBLOCK;
  s = fcntl(sock, F_SETFL, flags);
  if (s == -1) {
    syserr("fcntl");
  }
}

static ssize_t _read(Socket sock, void *buffer, size_t maxCount) {
  ssize_t count = read(sock, buffer, maxCount);
  if (count == -1 && errno != EAGAIN) {
    syserr("read");
  }
  return count;
}

std::string receiveShoutcast(Socket from, bool) {
  static char buffer[BUFFER_LEN];
  std::string result = INVALID_DATA;
  while (true) {
    ssize_t count = _read(from, buffer, BUFFER_LEN);
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

void sendShoutcastHeader(Socket sock) {
  static const char *header = "GET / HTTP/1.0\r\nAccept: */*\r\nIcy-MetaData: 0\r\nConnection: close\r\n\r\n";
  _write(sock, header, strlen(header));
}