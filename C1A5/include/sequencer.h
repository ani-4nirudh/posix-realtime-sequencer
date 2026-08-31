#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <pthread.h>

typedef struct {
  pthread_t thread;
  pthread_attr_t attr;

  int attr_initialised;
  int thread_created;
} Sequencer;

int sequencer_sems_init(void);
int sequencer_sems_destroy(void);

int sequencer_init(Sequencer *p_seq);
int sequencer_attr_destroy(Sequencer *p_seq);
int sequencer_start(Sequencer *p_seq);
int sequencer_join(Sequencer *p_seq);

#endif /* !SEQUENCER_H
#define */
