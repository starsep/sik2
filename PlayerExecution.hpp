#ifndef SIK2_PLAYEREXECUTION_HPP
#define SIK2_PLAYEREXECUTION_HPP

#include <string>

class PlayerExecution {
private:
  std::string computer;
  std::string parameters;
  int mPort;
public:
  PlayerExecution(std::string c, std::string p, int m);
  void quit();
};

#endif //SIK2_PLAYEREXECUTION_HPP
