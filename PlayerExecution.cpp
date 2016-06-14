#include "PlayerExecution.hpp"
#include "Utility.hpp"

PlayerExecution::PlayerExecution(std::string c, std::string p, unsigned m, std::mutex &mut, SocketTcp &s, int i) :
    valid(true),
    computer(c),
    parameters(p),
    mPort(m),
    udp(),
    telnet(s),
    id(i),
    mutex(mut),
    thread(&PlayerExecution::run, this) {
}

bool PlayerExecution::checkUdpEvent(epoll_event &e) {
  if (e.data.fd == udp.get()) {
    std::string msg = udp.receiveOnce() + "\n";
    mutex.lock();
    telnet.Send(msg);
    mutex.unlock();
  }
  return e.data.fd == udp.get();
}

void PlayerExecution::run() {
  running = true;
  udp.connectClient(computer, mPort);
  valid &= udp.checkValid();
  std::string msg;

  auto a = [this]() {
    std::string command = "nohup bash -l -c \\\"player " + parameters + "\\\"";
    std::string systemCommand = "ssh " + computer + " \"" + command + "\"";
    valid &= system(systemCommand.c_str()) == 0;
  };

  std::thread t(a);

  if (valid) {
    msg = "OK " + std::to_string(id) + "\n";
  } else {
    msg = "ERROR " + std::to_string(id) + " player launch failed\n";
  }

  mutex.lock();
  telnet.Send(msg);
  mutex.unlock();

  Epoll efd{};
  efd.addEvent(udp);

  while (running) {
    std::vector <epoll_event> events = efd.wait(MAX_EVENTS_TELNET, MAX_TIME_MS);
    for (epoll_event &event : events) {
      if (!checkUdpEvent(event)) {
      }
    }
  }
  t.join();
}

void PlayerExecution::quit() {
  udp.Send(QUIT);
  running = false;
}

bool PlayerExecution::isValid() {
  return valid;
}

void PlayerExecution::sendCommand(const std::string &s) {
  udp.Send(s);
}
