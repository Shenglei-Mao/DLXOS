#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  cbuffer *cb;        
  uint32 h_mem;            
  sem_t s_procs_completed;
  lock_t lock;
  char output;
  int i;
  cond_t not_full;
  cond_t not_empty;


  if (argc != 6) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore> <handle to lock>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  lock = dstrtol(argv[3], NULL, 10);
  not_full = dstrtol(argv[4], NULL, 10);
  not_empty = dstrtol(argv[5], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((cb = (cbuffer *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  // lock acquire
  // if (lock_acquire(lock) != SYNC_SUCCESS) {
  //   Printf("ERROR: lock acquire fail"); Printf(argv[0]); Printf(", exiting...\n");
  //   Exit();
  // }

  // output from buffer
  i = 0;
  while (i < 11) {
    // if buffer is empty, wait without decreament i

    if (lock_acquire(lock) != SYNC_SUCCESS) {
      Printf("ERROR: lock acquire fail"); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }

    if (cb->start - cb->end == 0) {
      if (cond_wait(not_empty) != SYNC_SUCCESS) {
        Printf("ERROR: not empty condition wait fail"); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
      }
    }
    
    output = cb->store[cb->end];
    cb->end++;
    cb->end = cb->end % BUFFERSIZE;
    Printf("Consumer %d removed: %c\n", Getpid(), output);

    if (cond_signal(not_full) != SYNC_SUCCESS) {
      Printf("ERROR: not full condition signal fail"); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }

    if (lock_release(lock) != SYNC_SUCCESS) {
      Printf("ERROR: lock release fail"); Printf(argv[0]); Printf(", exiting...\n");
      Exit();
    }

    i += 1;

  }
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
