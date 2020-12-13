#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  missile_code mc;         // Used to access missile codes from mailbox
  mbox_t h_mbox;           // Handle to the mailbox
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

  if (argc != 3) { 
    Printf("Usage: %s <handle_to_mailbox> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mbox = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);

  // Open the mailbox
  if (mbox_open(h_mbox) == MBOX_FAIL) {
    Printf("spawn_me (%d): Could not open the mailbox!\n", getpid());
    Exit();
  }

  // Wait for a message from the mailbox
  if (mbox_recv(h_mbox, sizeof(mc), (void *)&mc) == MBOX_FAIL) {
    Printf("spawn_me (%d): Could not map the virtual address to the memory!\n", getpid());
    Exit();
  }
 
  // Now print a message to show that everything worked
  Printf("spawn_me (%d): Received missile code: %c\n", getpid(), mc.really_important_char);

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("spawn_me (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("spawn_me (%d): Done!\n", getpid());
}
