#include "Socket.hpp"
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

Socket::Socket() :
    sock(0) {

}

Socket::Socket(int _sock) :
    sock(_sock) {
}

void Socket::Socket_(int domain, int type, int protocol) {
  sock = socket(domain, type, protocol);
  if (sock < 0) {
    Utility::syserr("socket");
  }
}

Socket::Socket(const Socket &c) :
    sock(c.sock) {
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

void Socket::Write(const std::string &s) {
  Write(s.c_str(), s.size());
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

std::string Socket::receive(int max_len) {
  static char buffer[BUFFER_LEN];
  std::string result = INVALID_DATA;
  while (true) {
    ssize_t count = Read(buffer, BUFFER_LEN);
    if (count == -1 && errno == EAGAIN) {
      break;
    }
    if (count == 0) {
      throw ClosedConnectionException();
    }
    result += std::string(buffer, count);
    if (max_len != INF && static_cast<int>(result.size()) >= max_len) {
      throw TooMuchDataException();
    }
  }
  return result;
}

std::string Socket::receiveOnce() {
  static char buffer[BUFFER_LEN];
  ssize_t count = Read(buffer, BUFFER_LEN);
  if (count == 0) {
    throw ClosedConnectionException();
  }
  return std::string(buffer, count);
}

void Socket::Close() {
  int err = close(sock);
  if (err < 0) {
    Utility::syserr("close");
  }
}

void Socket::Listen() {
  if (listen(sock, SOMAXCONN)) {
    Utility::syserr("listen");
  }
}

bool Socket::Bind(const sockaddr *addr, socklen_t addrlen) {
  return bind(sock, addr, addrlen) == 0;
}