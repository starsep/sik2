#include "SocketUdp.hpp"

void SocketUdp::connectServer(unsigned &port) {
  Socket_(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  sockaddr_in si_me;
  memset(&si_me, 0, sizeof(si_me));

  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(port);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);

  Bind(reinterpret_cast<sockaddr *>(&si_me), sizeof(si_me));

}

void SocketUdp::connectClient(const std::string &host, const unsigned port) {
  unsigned p = port;
  connectServer(p);
  host.size(); // TODO: remove
  //TODO?
}

SocketUdp::SocketUdp() :
    Socket() {

}

SocketUdp::SocketUdp(int d) :
    Socket(d) {

}