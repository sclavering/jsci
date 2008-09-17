#include "prthread.h"

#ifdef _WIN32
#include <windows.h>
int PR_GetCurrentThread() {
  return GetCurrentThreadId()<<1;
}
#else
#include <pthread.h>
int PR_GetCurrentThread() {
  return pthread_self()<<1;
}
#endif
