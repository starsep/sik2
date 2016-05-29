#ifndef SIK2_EPOLL_HPP
#define SIK2_EPOLL_HPP

#include <sys/epoll.h>
#include <vector>

class Socket;

class Epoll {
private:
  int efd;
public:
  void addEvent(const Socket &socket);

  std::vector <epoll_event> wait(int, int);

  Epoll();
};

#endif //SIK2_EPOLL_HPP
