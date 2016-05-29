#ifndef SIK2_TELNETSESSION_HPP
#define SIK2_TELNETSESSION_HPP

#include "Socket.hpp"
#include "Epoll.hpp"
#include "Utility.hpp"
#include "PlayerExecution.hpp"

#include <thread>

class TelnetSession {
private:
  struct TimedEvent {
  private:
    int minutes() const {
      return 60 * hh + mm;
    }
  public:
    int hh;
    int mm;
    int m;
    std::string computer;
    std::string parameters;
    int mPort;

    bool operator<(const TimedEvent &b) const {
      return minutes() < b.minutes();
    }
  };

  Socket client;
  bool running;
  std::thread *thread;
  std::vector<PlayerExecution *> playerExecutions;
  std::vector <TimedEvent> timedEvents;

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
