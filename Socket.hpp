#ifndef SIK2_SOCKET_HPP
#define SIK2_SOCKET_HPP

#include "Utility.hpp"

#include <netdb.h>
#include <string>

class Socket {
protected:
  int sock;

  void Close();

  bool Bind(const sockaddr *addr, socklen_t addrlen);

  void Listen();

  void Socket_(int, int, int);

  void Write(const void *, size_t);

  ssize_t Read(void *, size_t);

public:

  Socket(int);

  Socket();

  Socket(const Socket &);

  void Connect(const sockaddr *, socklen_t);

  void Write(const std::string &s);

  int get() const;

  void makeNonBlocking();

  std::string receive(int max_len = -1);

  std::string receiveOnce();

  virtual void connectServer(unsigned &port) = 0;

  virtual void connectClient(const std::string &host, const unsigned port) = 0;
};

#endif //SIK2_SOCKET_HPP
