#include "PlayerExecution.hpp"
#include "Utility.hpp"

#include <libssh2.h>
#include <sys/types.h>
#include <pwd.h>

void PlayerExecution::sshExec(std::string hostname, const std::string command) {
  //std::cerr << command << "\n";
  const int ssh_port = 22;

  passwd *user = getpwuid(getuid());
  std::string homeDirectory = user->pw_dir;
  std::string username = user->pw_name;

  boost::smatch result;
  const static boost::regex usernameWithHostnamePattern(R"((\s+?)@(\s+))");
  try {
    if (boost::regex_match(hostname, result, usernameWithHostnamePattern)) {
      username = result[1];
      hostname = result[2];
    }
  } catch (...) {
  }

  SocketTcp ssh;
  ssh.connectClient(hostname, ssh_port);

  LIBSSH2_SESSION *session = libssh2_session_init();

  if (libssh2_session_handshake(session, ssh.get()) != 0) {
    Utility::info("libssh2_session_handshake");
    valid = false;
  }

  std::string id_rsa_pub = homeDirectory + "/.ssh/id_rsa.pub";
  std::string id_rsa = homeDirectory + "/.ssh/id_rsa";

  if (libssh2_userauth_publickey_fromfile(session, username.c_str(), id_rsa_pub.c_str(), id_rsa.c_str(), "") != 0) {
    char *ptr;
    int length;
    libssh2_session_last_error(session, &ptr, &length, 1);
    valid = false;
    Utility::info("libssh2_userauth_publickey_fromfile");
    ptr[length] = '\0';
    //std::cerr << username << " " << hostname << "\n";
    Utility::info(ptr);
    free(ptr);
  }

  LIBSSH2_CHANNEL *channel = libssh2_channel_open_session(session);
  if (libssh2_channel_exec(channel, command.c_str()) != 0) {
    valid = false;
    Utility::info("libssh2_channel_exec");
  }

  const std::chrono::milliseconds timeToWait(200);
  std::this_thread::sleep_for(timeToWait);

  libssh2_channel_close(channel);
  libssh2_channel_free(channel);
  libssh2_session_disconnect(session, "");
  libssh2_session_free(session);

  //std::cerr << "EXECUTED\n";
}

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

  sshExec(computer, "nohup player " + parameters + " &");

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
    //std::cerr << "EVENTS?\n";
    for (epoll_event &event : events) {
      if (!checkUdpEvent(event)) {
      }
    }
  }
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