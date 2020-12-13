#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Now access memory beyond the maximum virtual address.
  // int - 4 byte access, just in case, access -8 from max virtual adddress
  Printf("testcase2 (%d): trying to access memory where at %d, value is %d\n", getpid(), (1<<20) - 8, *(unsigned int*)((1<<20) - 8));
  Printf("COMPARE TO Illegal Access, and the program should generate runtime error\n");
  // Actually start from (1<<20) - 3 is already invalid if we use int pointer
  Printf("testcase2 (%d): trying to access memory just outside the max virtual address at %d, value is %d\n", getpid(), (1<<20), *(unsigned int*)(1<<20));

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("testcase2 (%d): Done!\n", getpid());
}
