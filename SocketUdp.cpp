#include "SocketUdp.hpp"

void SocketUdp::connectServer(unsigned &port) {
  if (!checkValid()) {
    return;
  }
  Socket_(AF_INET, SOCK_DGRAM, 0);

  memset(&my_address, 0, sizeof(my_address));

  my_address.sin_family = AF_INET; // IPv4
  my_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
  my_address.sin_port = htons(port); // default port for receiving is PORT_NUM

  Bind(reinterpret_cast<sockaddr *>(&my_address), sizeof(my_address));

  makeNonBlocking();
}

void SocketUdp::connectClient(const std::string &host, const unsigned port) {
  if (!checkValid()) {
    return;
  }
  addrinfo hints;
  addrinfo *addr_result;

  // 'converting' host/port in string to struct addrinfo
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  if (!Utility::_getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &addr_result)) {
    makeInvalid("_getaddrinfo");
    return;
  }

  other_address.sin_family = AF_INET; // IPv4
  other_address.sin_addr.s_addr =
      reinterpret_cast<sockaddr_in *>(addr_result->ai_addr)->sin_addr.s_addr; // address IP
  other_address.sin_port = htons(port);

  freeaddrinfo(addr_result);

  Socket_(PF_INET, SOCK_DGRAM, 0);

  makeNonBlocking();
}

SocketUdp::SocketUdp() :
    Socket() {

}

SocketUdp::SocketUdp(int d) :
    Socket(d) {

}

void SocketUdp::Write(const void *data, size_t size) {
  if (!checkValid()) {
    return;
  }
  size_t len = sendto(sock, data, size, 0,
                      reinterpret_cast<sockaddr *>(&other_address),
                      static_cast<socklen_t>(sizeof(other_address)));
  if (len != size) {
    makeInvalid("sendto");
  }
}

ssize_t SocketUdp::Read(void *buffer, size_t maxCount) {
  if (!checkValid()) {
    return 0;
  }
  static socklen_t rcva_len = sizeof(other_address);
  ssize_t count = recvfrom(sock, buffer, maxCount, 0,
                           reinterpret_cast<sockaddr *>(&other_address), &rcva_len);
  if (count == -1 && errno != EAGAIN) {
    makeInvalid("recvfrom");
  }
  return count;
}