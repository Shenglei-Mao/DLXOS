#include "usertraps.h"
#include "misc.h"

#define LARGE_NUMBER 65535
void main (int argc, char *argv[])
{
  unsigned int i;
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Now print a message to show that everything worked
  Printf("hello_world (%d): Hello world!\n", getpid());
  Printf("Start Counting to a large number\n");

  for (i= 0; i < LARGE_NUMBER; i++) {}

  Printf("hello_world (%d): END\n", getpid());

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("hello_world (%d): Done!\n", getpid());
}
