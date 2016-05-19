#ifndef SIK2_PLAYER_H
#define SIK2_PLAYER_H

#include "Utility.h"

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

  void cleanup(ExitCode exitCode);

  void getArguments(int argc, const char **argv);

  void usage(const char **argv);

  bool checkSocket(epoll_event &event, const Socket &sock, bool header);

  Socket connect();

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

#endif // SIK2_PLAYER_H