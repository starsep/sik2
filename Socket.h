#ifndef SIK2_SOCKET_H
#define SIK2_SOCKET_H

#include <netdb.h>
#include <string>

class Socket {
private:
  int sock;

  void Close();

  bool Bind(const sockaddr *addr, socklen_t addrlen);

  void Listen();

public:
  Socket(int, int, int);

  Socket(int);

  Socket();

  void Connect(const sockaddr *, socklen_t);

  void Write(const void *, size_t);

  ssize_t Read(void *, size_t);

  int get() const;

  void makeNonBlocking();

  std::string receive();

  std::string receiveOnce();

  void sendShoutcastHeader(const std::string &, bool);

  void connectServer(unsigned &port);

  void connectClient(const std::string &host, const unsigned port);

  void connectUdp(const unsigned port);
};

#endif //SIK2_SOCKET_H
