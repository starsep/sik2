#ifndef SIK2_PLAYER_HPP
#define SIK2_PLAYER_HPP

#include "Utility.hpp"
#include "SocketTcp.hpp"
#include "SocketUdp.hpp"

class Player {
private:
  std::string host;
  std::string path;
  unsigned rPort;
  std::string filename;
  unsigned mPort;
  bool metadata;
  bool stillHeader;
  int metaInt;
  int byteCounter;
  int metadataCount;
  std::string metaData;
  std::string title;
  SocketTcp shoutcast;
  SocketUdp udp;
  bool playing;

  void cleanup(ExitCode exitCode);

  void getArguments(int argc, const char **argv);

  void usage(const char **argv);

  bool checkShoutcastSocket(epoll_event &event);

  bool checkUdpSocket(epoll_event &event);

  void handleMetadata();

  void printData(const std::string &data);

  void subtractMetadata(std::string &data);

  void handleData(std::string &data);

  void handleHeaderLine(const std::string &line);

  void handleHeader(const std::string &header);

public:
  Player(int argc, const char **argv);

  void run();
};

#endif // SIK2_PLAYER_HPP