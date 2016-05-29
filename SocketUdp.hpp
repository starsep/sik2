#ifndef SIK2_SOCKETUDP_HPP
#define SIK2_SOCKETUDP_HPP

#include "Socket.hpp"

class SocketUdp : public Socket {
private:
  sockaddr_in my_address;
  sockaddr_in other_address;
protected:
  virtual ssize_t Read(void *, size_t);

  virtual void Write(const void *, size_t);

public:
  SocketUdp();

  SocketUdp(int);

  virtual void connectServer(unsigned &port);

  virtual void connectClient(const std::string &host, const unsigned port);
};

#endif //SIK2_SOCKETUDP_HPP
