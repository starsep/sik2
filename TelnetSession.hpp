#ifndef SIK2_TELNETSESSION_HPP
#define SIK2_TELNETSESSION_HPP

#include "Socket.hpp"
#include "Epoll.hpp"
#include "Utility.hpp"
#include "PlayerExecution.hpp"

#include <thread>

class TelnetSession {
private:
  Socket client;
  bool running;
  std::thread *thread;
  std::vector<PlayerExecution *> playerExecutions;

  void run();

  void checkTelnetEvent(epoll_event &event);

  bool checkCommand(const std::string &);

  bool checkStart(const std::string &);

  bool checkAt(const std::string &);

  bool checkPlay(const std::string &);

  bool checkPause(const std::string &);

  bool checkQuit(const std::string &);

  bool checkTitle(const std::string &);

public:
  TelnetSession(Socket c);

  TelnetSession(const TelnetSession &);

  ~TelnetSession();
};


#endif //SIK2_TELNETSESSION_HPP
