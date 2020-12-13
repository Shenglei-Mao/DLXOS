#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  unsigned int* addr;
  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  addr = (1<<20) - (1<<12); //shit, second time forget to add brackets and pay 10 min on f**k** debugging
  Printf("testcase2 (%d): trying to access memory where at %d, value is %d\n", getpid(), addr, *addr);
  Printf("COMPARE TO Illegal Access, and the program should generate runtime error\n");
  addr = (1<<20) - (1<<12) - 1;
  Printf("testcase2 (%d): trying to access memory just outside the max virtual address at %d, value is %d\n", getpid(), addr, *addr);

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("testcase2 (%d): Done!\n", getpid());
}
