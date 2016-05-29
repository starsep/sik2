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
    sock(0),
    valid(true) {

}

Socket::Socket(int _sock) :
    sock(_sock),
    valid(true) {
}

void Socket::makeInvalid(std::string s) {
  valid = false;
  Utility::info(s.c_str());
}

bool Socket::checkValid() {
  return checkValid("");
}

bool Socket::checkValid(std::string pref) {
  if (!valid) {
    Utility::info((pref + " Socket is invalid").c_str());
    return false;
  }
  return true;
}

void Socket::Socket_(int domain, int type, int protocol) {
  if (!checkValid("Socket::Socket_")) {
    return;
  }
  sock = socket(domain, type, protocol);
  if (sock < 0) {
    Utility::info("socket");
    makeInvalid("Socket_");
  }
}

Socket::Socket(const Socket &c) :
    sock(c.sock),
    valid(c.valid) {
}

void Socket::Connect(const sockaddr *addr, socklen_t addrlen) {
  if (!checkValid("Socket::Connect")) {
    return;
  }
  int err = connect(sock, addr, addrlen);
  if (err < 0) {
    makeInvalid("connect");
  }
}

int Socket::get() const {
  return sock;
}

void Socket::makeNonBlocking() {
  if (!checkValid("Socket::makeNonBlocking")) {
    return;
  }
  int flags, s;

  flags = fcntl(sock, F_GETFL, 0);
  if (flags == -1) {
    return makeInvalid("fcntl");
  }

  flags |= O_NONBLOCK;
  s = fcntl(sock, F_SETFL, flags);
  if (s == -1) {
    return makeInvalid("fcntl");
  }
}

void Socket::Close() {
  if (!checkValid("Close")) {
    return;
  }
  int err = close(sock);
  if (err < 0) {
    makeInvalid("close");
  }
}

void Socket::Listen() {
  if (!checkValid("Socket::Listen")) {
    return;
  }
  if (listen(sock, SOMAXCONN)) {
    makeInvalid("listen");
  }
}

bool Socket::Bind(const sockaddr *addr, socklen_t addrlen) {
  if (!checkValid("Socket::Bind")) {
    return false;
  }
  int res = bind(sock, addr, addrlen);
  if (res != 0) {
    makeInvalid("bind");
  }
  return res == 0;
}

std::string Socket::receive(int max_len) {
  if (!checkValid("Socket::receive")) {
    return "";
  }
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

void Socket::Send(const std::string &s) {
  if (!checkValid("Socket::Send")) {
    return;
  }
  Write(s.c_str(), s.size());
}


std::string Socket::receiveOnce() {
  static char buffer[BUFFER_LEN];
  ssize_t count = Read(buffer, BUFFER_LEN);
  if (count == 0) {
    throw ClosedConnectionException();
  }
  return std::string(buffer, count);
}