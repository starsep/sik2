#include "Utility.hpp"
#include "Master.hpp"
#include <libssh/libssh.h>

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

static int verify_knownhost(ssh_session session) {
  int state, hlen;
  unsigned char *hash = NULL;
  char *hexa;
  char buf[10];
  state = ssh_is_server_known(session);
  hlen = ssh_get_pubkey_hash(session, &hash);
  if (hlen < 0)
    return -1;
  switch (state) {
    case SSH_SERVER_KNOWN_OK:
      break; /* ok */
    case SSH_SERVER_KNOWN_CHANGED:
      fprintf(stderr, "Host key for server changed: it is now:\n");
      ssh_print_hexa("Public key hash", hash, hlen);
      fprintf(stderr, "For security reasons, connection will be stopped\n");
      free(hash);
      return -1;
    case SSH_SERVER_FOUND_OTHER:
      fprintf(stderr, "The host key for this server was not found but an other"
          "type of key exists.\n");
      fprintf(stderr, "An attacker might change the default server key to"
          "confuse your client into thinking the key does not exist\n");
      free(hash);
      return -1;
    case SSH_SERVER_FILE_NOT_FOUND:
      fprintf(stderr, "Could not find known host file.\n");
      fprintf(stderr, "If you accept the host key here, the file will be"
          "automatically created.\n");
      /* fallback to SSH_SERVER_NOT_KNOWN behavior */
    case SSH_SERVER_NOT_KNOWN:
      hexa = ssh_get_hexa(hash, hlen);
      fprintf(stderr, "The server is unknown. Do you trust the host key?\n");
      fprintf(stderr, "Public key hash: %s\n", hexa);
      free(hexa);
      if (fgets(buf, sizeof(buf), stdin) == NULL) {
        free(hash);
        return -1;
      }
      if (strncasecmp(buf, "yes", 3) != 0) {
        free(hash);
        return -1;
      }
      if (ssh_write_knownhost(session) < 0) {
        fprintf(stderr, "Error %s\n", strerror(errno));
        free(hash);
        return -1;
      }
      break;
    case SSH_SERVER_ERROR:
      fprintf(stderr, "Error %s", ssh_get_error(session));
      free(hash);
      return -1;
  }
  free(hash);
  return 0;
}

static int show_remote_files(ssh_session session) {
  ssh_channel channel;
  int rc;
  channel = ssh_channel_new(session);
  if (channel == NULL) return SSH_ERROR;
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK) {
    ssh_channel_free(channel);
    return rc;
  }
  rc = ssh_channel_request_exec(channel, "ls -l");
  if (rc != SSH_OK) {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return rc;
  }
  char buffer[256];
  int nbytes;
  nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  while (nbytes > 0) {
    if (fwrite(buffer, 1, nbytes, stdout) != nbytes) {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return SSH_ERROR;
    }
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  }
  if (nbytes < 0) {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_ERROR;
  }
  ssh_channel_send_eof(channel);
  ssh_channel_close(channel);
  ssh_channel_free(channel);
  return SSH_OK;
}

void Master::run() {
  unsigned lastPort = port;
  sock.connectServer(port);
  if (lastPort != port) {
    std::cout << port << std::endl;
  }

  //*

  ssh_session my_ssh_session;
  int rc;
  // Open session and set options
  my_ssh_session = ssh_new();
  if (my_ssh_session == NULL)
    exit(-1);
  ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "students.mimuw.edu.pl");
  // Connect to server
  rc = ssh_connect(my_ssh_session);
  if (rc != SSH_OK) {
    fprintf(stderr, "Error connecting to localhost: %s\n",
            ssh_get_error(my_ssh_session));
    ssh_free(my_ssh_session);
    exit(-1);
  }
  // Verify the server's identity
  // For the source code of verify_knowhost(), check previous example
  if (verify_knownhost(my_ssh_session) < 0) {
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    exit(-1);
  }
  // Authenticate ourselves
  rc = ssh_userauth_agent(my_ssh_session, "fc359081");
  if (rc != SSH_AUTH_SUCCESS) {
    fprintf(stderr, "Error authenticating with password: %s\n",
            ssh_get_error(my_ssh_session));
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    exit(-1);
  }
  show_remote_files(my_ssh_session);
  ssh_disconnect(my_ssh_session);
  ssh_free(my_ssh_session);

  //*

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