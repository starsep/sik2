#ifndef SIK2_MASTER_H
#define SIK2_MASTER_H

#include "Utility.h"
#include "Socket.h"

class Master {
private:
  unsigned port;
  Socket sock;

  void usage(const char **argv);

  void getArguments(int argc, const char **argv);

  bool checkSocket(epoll_event &event);

  void cleanup(ExitCode exitCode);

public:
  Master(int argc, const char **argv);

  void run();
};

#endif // SIK2_MASTER_H