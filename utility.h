#ifndef SIK2_UTILITY_H
#define SIK2_UTILITY_H

#include <iostream>
#include <tuple>

using File = int;

const unsigned INVALID_PORT = 0;
const unsigned MIN_PORT = 1;
const unsigned MAX_PORT = (1 << 16) - 1;
const std::string stdOut = "-";
const std::string YES = "yes";
const std::string NO = "no";

const File stdOutFile = 1;

enum class ExitCode {
  Ok = 0,
  InvalidArguments = 1,
  SystemError = 1,
};

void _exit(ExitCode);
unsigned getPort(const char *);
unsigned getFreePort();
File _open(const char *, int);

#endif // SIK2_UTILITY_H