#include "clib#__unix__.h"

int call_stat(const char *file_name, struct stat *buf) {
  return stat(file_name, buf);
}

int call_fstat(int filedes, struct stat *buf) {
  return fstat(filedes, buf);
}

int call_lstat(const char *file_name, struct stat *buf) {
  return lstat(file_name, buf);
}

int call_mknod(const char *pathname, mode_t mode, dev_t dev) {
  return mknod(pathname, mode, dev);
}

void call_FD_CLR(int fd, fd_set *set) {
  FD_CLR(fd, set);
}

int call_FD_ISSET(int fd, fd_set *set) {
  return FD_ISSET(fd, set);
}

void call_FD_SET(int fd, fd_set *set) {
  FD_SET(fd, set);
}

void call_FD_ZERO(fd_set *set) {
  FD_ZERO(set);
}
