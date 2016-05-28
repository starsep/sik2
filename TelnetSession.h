#ifndef SIK2_TELNETSESSION_H
#define SIK2_TELNETSESSION_H

#include "Socket.h"
#include "Epoll.h"
#include "Utility.h"

#include <thread>

class TelnetSession {
private:
  Socket client;
  bool running;
  std::thread *thread;

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


#endif //SIK2_TELNETSESSION_H