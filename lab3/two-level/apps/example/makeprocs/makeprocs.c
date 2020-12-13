#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"
#define TESTCASE2 "test2.dlx.obj"
#define TESTCASE3 "test3.dlx.obj"
#define TESTCASE4 "test4.dlx.obj"
#define HELLO_WORLD_COUNT "hello_world_count.dlx.obj"

void main (int argc, char *argv[])
{
  int testcase = 0;             // Used to store number of processes to create
  int i;                               // Loop index variable
  sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes

  if (argc != 2) {
    Printf("Usage: %s <testcase selection, please check readme for a furthur explanation>\n", argv[0]);
    Exit();
  }

  //Get user selection for which testcase to run
  testcase = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("makeprocs (%d): Start to running testcase %d\n", getpid(), testcase);

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.
  if (testcase != 6) {
    if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
      Printf("makeprocs (%d): Bad sem_create\n", getpid());
      Exit();
    }
  }

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);

  if (testcase == 1) {
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Running Testcase%d: print \"Hello World\" and exit.\n", getpid(), testcase);
    process_create(HELLO_WORLD, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) { //testcase2: Access memory beyond the maximum virtual address.
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): testcase%d done.\n", getpid(), testcase);
  } else if (testcase == 2) {
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Running Testcase%d: Access memory beyond the maximum virtual address.\n", getpid(), testcase);
    process_create(TESTCASE2, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) { //testcase2: Access memory beyond the maximum virtual address.
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }   
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): testcase%d done.\n", getpid(), testcase);

  } else if (testcase == 3) {
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Running Testcase%d: Access memory inside the virtual address space, but outside of currently allocated pages.\n", getpid(), testcase);
    process_create(TESTCASE3, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) { //testcase2: Access memory beyond the maximum virtual address.
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): testcase%d done.\n", getpid(), testcase);
  } else if (testcase == 4) {
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Running Testcase%d: Cause the user function call stack to grow larger than 1 page.\n", getpid(), testcase);
    process_create(TESTCASE4, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) { //testcase2: Access memory beyond the maximum virtual address.
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): testcase%d done.\n", getpid(), testcase);
  } else if (testcase == 5) {
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Running Testcase%d: calling hello world 100 times sequentially.\n", getpid(), testcase);
    for(i=0; i<100; i++) {
      Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
      process_create(HELLO_WORLD, s_procs_completed_str, NULL);
      if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
        Exit();
      }
    }
  } else if (testcase == 6) {
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Running Testcase%d: test concurrent 30 processes.\n", getpid(), testcase);
    if ((s_procs_completed = sem_create(-(30 - 1))) == SYNC_FAIL) {
      Printf("makeprocs (%d): Bad sem_create\n", getpid());
      Exit();
    }
    for(i=0; i<30; i++) {
      Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
      process_create(HELLO_WORLD_COUNT, s_procs_completed_str, NULL);
    }
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
        Exit();
    }
    Printf("Testcase%d passed\n", testcase);
  }
 

  // Create Hello World processes
  // Printf("-------------------------------------------------------------------------------------\n");
  // Printf("makeprocs (%d): Creating %d hello world's in a row, but only one runs at a time\n", getpid(), num_hello_world);
  // for(i=0; i<num_hello_world; i++) {
  //   Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
  //   process_create(HELLO_WORLD, s_procs_completed_str, NULL);
  //   if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
  //     Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
  //     Exit();
  //   }
  // }

  // Printf("-------------------------------------------------------------------------------------\n");
  // Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
