#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int numprocs = 0;               // Used to store number of processes to create
  int i;                          // Loop index variable
  missile_code mc;                // Used as message for mailbox
  mbox_t h_mbox;                  // Used to hold handle to mailbox
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  char h_mbox_str[10];            // Used as command-line argument to pass mem_handle to new processes
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes

  if (argc != 2) {
    Printf("Usage: %s <number of processes to create\n", argv[0]);
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  numprocs = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("makeprocs (%d): Creating %d processes\n", getpid(), numprocs);

  // Allocate space for a mailbox
  if ((h_mbox = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }

  // Open mailbox to prevent deallocation
  if (mbox_open(h_mbox) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), h_mbox);
    Exit();
  }

  // Put some values in the mc structure to send as a message
  mc.numprocs = numprocs;
  mc.really_important_char = 'A';

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  if ((s_procs_completed = sem_create(-(numprocs-1))) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(h_mbox, h_mbox_str);
  ditoa(s_procs_completed, s_procs_completed_str);

  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  for(i=0; i<numprocs; i++) {
    process_create(FILENAME_TO_RUN, 0, 0, h_mbox_str, s_procs_completed_str, NULL);
    Printf("makeprocs (%d): Process %d created\n", getpid(), i);
  }

  // Send the missile code messages
  for (i=0; i<numprocs; i++) {
    if (mbox_send(h_mbox, sizeof(missile_code), (void *)&mc) == MBOX_FAIL) {
      Printf("Could not send message to mailbox %d in %s (%d)\n", h_mbox, argv[0], getpid());
      Exit();
    }
    Printf("makeprocs (%d): Sent message %d\n", getpid(), i);
  }

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());
}
