#include "PlayerExecution.hpp"
#include "Utility.hpp"

#include <libssh2.h>
#include <sys/types.h>
#include <pwd.h>

/*static std::string sshExec(std::string hostname, std::string command) {
//   std::cout << "--- SSH EXEC --- \n";
//   std::cout << "HOSTNAME: " << hostname << "\n";
//   std::cout << "COMMAND: " << command << "\n";

  const int port = 22;
  passwd *user = getpwuid(getuid());
  std::string homeDirectory = user->pw_dir;
  std::string username = "fc359081";//user->pw_name;

  size_t index = hostname.find("@");
  if (index != std::string::npos) {
    username = hostname.substr(0, index);
    hostname = hostname.substr(index + 1, hostname.length() - (index + 1));
  }

  telnet.connectClient(hostname, port);

  LIBSSH2_SESSION *session = libssh2_session_init();

  if (libssh2_session_handshake(session, connection.get()) != 0) {
    Utility::info("libssh2_session_handshake");
  }

  std::string id_rsa_pub = homeDirectory + "/.ssh/id_rsa.pub";
  std::string id_rsa = homeDirectory + "/.ssh/id_rsa";

  if (libssh2_userauth_publickey_fromfile(session, username.c_str(), id_rsa_pub.c_str(), id_rsa.c_str(), "") != 0) {
    char *ptr;
    int length;
    libssh2_session_last_error(session, &ptr, &length, 1);
    free(ptr);
    return "ERROR";
  }


  LIBSSH2_CHANNEL *channel = libssh2_channel_open_session(session);
  if (libssh2_channel_exec(channel, command.c_str()) != 0) return "ERROR";

  std::string buffer(1000, 0);
  std::string log;
  int size;
  while ((size = libssh2_channel_read(channel, buffer.begin().base(), buffer.length())) > 0) {
    log += std::string(buffer.begin(), buffer.begin() + size);
    //std::cout << std::string(buffer.begin(), buffer.begin() + size) << "\n";
  }

  *//*if (size == 0) {
    std::cout << "OK Player finished.\n" + log;
  }
  else {
    std::cout << "ERROR Connection interrupted.\n" + log;
  }*//*

  libssh2_channel_close(channel);
  libssh2_channel_free(channel);
  libssh2_session_disconnect(session, "");
  libssh2_session_free(session);

  return log;
}*/

//std::cerr << sshExec("students.mimuw.edu.pl", "source $HOME/.bash_profile; player ant-waw-01.cdn.eurozet.pl / 8602 file.mp3 50000 no");

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

void PlayerExecution::run() {
  udp.connectClient(computer, mPort);
  valid &= udp.checkValid();
  std::string msg;
  if (valid) {
    msg = "OK " + std::to_string(id) + "\n";
  } else {
    msg = "ERROR " + std::to_string(id) + " player launch failed\n";
  }
  mutex.lock();
  telnet.Send(msg);
  mutex.unlock();
  /*if (system("player ant-waw-01.cdn.eurozet.pl / 8602 /tmp/file.mp3 50000 no") != 0) {
    Utility:info("system");
  }*/
}

void PlayerExecution::quit() {
  udp.Send(QUIT);
}

bool PlayerExecution::isValid() {
  return valid;
}

void PlayerExecution::sendCommand(const std::string &s) {
  udp.Send(s);
}