#include "utility.h"

void cleanup(ExitCode exitCode) { _exit(exitCode); }

void usagePlayer(const char **argv) {
  std::cerr << "Usage: " << argv[0] << " host path r-port file m-port md"
            << std::endl;
  cleanup(ExitCode::InvalidArguments);
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
    std::cerr << "md argument must be " << YES << " or " << NO << std::endl;
    usagePlayer(argv);
  }
  bool metadata = md == YES;
  return std::make_tuple(host, path, rPort, filename, mPort, metadata);
}

Socket connectPlayer(const std::string &host, unsigned port) {
  std::cerr << "Connecting with: " << host << " on port: " << port << std::endl;
  addrinfo hints;
  addrinfo *result;

  _getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);

  Socket sock =
      _socket(result->ai_family, result->ai_socktype, result->ai_protocol);

  _connect(sock, result->ai_addr, result->ai_addrlen);
  freeaddrinfo(result);

  return sock;
}

void handleData(const std::string &data, bool metadata) {
  std::cout << data;
  std::cout.flush();
}

void handleHeader(const std::string &header, bool metadata) {
  std::stringstream sstream(header);
  std::string line;
  while (!sstream.eof()) {
    std::getline(sstream, line);
    if (line.empty()) {
      std::getline(sstream, line, char(EOF));
      return handleData(line, metadata);
    }
    std::cerr << line << std::endl;
  }
}

bool checkSocket(epoll_event &event, Socket sock, bool metadata, bool header) {
  if (event.data.fd == sock) {
    try {
      std::string data = receiveShoutcast(sock, metadata);
      //      std::cerr << data.size() << std::endl;
      if (header) {
        handleHeader(data, metadata);
      } else {
        handleData(data, metadata);
      }
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

int main(int argc, const char **argv) {
  auto arguments = get_arguments(argc, argv);
  std::string host(std::get<0>(arguments));
  std::string path(std::get<1>(arguments));
  unsigned rPort(std::get<2>(arguments));
  std::string filename(std::get<3>(arguments));
  unsigned mPort(std::get<4>(arguments));
  bool metadata = (std::get<5>(arguments));

  if (filename == stdOut) {
    filename = stdOutFilename;
  }
  std::ofstream ofstream(filename, std::ios_base::binary | std::ios_base::out);
  std::cout.rdbuf(ofstream.rdbuf());

  Socket sock = connectPlayer(host, rPort);
  makeSocketNonBlocking(sock);
  Epoll efd = _epoll_create();
  addEpollEvent(efd, sock);
  sendShoutcastHeader(sock, path, metadata);

  int headerCounter = 0;
  while (true) {
    epoll_event *events = new epoll_event[MAX_SOCKETS_PLAYER];
    int numberOfEvents = epoll_wait(efd, events, MAX_SOCKETS_PLAYER, MAX_TIME);
    // std::cerr << "here I am: " << numberOfEvents << std::endl;
    for (int i = 0; i < numberOfEvents; i++) {
      checkSocket(events[i], sock, metadata, headerCounter < HEADER_EVENTS);
      if (headerCounter < HEADER_EVENTS) {
        headerCounter++;
      }
    }
    delete[] events;
  }
}