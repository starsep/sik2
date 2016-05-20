#ifndef SIK2_MASTER_H
#define SIK2_MASTER_H

#include "Socket.h"

class Master {
private:
  unsigned port;
  Socket sock;

  void usage(const char **argv);

  void getArguments(int argc, const char **argv);

public:
  Master(int argc, const char **argv);

  void run();
};

#endif // SIK2_MASTER_H