#include <sys/stat.h>

extern "C"
int call_stat(const char *file_name, struct stat *buf) {
  return stat(file_name, buf);
}

extern "C"
int call_fstat(int filedes, struct stat *buf) {
  return fstat(filedes, buf);
}

extern "C"
int call_lstat(const char *file_name, struct stat *buf) {
  return lstat(file_name, buf);
}

extern "C"
int call_mknod(const char *pathname, mode_t mode, dev_t dev) {
  return mknod(pathname, mode, dev);
}
