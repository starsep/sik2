#include "utility.h"

void cleanup(ExitCode exitCode) { _exit(exitCode); }

void usagePlayer(const char **argv) {
  std::cerr << "Usage: " << argv[0] << " host path r-port file m-port md" << std::endl;
  cleanup(ExitCode::InvalidArguments);
}

std::tuple<std::string, std::string, unsigned, std::string, unsigned, bool>
getArguments(int argc, const char **argv) {
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

bool stillHeader = true;
int metaInt = 0;
int byteCounter = 0;
int metadataCount = 0;
std::string metaData;

void handleMetadata() {
  boost::smatch what;
  const static boost::regex metaIntPattern(R"(StreamTitle='(.*?)';)");
  try {
    if (boost::regex_search(metaData, what, metaIntPattern)) {
      std::cerr << what[1] << std::endl;
    }
  }
  catch (...) {
  }
  metaData = "";
}


void printData(const std::string &data) {
  std::cout << data;
  std::cout.flush();
}

void handleData(std::string &data, bool metadata) {
  stillHeader = false;
  if (metadata && metaInt > 0) {
    auto subtractMetadata = [&data, &metaData] () {
      size_t oldSize = metaData.size();
      metaData += data.substr(0, metadataCount);
      data = data.substr(metaData.size() - oldSize, data.size());
      metadataCount -= metaData.size();
      if (metadataCount <= 0) {
        metadataCount = 0;
        handleMetadata();
      }
    };
    if (metadataCount > 0) {
      subtractMetadata();
    }
    byteCounter += data.size();
    if (byteCounter > metaInt) {
      byteCounter -= data.size();
      int dataSize = metaInt - byteCounter;
      printData(data.substr(0, dataSize));
      byteCounter += dataSize;
      byteCounter %= metaInt;
      data = data.substr(dataSize, data.size());
      metadataCount = (unsigned char) data[0] * 16;
      data = data.substr(1, data.size());
      subtractMetadata();
      printData(data);
      byteCounter += data.size();
      byteCounter %= metaInt;
    } else {
      printData(data);
    }
  } else {
    printData(data);
  }
}

void handleHeaderLine(const std::string &line) {
  std::cerr << line << std::endl;
  boost::smatch what;
  const static boost::regex metaIntPattern(R"(icy-metaint:\s*(\d+).*)");
  try {
    if (boost::regex_match(line, what, metaIntPattern)) {
      metaInt = std::stoi(what[1]);
    }
  }
  catch (...) {
  }
}

void handleHeader(const std::string &header, bool metadata) {
  std::stringstream sstream(header);
  std::string line;
  while (!sstream.eof()) {
    std::getline(sstream, line);
    if (line.empty() || line == "\r") {
      std::getline(sstream, line, char(EOF));
      return handleData(line, metadata);
    }
    handleHeaderLine(line);
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
  auto arguments = getArguments(argc, argv);
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
  while (true) {
    epoll_event *events = new epoll_event[MAX_SOCKETS_PLAYER];
    int numberOfEvents = epoll_wait(efd, events, MAX_SOCKETS_PLAYER, MAX_TIME);
    for (int i = 0; i < numberOfEvents; i++) {
      checkSocket(events[i], sock, metadata, stillHeader);
    }
    delete[] events;
  }
}