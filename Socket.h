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

  void Connect(const sockaddr *, socklen_t);

  void Write(const void *, size_t);

  ssize_t Read(void *, size_t);

  int get() const;

  void makeNonBlocking();

  std::string receiveShoutcast(bool);

  void sendShoutcastHeader(const std::string &, bool);

  void connectServer(unsigned &port);

  void connectClient(const std::string &host, const unsigned port);
};

#endif //SIK2_SOCKET_H
