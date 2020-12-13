#include "usertraps.h"
#include "misc.h"

int init_page = -1;

void helper() {
  //base case
  unsigned int a = 0;
  int current_page = -1;
  // Why we can't use this?
  //unsigned int* addr = (1<<20) - (1<<12) * 16;
  current_page = (unsigned int)(&a) >> 12;

  if (current_page < init_page - 20) {
    Printf("local variable is accessing address = %d at page = %d, testcase passed\n", &a, current_page);
    return;
  }
  // } else {
  //   Printf("local variable is accessing address = %d at page = %d, continue\n", &a, current_page);
  // }
  return helper();
}

void main (int argc, char *argv[])
{
  unsigned int input = 10;
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

  init_page = (unsigned int)(&input) >> 12;
  Printf("Input at main stack at addr = %d, on page = %d\n", &input, init_page);

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Convert the command-line strings into integers for use as handles
  helper();

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("testcase2 (%d): Done!\n", getpid());
}