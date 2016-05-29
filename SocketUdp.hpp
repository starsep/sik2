#ifndef SIK2_SOCKETUDP_HPP
#define SIK2_SOCKETUDP_HPP

#include "Socket.hpp"

class SocketUdp : public Socket {
private:
protected:
public:
  SocketUdp();

  SocketUdp(int);

  virtual void connectServer(unsigned &port);

  virtual void connectClient(const std::string &host, const unsigned port);
};

#endif //SIK2_SOCKETUDP_HPP
