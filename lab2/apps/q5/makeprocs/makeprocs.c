#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "spawn.h"

void main (int argc, char *argv[])
{
  int num_n2 = 0;
  int num_h2o = 0;
  int numprocs = 5;
  uint32 h_mem;                   // Used to hold handle to shared memory page
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  char h_mem_str[10];             // Used as command-line argument to pass mem_handle to new processes
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  //n2, n, h2o, o2, no2, h2 not needed
  sem_t n2;
  sem_t n;
  sem_t h2o;
  sem_t o2;
  char n2_str[10];
  char n_str[10];
  char h2o_str[10];
  char o2_str[10];
  char num_n2_str[10];
  char num_h2o_str[10];

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }

  num_n2 = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("Number of N2 molecule %d.\n", num_n2);

  num_h2o = dstrtol(argv[2], NULL, 10); // the "10" means base 10
  Printf("Number of H2O molecule %d.\n", num_h2o);

  // Allocate space for a shared memory page, which is exactly 64KB
  // Note that it doesn't matter how much memory we actually need: we 
  // always get 64KB
  if ((h_mem = shmget()) == 0) {
    Printf("ERROR: could not allocate shared memory page in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  if ((s_procs_completed = sem_create(-(numprocs-1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((n2 = sem_create(INITSIZE)) == SYNC_FAIL) {
    Printf("Bad empty sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((n = sem_create(INITSIZE)) == SYNC_FAIL) {
    Printf("Bad empty sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((h2o = sem_create(INITSIZE)) == SYNC_FAIL) {
    Printf("Bad empty sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((o2 = sem_create(INITSIZE)) == SYNC_FAIL) {
    Printf("Bad empty sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(h_mem, h_mem_str);
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(num_n2, num_n2_str);
  ditoa(num_h2o, num_h2o_str);
  ditoa(n2, n2_str);
  ditoa(n, n_str);
  ditoa(h2o, h2o_str);
  ditoa(o2, o2_str);

  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  process_create(PRODUCER1_FILENAME_TO_RUN, h_mem_str, s_procs_completed_str, num_n2_str, n2_str, NULL);
  Printf("Producer1 created\n");
  process_create(PRODUCER2_FILENAME_TO_RUN, h_mem_str, s_procs_completed_str, num_h2o_str, h2o_str, NULL);
  Printf("Producer2 created\n");
  process_create(REACTION1_FILENAME_TO_RUN, h_mem_str, s_procs_completed_str, num_n2_str, n2_str, n_str, NULL);
  Printf("Reaction1 created\n");
  process_create(REACTION2_FILENAME_TO_RUN, h_mem_str, s_procs_completed_str, num_h2o_str, h2o_str, o2_str, NULL);
  Printf("Reaction2 created\n");
  process_create(REACTION3_FILENAME_TO_RUN, h_mem_str, s_procs_completed_str, num_n2_str, num_h2o_str, n_str, o2_str, NULL);
  Printf("Reaction3 created\n");

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }

  Printf("All other processes completed, exiting main process.\n");
}
