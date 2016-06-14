#ifndef SIK2_PLAYEREXECUTION_HPP
#define SIK2_PLAYEREXECUTION_HPP

#include "SocketUdp.hpp"
#include "SocketTcp.hpp"

#include <string>
#include <thread>
#include <mutex>

class PlayerExecution {
private:
  bool valid;
  bool running;
  std::string computer;
  std::string parameters;
  unsigned mPort;
  SocketUdp udp;
  SocketTcp telnet;
  int id;
  std::mutex &mutex;

  bool checkUdpEvent(epoll_event &e);

public:
  std::thread thread;

  PlayerExecution(std::string c, std::string p, unsigned m, std::mutex &mut, SocketTcp &s, int i);

  void quit();

  void run();

  bool isValid();

  void sendCommand(const std::string &s);
};

#endif //SIK2_PLAYEREXECUTION_HPP
