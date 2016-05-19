#include "utility.h"

class Master {
private:
  unsigned port;

  void usage(const char **argv) {
    std::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
    _exit(ExitCode::InvalidArguments);
  }

  void getArguments(int argc, const char **argv) {
    if (argc < 1 || argc > 2) {
      std::cerr << "Bad number of arguments" << std::endl;
      usage(argv);
    }
    if (argc == 1) {
      port = getFreePort();
      std::cerr << port << std::endl;
      return;
    }
    port = getPort(argv[1]);
    if (port == INVALID_PORT) {
      std::cerr << "Bad port number" << std::endl;
      usage(argv);
    }
  }

public:
  Master(int argc, const char **argv) {
    getArguments(argc, argv);
  }

  void run() {

  }
};


int main(int argc, const char **argv) {
  Master master(argc, argv);
  master.run();
}