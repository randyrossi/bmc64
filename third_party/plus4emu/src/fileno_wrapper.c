#include <stdio.h>

// Wrapper around POSIX fileno to get around g++ compile issue.
// Included from tape.cpp
int plus4_fileno(FILE* fp) {
  return fileno(fp);
}
