#include "Epoll.hpp"
#include "Utility.hpp"
#include "Socket.hpp"

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <csignal>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

void Epoll::addEvent(const Socket &sock) {
  if (!checkValid()) {
    return;
  }
  epoll_event event;
  memset(&event, 0, sizeof(epoll_event));
  event.data.fd = sock.get();
  event.events = EPOLLIN | EPOLLET;
  if (epoll_ctl(efd, EPOLL_CTL_ADD, sock.get(), &event) == -1) {
    Utility::info("epoll_ctl");
  }
}

Epoll::Epoll() {
  valid = true;
  efd = epoll_create1(0);
  if (efd == -1) {
    Utility::info("epoll_create");
  }
}

std::vector <epoll_event> Epoll::wait(int maxevents, int timeout) {
  std::vector <epoll_event> result;
  if (!checkValid()) {
    return result;
  }
  result.resize(maxevents);
  epoll_wait(efd, result.data(), maxevents, timeout);
  return result;
}

void Epoll::makeInvalid() {
  valid = false;
}

bool Epoll::checkValid() {
  if (!valid) {
    Utility::info("Epoll is invalid");
    return false;
  }
  return true;
}