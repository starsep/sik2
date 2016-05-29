#include "TelnetSession.hpp"

TelnetSession::TelnetSession(Socket c) :
    client(c.get()),
    running(true),
    thread(new std::thread(&TelnetSession::run, this)),
    playerExecutions() {
}

TelnetSession::TelnetSession(const TelnetSession &t) {
  client = Socket(t.client.get());
  thread = t.thread;
  playerExecutions = t.playerExecutions;
}

void TelnetSession::run() {
  client.makeNonBlocking();
  Epoll efd{};
  efd.addEvent(client);
  while (running) {
    std::vector <epoll_event> events = efd.wait(MAX_EVENTS_TELNET, INF);
    for (epoll_event &event : events) {
      checkTelnetEvent(event);
    }
  }
}

bool TelnetSession::checkStart(const std::string &command) {
  if (!Utility::startsWith(command, START)) {
    return false;
  }
  boost::smatch result;
  const static boost::regex startPattern(START + R"(\s+(\S+)\s+(\S+)\s+(\S+)\s+(\d+)\s+(\S+)\s+(\d+)\s+(\l{2,3})\s*)");
  try {
    if (boost::regex_match(command, result, startPattern)) {
      std::string computer = result[1];
      std::string host = result[2];
      std::string path = result[3];
      int rPort = std::stoi(result[4]);
      std::string file = result[5];
      int mPort = std::stoi(result[6]);
      std::string metadata = result[7];
      if (metadata != NO && metadata != YES) {
        static const std::string INVALID_START_METADATA = "ERROR metadata option must be " + YES + " or " + NO + "\n";
        client.Write(INVALID_START_METADATA);
        return true;
      }
      std::cerr << START << "ing player @ " << computer <<
      " with parameters: " << host << " " << path << " " << rPort << " " << file << " " << mPort << " " << metadata <<
      "\n";
      return true;
    }
  }
  catch (...) {
  }
  static const std::string INVALID_START =
      "ERROR in " + START + " command. Usage: " + START + " computer host path r-port file m-port metadata\n";
  client.Write(INVALID_START);
  return true;
}

bool TelnetSession::checkAt(const std::string &command) {
  if (!Utility::startsWith(command, AT)) {
    return false;
  }
  boost::smatch result;
  const static boost::regex atPattern(
      AT + R"(\s+(\d{2})\.(\d{2})\s+(\d+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\d+)\s+(\S+)\s+(\d+)\s+(\l{2,3})\s*)");
  try {
    if (boost::regex_match(command, result, atPattern)) {
      int hh = std::stoi(result[1]);
      int mm = std::stoi(result[2]);
      int m = std::stoi(result[3]);
      std::string computer = result[4];
      std::string host = result[5];
      std::string path = result[6];
      int rPort = std::stoi(result[7]);
      std::string file = result[8];
      int mPort = std::stoi(result[9]);
      std::string metadata = result[10];
      std::cerr << AT << "ing player @ " << computer <<
      " AT " << hh << ":" << mm << " for " << m << " minutes with parameters: " << host << " " << path << " " <<
      rPort <<
      " " << file << " " << mPort << " " << metadata <<
      "\n";
      return true;
    }
  }
  catch (...) {
  }
  static const std::string INVALID_AT =
      "ERROR in " + AT + " command. Usage: " + AT + " HH.MM M computer host path r-port file m-port metadata\n";
  client.Write(INVALID_AT);
  return true;
}

bool TelnetSession::checkPlay(const std::string &command) {
  if (!Utility::startsWith(command, PLAY)) {
    return false;
  }
  boost::smatch result;
  const static boost::regex playPattern(PLAY + R"(\s+(\d+)\s*)");
  try {
    if (boost::regex_match(command, result, playPattern)) {
      std::cerr << PLAY << " " << result[1] << "\n";
      return true;
    }
  }
  catch (...) {
  }
  static const std::string INVALID_PLAY =
      "ERROR in " + PLAY + " command. Usage: " + PLAY + " id\n";
  client.Write(INVALID_PLAY);
  return true;
}

bool TelnetSession::checkPause(const std::string &command) {
  if (!Utility::startsWith(command, PAUSE)) {
    return false;
  }
  boost::smatch result;
  const static boost::regex pausePattern(PAUSE + R"(\s+(\d+)\s*)");
  try {
    if (boost::regex_match(command, result, pausePattern)) {
      std::cerr << PAUSE << " " << result[1] << "\n";
      return true;
    }
  }
  catch (...) {
  }
  static const std::string INVALID_PAUSE =
      "ERROR in " + PAUSE + " command. Usage: " + PAUSE + " id\n";
  client.Write(INVALID_PAUSE);
  return true;
}

bool TelnetSession::checkQuit(const std::string &command) {
  if (!Utility::startsWith(command, QUIT)) {
    return false;
  }
  boost::smatch result;
  const static boost::regex quitPattern(QUIT + R"(\s+(\d+)\s*)");
  try {
    if (boost::regex_match(command, result, quitPattern)) {
      std::cerr << QUIT << " " << result[1] << "\n";
      return true;
    }
  }
  catch (...) {
  }
  static const std::string INVALID_QUIT =
      "ERROR in " + QUIT + " command. Usage: " + QUIT + " id\n";
  client.Write(INVALID_QUIT);
  return true;
}

bool TelnetSession::checkTitle(const std::string &command) {
  if (!Utility::startsWith(command, TITLE)) {
    return false;
  }
  boost::smatch result;
  const static boost::regex titlePattern(TITLE + R"(\s+(\d+)\s*)");
  try {
    if (boost::regex_match(command, result, titlePattern)) {
      std::cerr << TITLE << " " << result[1] << "\n";
      return true;
    }
  }
  catch (...) {
  }
  static const std::string INVALID_TITLE =
      "ERROR in " + TITLE + " command. Usage: " + TITLE + " id\n";
  client.Write(INVALID_TITLE);
  return true;
}

bool TelnetSession::checkCommand(const std::string &c) {
  return checkAt(c) || checkPause(c) || checkPlay(c) || checkQuit(c) || checkStart(c) || checkTitle(c);
}

void TelnetSession::checkTelnetEvent(epoll_event &event) {
  if (event.data.fd == client.get()) {
    //std::cerr << "TELNET EVENT FROM " << event.data.fd << " " << client.get() << " :D\n";
    std::string msg;
    try {
      msg = client.receiveOnce();
    } catch (ClosedConnectionException) {
      running = false;
      return;
    }
    if (!checkCommand(msg)) {
      static const std::string INVALID_COMMAND = "ERROR Invalid command. Available: AT START PLAY PAUSE QUIT TITLE\n";
      client.Write(INVALID_COMMAND);
    }
  }
}

TelnetSession::~TelnetSession() {
  delete thread;
}