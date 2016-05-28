#include "Utility.hpp"
#include "Master.hpp"
#include <libssh2.h>
#include <sys/types.h>
#include <pwd.h>

void Master::cleanup(ExitCode exitCode) { Utility::_exit(exitCode); }

void Master::usage(const char **argv) {
  std::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
  Utility::_exit(ExitCode::InvalidArguments);
}

void Master::getArguments(int argc, const char **argv) {
  if (argc < 1 || argc > 2) {
    std::cerr << "Bad number of arguments" << std::endl;
    usage(argv);
  }
  if (argc == 1) {
    port = INVALID_PORT;
    return;
  }
  port = Utility::getPort(argv[1]);
  if (port == INVALID_PORT) {
    std::cerr << "Bad port number" << std::endl;
    usage(argv);
  }
}

Master::Master(int argc, const char **argv) :
    sock(0) {
  getArguments(argc, argv);
}

void Master::newConnectionDebug(Socket &client, sockaddr *in_addr, socklen_t *in_len) {
  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
  if (getnameinfo(in_addr, *in_len, hbuf, sizeof hbuf, sbuf, sizeof sbuf,
                  NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
    std::cerr << "Connection with " << client.get() << " (host=" << hbuf
    << ", port=" << sbuf << ")" << std::endl;
    std::cerr.flush();
  }
}

bool Master::checkListeningSocket(epoll_event &event) {
  if (event.data.fd == sock.get()) {
    while (true) {
      sockaddr in_addr;
      socklen_t in_len = sizeof in_addr;
      Socket client = sock.Accept(&in_addr, &in_len);
      if (client.get() == -1) {
        break;
      }
      telnetSessions.push_back(new TelnetSession(client));
      newConnectionDebug(client, &in_addr, &in_len);
    }
    return true;
  }
  return false;
}

static std::string sshExec(std::string hostname, std::string command) {
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

  Socket connection;
  connection.connectClient(hostname, port);

  LIBSSH2_SESSION *session = libssh2_session_init();

  if (libssh2_session_handshake(session, connection.get()) != 0) {
    Utility::syserr("libssh2_session_handshake");
  }

  /* Connect to the ssh-agent */
  LIBSSH2_AGENT *agent = libssh2_agent_init(session);
  int rc;
  libssh2_agent_publickey *identity = nullptr, *prev_identity = nullptr;

  if (!agent) {
    fprintf(stderr, "Failure initializing ssh-agent support\n");
  }
  if (libssh2_agent_connect(agent)) {
    Utility::syserr("Failure connecting to ssh-agent");
  }
  if (libssh2_agent_list_identities(agent)) {
    fprintf(stderr, "Failure requesting identities to ssh-agent\n");
  }

  while (true) {
    rc = libssh2_agent_get_identity(agent, &identity, prev_identity);
    if (rc == 1) {
      break;
    }
    if (rc < 0) {
      fprintf(stderr,
              "Failure obtaining identity from ssh-agent support\n");
      rc = 1;
    }
    if (libssh2_agent_userauth(agent, username.c_str(), identity)) {

      fprintf(stderr, "\tAuthentication with username %s and "
                  "public key %s failed!\n",
              username.c_str(), identity->comment);
    } else {
      fprintf(stderr, "\tAuthentication with username %s and "
                  "public key %s succeeded!\n",
              username.c_str(), identity->comment);
      break;
    }
    prev_identity = identity;
  }
  if (rc) {
    fprintf(stderr, "Couldn't continue authentication\n");
  }

  LIBSSH2_CHANNEL *channel = libssh2_channel_open_session(session);
  if (libssh2_channel_exec(channel, command.c_str()) != 0) return "ERROR";

  std::cout << "POSZLO\n";

  std::string buffer(1000, 0);
  std::string log;
  int size;
  while ((size = libssh2_channel_read(channel, buffer.begin().base(), buffer.length())) > 0) {
    log += std::string(buffer.begin(), buffer.begin() + size);
    std::cout << std::string(buffer.begin(), buffer.begin() + size) << "\n";
  }

  if (size == 0)
    std::cout << "OK Player finished.\n" + log;
  else
    std::cout << "ERROR Connection interrupted.\n" + log;

  libssh2_channel_close(channel);
  libssh2_channel_free(channel);
  libssh2_session_disconnect(session, "");
  libssh2_session_free(session);

  return log;
}

void Master::run() {
  unsigned lastPort = port;
  sock.connectServer(port);
  if (lastPort != port) {
    std::cout << port << std::endl;
  }

  std::cerr << sshExec("students.mimuw.edu.pl", "if [ -z \"$SSH_AUTH_SOCK\" ] ; then eval `ssh-agent`; ssh-add fi; ls -l") << "\n";

  Epoll efd{};
  efd.addEvent(sock);
  while (true) {
    std::vector <epoll_event> events = efd.wait(MAX_SOCKETS_PLAYER, MAX_TIME);
    for (epoll_event &event : events) {
      checkListeningSocket(event);
    }
  }
}

Master::~Master() {
  for (TelnetSession *t : telnetSessions) {
    delete t;
  }
}