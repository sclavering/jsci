#include <pthread.h>

void pthread_free(pthread_t *pt) {
  pthread_detach(*pt);
}
