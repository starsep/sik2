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

  void Connect(const sockaddr *, socklen_t);

  virtual ssize_t Read(void *, size_t) = 0;

  virtual void Write(const void *, size_t) = 0;

public:

  Socket(int);

  Socket();

  Socket(const Socket &);

  void Send(const std::string &s);

  int get() const;

  void makeNonBlocking();

  std::string receive(int max_len = -1);

  virtual void connectServer(unsigned &port) = 0;

  virtual void connectClient(const std::string &host, const unsigned port) = 0;

  ~Socket();
};

#endif //SIK2_SOCKET_HPP
