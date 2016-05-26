#include "Utility.h"
#include "Player.h"

void Player::cleanup(ExitCode exitCode) { Utility::_exit(exitCode); }

void Player::getArguments(int argc, const char **argv) {
  if (argc != 7) {
    std::cerr << "Bad number of arguments" << std::endl;
    usage(argv);
  }
  host = std::string(argv[1]);
  path = std::string(argv[2]);
  rPort = Utility::getPort(argv[3]);
  filename = std::string(argv[4]);
  mPort = Utility::getPort(argv[5]);
  if (rPort == INVALID_PORT || mPort == INVALID_PORT) {
    std::cerr << "Bad port number" << std::endl;
    usage(argv);
  }
  std::string md(argv[6]);
  if (md != YES && md != NO) {
    std::cerr << "md argument must be " << YES << " or " << NO << std::endl;
    usage(argv);
  }
  metadata = md == YES;
}

void Player::usage(const char **argv) {
  std::cerr << "Usage: " << argv[0] << " host path r-port file m-port md" << std::endl;
  cleanup(ExitCode::InvalidArguments);
}

bool Player::checkShoutcastSocket(epoll_event &event) {
  if (event.data.fd == shoutcast.get()) {
    try {
      std::string data = shoutcast.receive();
      if (stillHeader) {
        handleHeader(data);
      } else {
        handleData(data);
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

bool Player::checkUdpSocket(epoll_event &event) {
  if (event.data.fd == udp.get()) {
    std::string msg = udp.receiveOnce();
    std::cerr << "UDP MSG: ";
    std::cerr << msg;
    if (msg == "PAUSE\n") {
      playing = false;
    } else if(msg == "PLAY\n") {
      playing = true;
    } else if(msg == "QUIT\n") {
      cleanup(ExitCode::Ok);
    } else if(msg == "TITLE\n") {
      std::cerr << "SENDING TITLE\n";
      //TODO
    }
    return true;
  }
  return false;
}

void Player::handleMetadata() {
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


void Player::printData(const std::string &data) {
  std::cout << data;
  std::cout.flush();
}

void Player::subtractMetadata(std::string &data) {
  size_t oldSize = metaData.size();
  metaData += data.substr(0, metadataCount);
  data = data.substr(metaData.size() - oldSize, data.size());
  metadataCount -= metaData.size();
  if (metadataCount <= 0) {
    metadataCount = 0;
    handleMetadata();
  }
}

void Player::handleData(std::string &data) {
  if (!playing) {
    return;
  }
  stillHeader = false;
  if (metadata && metaInt > 0) {
    if (metadataCount > 0) {
      subtractMetadata(data);
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
      subtractMetadata(data);
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

void Player::handleHeaderLine(const std::string &line) {
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

void Player::handleHeader(const std::string &header) {
  std::stringstream sstream(header);
  std::string line;
  while (!sstream.eof()) {
    std::getline(sstream, line);
    if (line.empty() || line == "\r") {
      std::getline(sstream, line, char(EOF));
      return handleData(line);
    }
    handleHeaderLine(line);
  }
}

Player::Player(int argc, const char **argv) :
    stillHeader(true),
    metaInt(8192),
    byteCounter(0),
    metadataCount(0),
    shoutcast(0),
    udp(0),
    playing(true) {
  getArguments(argc, argv);

}


void Player::run() {
  if (filename == stdOut) {
    filename = stdOutFilename;
  }
  std::ofstream ofstream(filename, std::ios_base::binary | std::ios_base::out);
  std::cout.rdbuf(ofstream.rdbuf());

  std::cerr << "Connecting with: " << host << " on port: " << rPort << std::endl;
  shoutcast.connectClient(host, rPort);
  udp.connectUdp(mPort);

  Epoll efd{};
  efd.addEvent(shoutcast);
  efd.addEvent(udp);
  shoutcast.sendShoutcastHeader(path, metadata);

  while (true) {
    std::vector <epoll_event> events = efd.wait(MAX_SOCKETS_PLAYER, MAX_TIME);
    for (epoll_event &event : events) {
      if (!checkShoutcastSocket(event)) {
        checkUdpSocket(event);
      }
    }
  }
}