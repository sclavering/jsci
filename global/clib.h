#pragma JSEXT dl main  // Functions are found within main binary executable

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <pwd.h>
#include <sys/select.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/times.h>
#include <sys/time.h>
//#include <time.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <utime.h>
#include <sched.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

// These functions need a wrapper, as they are sometimes inlined

int call_stat(const char *file_name, struct stat *buf);
int call_fstat(int filedes, struct stat *buf);
int call_lstat(const char *file_name, struct stat *buf);
int call_mknod(const char *pathname, mode_t mode, dev_t dev);
