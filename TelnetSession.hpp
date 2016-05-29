#ifndef SIK2_TELNETSESSION_HPP
#define SIK2_TELNETSESSION_HPP

#include "Socket.hpp"
#include "Epoll.hpp"
#include "Utility.hpp"
#include "PlayerExecution.hpp"

#include <thread>
#include <mutex>

class TelnetSession {
private:
  Socket client;
  bool running;
  std::mutex mutex;
  std::thread *telnetThread;
  std::vector<std::thread *> waitingThreads;
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

  void waitForStart(int begin, int m, std::string c, std::string p, int mPort);

  void waitForEnd(int end, int id);

  int launchPlayer(const std::string &computer, const std::string &parameters, int mPort);

  void sendClient(const std::string &msg);

  bool checkId(int id);

public:
  TelnetSession(Socket c);

  TelnetSession(const TelnetSession &) = delete;

  ~TelnetSession();
};


#endif //SIK2_TELNETSESSION_HPP
