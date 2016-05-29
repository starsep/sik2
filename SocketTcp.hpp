#ifndef SIK2_SOCKETTCP_HPP
#define SIK2_SOCKETTCP_HPP

#include "Socket.hpp"

class SocketTcp : public Socket {
private:
protected:
public:
  SocketTcp();
  SocketTcp(int);

  SocketTcp Accept(sockaddr *, socklen_t *);

  void sendShoutcastHeader(const std::string &, bool);

  virtual void connectServer(unsigned &port);

  virtual void connectClient(const std::string &host, const unsigned port);
};

#endif //SIK2_SOCKETTCP_HPP
