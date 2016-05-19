#include "Socket.h"
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

Socket::Socket(int domain, int type, int protocol) {
  sock = socket(domain, type, protocol);
  if (sock < 0) {
    Utility::syserr("socket");
  }
}

void Socket::Connect(const sockaddr *addr, socklen_t addrlen) {
  int err = connect(sock, addr, addrlen);
  if (err < 0) {
    Utility::syserr("connect");
  }
}

void Socket::Write(const void *buf, size_t count) {
  size_t err = write(sock, buf, count);
  if (err != count) {
    Utility::syserr("partial / failed write");
  }
}

int Socket::get() const {
  return sock;
}

void Socket::makeNonBlocking() {
  int flags, s;

  flags = fcntl(sock, F_GETFL, 0);
  if (flags == -1) {
    Utility::syserr("fcntl");
  }

  flags |= O_NONBLOCK;
  s = fcntl(sock, F_SETFL, flags);
  if (s == -1) {
    Utility::syserr("fcntl");
  }
}

ssize_t Socket::Read(void *buffer, size_t maxCount) {
  ssize_t count = read(sock, buffer, maxCount);
  if (count == -1 && errno != EAGAIN) {
    Utility::syserr("read");
  }
  return count;
}