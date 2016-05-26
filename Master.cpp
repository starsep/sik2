#include "Utility.h"
#include "Master.h"

void Master::cleanup(ExitCode exitCode) { Utility::_exit(exitCode); }

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

Master::Master(int argc, const char **argv) :
    sock(0) {
  getArguments(argc, argv);
}

bool Master::checkSocket(epoll_event &event) {
  if (event.data.fd == sock.get()) {
    try {
      std::string data = sock.receive();
      std::cerr << data << '\n';
    } catch (BadNetworkDataException) {
      std::cerr << "Incorrect data received from server. Exiting." << std::endl;
      cleanup(ExitCode::BadData);
    } catch (ClosedConnectionException) {
      cleanup(ExitCode::Ok);
    }
    return true;
  }
  return false;
}

void Master::run() {
  unsigned lastPort = port;
  sock.connectServer(port);
  if (lastPort != port) {
    std::cout << port << std::endl;
  }

  Epoll efd{};
  efd.addEvent(sock);
  while (true) {
    std::vector <epoll_event> events = efd.wait(MAX_SOCKETS_PLAYER, MAX_TIME);
    for (epoll_event &event : events) {
      checkSocket(event);
    }
  }

}