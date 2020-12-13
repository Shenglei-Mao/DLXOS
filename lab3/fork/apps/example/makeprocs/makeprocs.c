#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"

void main (int argc, char *argv[])
{
  int child_pid;                               // child process PID

  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): Test fork \n", getpid());

  Printf ("The main program process ID is %d\n", (int) getpid ());
  child_pid = fork();
  Printf ("Child PID is %d\n", child_pid);
  if (child_pid != 0) {
    print_pte((int) getpid ());
    //Printf ("This is the parent process, with id %d\n", (int) getpid ());
    //Printf ("The child's process ID is %d\n", child_pid);
  } else {
    // int local = 5;
    // Printf ("Local = %d\n", local);
    print_pte((int) getpid ());
    //Printf ("This is the child process, with id %d\n", (int) getpid ());
  }

  // if (child_pid != 0) {
  //   printf("Hi, I'm the parent.\n");
  // } else {
  //   exec("new_program_execuatable.dlx.obj", arg1, arg2);
  //   printf("This line will never run\n");
  // }

  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
