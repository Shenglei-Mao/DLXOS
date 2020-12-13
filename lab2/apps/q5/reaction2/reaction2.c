#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{       
  uint32 h_mem;            
  sem_t s_procs_completed;
  uint32 num_h2o;
  sem_t h2o;
  sem_t o2;
  int i;
  uint32 stop_point;
  
  if (argc != 6) { 
    Printf("Usage: "); Printf(argv[0]); Printf("Augument number wrong in producer1\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  num_h2o = dstrtol(argv[3], NULL, 10);
  h2o = dstrtol(argv[4], NULL, 10);
  o2 = dstrtol(argv[5], NULL, 10);
  stop_point = num_h2o / 2;

  i = 0;
  while (i < stop_point) {
    if (sem_wait(h2o) != SYNC_SUCCESS) {
      Printf("Bad semaphore wait (%d) in reaction2 ", h2o); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }
    if (sem_wait(h2o) != SYNC_SUCCESS) {
      Printf("Bad semaphore wait (%d) in reaction2 ", h2o); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }
    if(sem_signal(o2) != SYNC_SUCCESS) {
      Printf("Bad semaphore signal (%d) in reaction2", o2); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }
    Printf("An H2 molecule is created\n");
    Printf("An H2 molecule is created\n");
    Printf("An O2 molecule is created\n");
    i += 1;
  }
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}