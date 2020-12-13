#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "spawn.h"

void main (int argc, char *argv[])
{
  int numprocs = 0;               // Used to store number of processes to create
  int i;                          // Loop index variable
  cbuffer *cb;            // circular buffer
  uint32 h_mem;                   // Used to hold handle to shared memory page
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  char h_mem_str[10];             // Used as command-line argument to pass mem_handle to new processes
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  lock_t lock;
  char lock_str[10];
  // char full[10]; // 0
  // char empty[10]; //N
  // sem_t empty;
  // sem_t full; 
  // char full_str[10];
  // char empty_str[10];
  cond_t not_full;
  cond_t not_empty;
  char not_full_str[10];
  char not_empty_str[10];


  if (argc != 2) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  numprocs = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("Creating %d processes\n", numprocs);

  // Allocate space for a shared memory page, which is exactly 64KB
  // Note that it doesn't matter how much memory we actually need: we 
  // always get 64KB
  if ((h_mem = shmget()) == 0) {
    Printf("ERROR: could not allocate shared memory page in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  // Map shared memory page into this process's memory space
  // if ((mc = (missile_code *)shmat(h_mem)) == NULL) {
  //   Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
  //   Exit();
  // }

  //Create a circular buffer TODO
  if ((cb = (cbuffer *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
    Exit();
  }
  cb->start = 0;
  cb->end = 0;
  // Put some values in the shared memory, to be read by other processes
  // mc->numprocs = numprocs;
  // mc->really_important_char = 'A';

  // Create Producer TODO

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  
  if ((s_procs_completed = sem_create(-(numprocs * 2 -1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // if ((empty = sem_create(BUFFERSIZE)) == SYNC_FAIL) {
  //   Printf("Bad empty sem_create in "); Printf(argv[0]); Printf("\n");
  //   Exit();
  // }

  // if ((full = sem_create(0)) == SYNC_FAIL) {
  //   Printf("Bad full sem_create in "); Printf(argv[0]); Printf("\n");
  //   Exit();
  // }

  //Create Lock
  if ((lock = lock_create()) == SYNC_FAIL) {
    Printf("ERROR: Not able to create lock"); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((not_full = cond_create(lock)) == SYNC_FAIL) {
    Printf("Bad condition not_full create in "); Printf(argv[0]); Printf("\n");
    Exit(); 
  }

  if ((not_empty = cond_create(lock)) == SYNC_FAIL) {
    Printf("Bad condition not_full create in "); Printf(argv[0]); Printf("\n");
    Exit(); 
  }

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(h_mem, h_mem_str);
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(lock, lock_str);
  // ditoa(empty, empty_str);
  // ditoa(full, full_str);
  ditoa(not_full, not_full_str);
  ditoa(not_empty, not_empty_str);


  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  for(i=0; i<numprocs; i++) {

    process_create(PRODUCER_FILENAME_TO_RUN, h_mem_str, s_procs_completed_str, lock_str, not_full_str, not_empty_str, NULL);

    Printf("Producer %d created\n", i);
  };

  for(i=0; i<numprocs; i++) {

    process_create(CONSUMER_FILENAME_TO_RUN, h_mem_str, s_procs_completed_str, lock_str, not_full_str, not_empty_str, NULL);

    Printf("Consumer %d created\n", i);
  }

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }


  Printf("All other processes completed, exiting main process.\n");

}