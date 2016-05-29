#ifndef SIK2_PLAYEREXECUTION_HPP
#define SIK2_PLAYEREXECUTION_HPP

#include "SocketUdp.hpp"
#include "SocketTcp.hpp"

#include <string>
#include <thread>

class PlayerExecution {
private:
  std::string computer;
  std::string parameters;
  unsigned mPort;
  SocketUdp udp;
  SocketTcp telnet;
  std::thread thread;
public:
  PlayerExecution(std::string c, std::string p, unsigned m);
  void quit();
  void run();
};

#endif //SIK2_PLAYEREXECUTION_HPP
