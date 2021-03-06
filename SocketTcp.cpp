#include "SocketTcp.hpp"

SocketTcp SocketTcp::Accept(sockaddr *addr, socklen_t *addrlen) {
  SocketTcp result{};
  result.sock = accept(sock, addr, addrlen);
  result.valid = true;
  return result;
}

void SocketTcp::sendShoutcastHeader(const std::string &path, bool md) {
  if (!checkValid("SocketTcp::sendShoutcastHeader")) {
    return;
  }
  std::string request;
  request += "GET " + path + " HTTP/1.0\r\n"; // http header
  request += "Accept: */*\r\n";               // accept header
  if (md) {
    request += "Icy-MetaData: 1\r\n"; // whether we want metadata
  }
  request += "Connection: close\r\n"; // connection header
  request += "\r\n";                  // empty line
  Send(request);
}

void SocketTcp::connectServer(unsigned &port) {
  if (!checkValid("SocketTcp::connectServer")) {
    return;
  }
  addrinfo hints;
  addrinfo *addr_result;

  if (!Utility::_getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &addr_result, true)) {
    makeInvalid("_getaddrinfo");
    return;
  }

  addrinfo *rp;
  for (rp = addr_result; rp != nullptr; rp = rp->ai_next) {
    Socket_(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

    if (Bind(rp->ai_addr, rp->ai_addrlen)) {
      break;
    }

    Close();
  }

  if (rp == nullptr) {
    makeInvalid("Could not bind");
  }

  freeaddrinfo(addr_result);

  makeNonBlocking();
  Listen();

  if (port == 0) {
    sockaddr_in serv_addr;
    socklen_t len = sizeof(serv_addr);
    if (getsockname(get(), reinterpret_cast<sockaddr *>(&serv_addr), &len) == -1) {
      makeInvalid("getsockname");
    }
    port = ntohs(serv_addr.sin_port);
  }
}

void SocketTcp::connectClient(const std::string &host, const unsigned port) {
  if (!checkValid("SocketTcp::connectClient")) {
    return;
  }
  addrinfo hints;
  addrinfo *addr_result;

  if (!Utility::_getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &addr_result)) {
    makeInvalid("_getaddrinfo");
    return;
  }

  Socket_(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);

  setMaxTimeout(MAX_TIME_S);

  Connect(addr_result->ai_addr, addr_result->ai_addrlen);
  freeaddrinfo(addr_result);
  makeNonBlocking();
}

SocketTcp::SocketTcp() :
    Socket() {

}

SocketTcp::SocketTcp(int d) :
    Socket(d) {
}

void SocketTcp::Write(const void *buf, size_t count) {
  if (!checkValid("SocketTcp::Write")) {
    return;
  }
  size_t err = write(sock, buf, count);
  if (err != count) {
    makeInvalid("partial / failed write");
  }
}

ssize_t SocketTcp::Read(void *buffer, size_t maxCount) {
  if (!checkValid("SocketTcp::Read")) {
    return 0;
  }
  ssize_t count = read(sock, buffer, maxCount);
  if (count == -1 && errno != EAGAIN) {
    makeInvalid("read");
  }
  return count;
}

void SocketTcp::setMaxTimeout(int seconds) {
  if (!checkValid("SocketTcp::setMaxTimeout")) {
    return;
  }
  struct timeval timeout;
  timeout.tv_sec = seconds;
  timeout.tv_usec = 0;

  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                 sizeof(timeout)) < 0) {
    makeInvalid("setsockopt failed\n");
    return;
  }

  if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                 sizeof(timeout)) < 0) {
    makeInvalid("setsockopt failed\n");
  }
}
