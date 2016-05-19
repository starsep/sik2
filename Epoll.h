#ifndef SIK2_EPOLL_H
#define SIK2_EPOLL_H

#include "Socket.h"
#include <sys/epoll.h>
#include <vector>

class Epoll {
private:
  int efd;
public:
  void addEvent(const Socket &socket);

  std::vector <epoll_event> wait(int, int);

  Epoll();
};

#endif //SIK2_EPOLL_H
