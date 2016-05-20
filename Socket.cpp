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

Socket::Socket(int _sock) :
    sock(_sock) {
}

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

std::string Socket::receiveShoutcast(bool) {
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
  }
  return result;
}

void Socket::sendShoutcastHeader(const std::string &path, bool md) {
  std::string request;
  request += "GET " + path + " HTTP/1.0\r\n"; // http header
  request += "Accept: */*\r\n";               // accept header
  request += std::string("Icy-MetaData: ") + (md ? "1" : "0") +
             "\r\n";                  // whether we want metadata
  request += "Connection: close\r\n"; // connection header
  request += "\r\n";                  // empty line
  Write(request.c_str(), request.size());
}

Socket Socket::connectServer(unsigned &port) {
  addrinfo hints;
  addrinfo *addr_result;

  Utility::_getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &addr_result, true);

  Socket result(0);
  addrinfo *rp;
  for (rp = addr_result; rp != nullptr; rp = rp->ai_next) {
    result = Socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

    if (result.Bind(rp->ai_addr, rp->ai_addrlen)) {
      break;
    }

    result.Close();
  }

  if (rp == nullptr) {
    Utility::syserr("Could not bind\n");
  }

  freeaddrinfo(addr_result);

  result.makeNonBlocking();
  result.Listen();

  if (port == 0) {
    sockaddr_in serv_addr;
    socklen_t len = sizeof(serv_addr);
    if (getsockname(result.get(), (struct sockaddr *) &serv_addr, &len) == -1) {
      Utility::syserr("getsockname");
    }
    port = ntohs(serv_addr.sin_port);
  }
  return result;
}

Socket Socket::connectClient(const std::string &host, const unsigned port) {
  addrinfo hints;
  addrinfo *addr_result;

  Utility::_getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &addr_result);

  Socket result(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);

  result.Connect(addr_result->ai_addr, addr_result->ai_addrlen);
  freeaddrinfo(addr_result);
  return result;
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
