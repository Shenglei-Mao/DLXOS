#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{       
  uint32 h_mem;            
  sem_t s_procs_completed;
  uint32 num_n2;
  uint32 num_h2o;
  sem_t n;
  sem_t o2;
  int i;
  uint32 stop_point;

  if (argc != 7) { 
    Printf("Usage: "); Printf(argv[0]); Printf("Augument number wrong in reaction3\n"); 
    Exit();
  } 
  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  num_n2 = dstrtol(argv[3], NULL, 10);
  num_h2o = dstrtol(argv[4], NULL, 10);
  n = dstrtol(argv[5], NULL, 10);
  o2 = dstrtol(argv[6], NULL, 10);
  stop_point = min(num_n2*2, num_h2o/2);
  i = 0;
  while (i < stop_point) {
    if (sem_wait(n) != SYNC_SUCCESS) {
      Printf("Bad semaphore wait (%d) in reaction3 ", n); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }
    if (sem_wait(o2) != SYNC_SUCCESS) {
      Printf("Bad semaphore wait (%d) in reaction3 ", o2); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }
    Printf("An NO2 molecule is created\n");
    i += 1;
  }
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}