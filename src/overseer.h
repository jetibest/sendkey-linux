#ifndef OVERSEER_H
#define OVERSEER_H

extern int running;

extern pthread_mutex_t lock;// = PTHREAD_MUTEX_INITIALIZER;
extern pthread_cond_t cond;// = PTHREAD_COND_INITIALIZER;

#endif // OVERSEER_H
