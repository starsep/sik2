#ifndef SIK2_MASTER_H
#define SIK2_MASTER_H

#endif // SIK2_MASTER_H

class Master {
private:
  unsigned port;

  void usage(const char **argv);

  void getArguments(int argc, const char **argv);

public:
  Master(int argc, const char **argv);

  void run();
};