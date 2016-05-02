#include "utility.h"

void usagePlayer(const char **argv) {
  std::cerr << "Usage: " << argv[0] << " host path r-port file m-port md"
            << std::endl;
  _exit(ExitCode::InvalidArguments);
}

std::tuple<std::string, std::string, unsigned, std::string, unsigned, bool>
get_arguments(int argc, const char **argv) {
  if (argc != 7) {
    std::cerr << "Bad number of arguments" << std::endl;
    usagePlayer(argv);
  }
  std::string host(argv[1]);
  std::string path(argv[2]);
  unsigned rPort = getPort(argv[3]);
  std::string filename(argv[4]);
  unsigned mPort = getPort(argv[5]);
  if (rPort == INVALID_PORT || mPort == INVALID_PORT) {
    std::cerr << "Bad port number" << std::endl;
    usagePlayer(argv);
  }
  std::string md(argv[6]);
  if (md != YES && md != NO) {
    std::cerr << "md argument must be yes or no" << std::endl;
    usagePlayer(argv);
  }
  bool metadata = md == YES;
  return std::make_tuple(host, path, rPort, filename, mPort, metadata);
}

int main(int argc, const char **argv) {
  auto arguments = get_arguments(argc, argv);
  std::string host(std::get<0>(arguments));
  std::string path(std::get<1>(arguments));
  unsigned rPort(std::get<2>(arguments));
  std::string filename(std::get<3>(arguments));
  unsigned mPort(std::get<4>(arguments));
  bool metadata = (std::get<5>(arguments));

  File file = filename == stdOut ? stdOutFile : _open(filename.c_str(), 0);

  _exit(ExitCode::Ok);
}