#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{       
  uint32 h_mem;            
  sem_t s_procs_completed;
  sem_t n2;
  uint32 num_n2;
  int i;
  
  if (argc != 5) { 
    Printf("Usage: "); Printf(argv[0]); Printf("Augument number wrong in producer1\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  num_n2 = dstrtol(argv[3], NULL, 10);
  n2 = dstrtol(argv[4], NULL, 10);

  i = 0;
  while (i < num_n2) {
    if(sem_signal(n2) != SYNC_SUCCESS) {
      Printf("Bad semaphore signal (%d) in producer1", n2); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }
    Printf("An N2 molecule is created\n");
    i += 1;
  }
}