#ifndef SIK2_MASTER_HPP
#define SIK2_MASTER_HPP

#include "Utility.hpp"
#include "SocketUdp.hpp"
#include "TelnetSession.hpp"

#include <thread>
#include <vector>

class Master {
private:
  unsigned port;
  SocketTcp sock;
  std::vector<TelnetSession *> telnetSessions;

  void usage(const char **argv);

  void getArguments(int argc, const char **argv);

  bool checkListeningSocket(epoll_event &event);

  void cleanup(ExitCode exitCode);

  void newConnectionDebug(Socket &client, sockaddr *in_addr, socklen_t *in_len);

public:
  Master(int argc, const char **argv);

  void run();

  ~Master();
};

#endif // SIK2_MASTER_HPP