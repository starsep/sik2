#ifndef SIK2_SOCKET_H
#define SIK2_SOCKET_H

#include <netdb.h>
#include <string>

class Socket {
private:
  int sock;
public:
  Socket(int, int, int);

  void Connect(const sockaddr *, socklen_t);

  void Write(const void *, size_t);

  ssize_t Read(void *, size_t);

  int get() const;

  void makeNonBlocking();

  std::string receiveShoutcast(bool);

  void sendShoutcastHeader(const std::string &, bool);
};

#endif //SIK2_SOCKET_H
