#include "utility.h"

void usageMaster(const char **argv) {
  std::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
  _exit(ExitCode::InvalidArguments);
}

unsigned get_arguments(int argc, const char **argv) {
  if (argc < 1 || argc > 2) {
    std::cerr << "Bad number of arguments" << std::endl;
    usageMaster(argv);
  }
  if (argc == 1) {
    unsigned port = getFreePort();
    std::cerr << port << std::endl;
    return port;
  }
  unsigned port = getPort(argv[1]);
  if (port == INVALID_PORT) {
    std::cerr << "Bad port number" << std::endl;
    usageMaster(argv);
  }
  return port;
}

int main(int argc, const char **argv) {
  unsigned port = get_arguments(argc, argv);
}