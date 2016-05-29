#include "TelnetSession.hpp"
#include <chrono>

TelnetSession::TelnetSession(SocketTcp &c) :
    client(c.get()),
    running(true),
    mutex(),
    telnetThread(new std::thread(&TelnetSession::run, this)),
    waitingThreads(),
    playerExecutions() {
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
  for (std::thread *t: waitingThreads) {
    t->join();
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
      //unsigned rPort = static_cast<unsigned>(std::stoi(result[4]));
      std::string file = result[5];
      unsigned mPort = static_cast<unsigned>(std::stoi(result[6]));
      std::string metadata = result[7];
      if (metadata != NO && metadata != YES) {
        static const std::string INVALID_START_METADATA = "ERROR metadata option must be " + YES + " or " + NO + "\n";
        sendClient(INVALID_START_METADATA);
        return true;
      }
      std::string parameters;
      for (unsigned i = 2; i <= 7; i++) {
        parameters += result[i] + " ";
      }
      /*std::cerr << START << "ing player @ " << computer <<
      " with parameters: " << host << " " << path << " " << rPort << " " << file << " " << mPort << " " << metadata <<
      "\n";*/
      launchPlayer(computer, parameters, mPort);
      return true;
    }
  }
  catch (...) {
  }
  static const std::string INVALID_START =
      "ERROR in " + START + " command. Usage: " + START + " computer host path r-port file m-port metadata\n";
  sendClient(INVALID_START);
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
      if (hh > 23) {
        static const std::string INVALID_AT_HH =
            "ERROR in " + AT + " command. HH is too large\n";
        sendClient(INVALID_AT_HH);
        return true;
      }
      if (mm > 59) {
        static const std::string INVALID_AT_MM =
            "ERROR in " + AT + " command. MM is too large\n";
        sendClient(INVALID_AT_MM);
        return true;
      }
      int m = std::stoi(result[3]);
      std::string computer = result[4];
      std::string host = result[5];
      std::string path = result[6];
      //unsigned rPort = static_cast<unsigned>(std::stoi(result[7]));
      //std::string file = result[8];
      unsigned mPort = static_cast<unsigned>(std::stoi(result[9]));
      std::string metadata = result[10];
      if (metadata != NO && metadata != YES) {
        static const std::string INVALID_AT_METADATA = "ERROR metadata option must be " + YES + " or " + NO + "\n";
        sendClient(INVALID_AT_METADATA);
        return true;
      }
      std::string parameters;
      for (unsigned i = 5; i <= 10; i++) {
        parameters += result[i] + " ";
      }
      /*std::cerr << AT << "ing player @ " << computer <<
      " AT " << hh << ":" << mm << " for " << m << " minutes with parameters: " << parameters <<
      "\n";*/
      waitingThreads.push_back(
          new std::thread(&TelnetSession::waitForStart, this, hh * 60 + mm, m, computer, parameters, mPort));
      return true;
    }
  }
  catch (...) {
  }
  static const std::string INVALID_AT =
      "ERROR in " + AT + " command. Usage: " + AT + " HH.MM M computer host path r-port file m-port metadata\n";
  sendClient(INVALID_AT);
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
      unsigned id = std::stoi(result[1]);
      if (!checkId(id)) {
        static const std::string INVALID_PLAY_ID =
            "ERROR in " + PLAY + " command. Invalid id.\n";
        sendClient(INVALID_PLAY_ID);
        return true;
      }
      return true;
    }
  }
  catch (...) {
  }
  static const std::string INVALID_PLAY =
      "ERROR in " + PLAY + " command. Usage: " + PLAY + " id\n";
  sendClient(INVALID_PLAY);
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
      unsigned id = std::stoi(result[1]);
      if (!checkId(id)) {
        static const std::string INVALID_PAUSE_ID =
            "ERROR in " + PAUSE + " command. Invalid id.\n";
        sendClient(INVALID_PAUSE_ID);
        return true;
      }
      return true;
    }
  }
  catch (...) {
  }
  static const std::string INVALID_PAUSE =
      "ERROR in " + PAUSE + " command. Usage: " + PAUSE + " id\n";
  sendClient(INVALID_PAUSE);
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
      unsigned id = std::stoi(result[1]);
      if (!checkId(id)) {
        static const std::string INVALID_QUIT_ID =
            "ERROR in " + QUIT + " command. Invalid id.\n";
        sendClient(INVALID_QUIT_ID);
        return true;
      }
      quitPlayerExecution(id);
      const std::string OK_QUIT = "OK " + std::to_string(id) + "\n";
      sendClient(OK_QUIT);
      return true;
    }
  }
  catch (...) {
  }
  static const std::string INVALID_QUIT =
      "ERROR in " + QUIT + " command. Usage: " + QUIT + " id\n";
  sendClient(INVALID_QUIT);
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
      unsigned id = std::stoi(result[1]);
      if (!checkId(id)) {
        static const std::string INVALID_TITLE_ID =
            "ERROR in " + TITLE + " command. Invalid id.\n";
        sendClient(INVALID_TITLE_ID);
        return true;
      }
      return true;
    }
  }
  catch (...) {
  }
  static const std::string INVALID_TITLE =
      "ERROR in " + TITLE + " command. Usage: " + TITLE + " id\n";
  sendClient(INVALID_TITLE);
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
      msg = client.receive();
    } catch (ClosedConnectionException) {
      running = false;
      return;
    }
    if (!checkCommand(msg)) {
      static const std::string INVALID_COMMAND = "ERROR Invalid command. Available: AT START PLAY PAUSE QUIT TITLE\n";
      sendClient(INVALID_COMMAND);
    }
  }
}

TelnetSession::~TelnetSession() {
  for (std::thread *t: waitingThreads) {
    delete t;
  }
  delete telnetThread;
}

void TelnetSession::waitForStart(int begin, int m, std::string c, std::string p, unsigned mPort) {
  const std::chrono::milliseconds timeToWait(MAX_TIME_MS);
  while (running && Utility::currentMinutes() != begin) {
    std::this_thread::sleep_for(timeToWait);
  }
  if (!running) {
    return;
  }
  unsigned id = launchPlayer(c, p, mPort);
  waitForEnd((begin + m) % (24 * 60), id);
}

void TelnetSession::waitForEnd(int end, unsigned id) {
  const std::chrono::milliseconds timeToWait(MAX_TIME_MS);
  while (running && Utility::currentMinutes() != end) {
    std::this_thread::sleep_for(timeToWait);
  }
  quitPlayerExecution(id);
}

unsigned TelnetSession::launchPlayer(const std::string &computer, const std::string &parameters, unsigned mPort) {
  mutex.lock();
  playerExecutions.push_back(new PlayerExecution(computer, parameters, mPort));
  unsigned id = playerExecutions.size() - 1;
  mutex.unlock();
  std::string msg = "OK " + std::to_string(id) + "\n";
  sendClient(msg);
  return id;
}

void TelnetSession::sendClient(const std::string &msg) {
  mutex.lock();
  client.Send(msg);
  mutex.unlock();
}

bool TelnetSession::checkId(unsigned id) {
  mutex.lock();
  bool result = id < playerExecutions.size() && playerExecutions[id] != nullptr;
  mutex.unlock();
  return result;
}

void TelnetSession::quitPlayerExecution(unsigned id) {
  if (checkId(id)) {
    mutex.lock();
    PlayerExecution *toRemove = playerExecutions[id];
    playerExecutions[id] = nullptr;
    mutex.unlock();
    toRemove->quit();
    delete toRemove;
  }
}