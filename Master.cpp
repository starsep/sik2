#include "Utility.hpp"
#include "Master.hpp"

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
    port = INVALID_PORT;
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

void Master::newConnectionDebug(Socket &client, sockaddr *in_addr, socklen_t *in_len) {
  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
  if (getnameinfo(in_addr, *in_len, hbuf, sizeof hbuf, sbuf, sizeof sbuf,
                  NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
    std::cerr << "Connection with " << client.get() << " (host=" << hbuf
    << ", port=" << sbuf << ")" << std::endl;
    std::cerr.flush();
  }
}

bool Master::checkListeningSocket(epoll_event &event) {
  if (event.data.fd == sock.get()) {
    while (true) {
      sockaddr in_addr;
      socklen_t in_len = sizeof in_addr;
      SocketTcp client = sock.Accept(&in_addr, &in_len);
      if (client.get() == -1) {
        break;
      }
      telnetSessions.push_back(new TelnetSession(client));
      newConnectionDebug(client, &in_addr, &in_len);
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
      checkListeningSocket(event);
    }
  }
}

Master::~Master() {
  for (TelnetSession *t : telnetSessions) {
    delete t;
  }
}