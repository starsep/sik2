#include "Utility.h"
#include "Master.h"

void Master::usage(const char **argv) {
  std::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
  Utility::_exit(ExitCode::InvalidArguments);
}

void Master::getArguments(int argc, const char **argv) {
  if (argc < 1 || argc > 2) {
    std::cerr << "Bad number of arguments" << std::endl;
    usage(argv);
  }
  if (argc == 1) {
    port = Utility::getFreePort();
    return;
  }
  port = Utility::getPort(argv[1]);
  if (port == INVALID_PORT) {
    std::cerr << "Bad port number" << std::endl;
    usage(argv);
  }
}

void Master::connect() {
  unsigned lastPort = port;
  sock = Socket::connectServer(port);
  if (lastPort != port) {
    std::cout << port << std::endl;
  }
}

Master::Master(int argc, const char **argv) :
    sock(0) {
  getArguments(argc, argv);
}

void Master::run() {
  connect();
}