#ifndef SIK2_EPOLL_H
#define SIK2_EPOLL_H

#include "Socket.h"
#include <sys/epoll.h>

class Epoll {
private:
  int efd;
public:
  void addEvent(const Socket &socket);
  int wait(epoll_event *, int, int);
  Epoll();
};

#endif //SIK2_EPOLL_H
