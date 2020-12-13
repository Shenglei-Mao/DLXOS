//
//  memory.c
//
//  Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

//@Michael: Do explicit type conversion for all to avoid unnecessary mistakes
#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page
// static uint32 freemap[/*size*/];
static uint32 freemap[MEM_MAX_PAGES / 32];
static uint32 phypage_referencecounter[MEM_MAX_PAGES];  //PCW: initial the global array of reference counters that are associated with each physical page.
// static uint32 pagestart;
// static int nfreepages;
// static int freemapmax;

//----------------------------------------------------------------------
//
//  This silliness is required because the compiler believes that
//  it can invert a number by subtracting it from zero and subtracting
//  an additional 1.  This works unless you try to negate 0x80000000,
//  which causes an overflow when subtracted from 0.  Simply
//  trying to do an XOR with 0xffffffff results in the same code
//  being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//  MemoryGetSize
//
//  Return the total size of memory in the simulator.  This is
//  available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}


//----------------------------------------------------------------------
//
//  MemoryInitModule
//
//  Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------
//@Michael: 0 - already in-used, 1 - available
void MemoryModuleInit() {
  // mark 0 for pages used by OS and 1 for others
  int i;
  int num_pages;
  int pages_to_shift;

  for (i = 0; i < MEM_MAX_PAGES / 32; i++) {
    freemap[i] = -1;
  }
  //TODO: Check lastosaddress physical address or pages. Assume physical address
  dbprintf ('m', "Last OS address is: %d.\n", lastosaddress);

  //TODO: Optimizing
  if (lastosaddress % MEM_PAGESIZE == 0) {
    num_pages = lastosaddress / MEM_PAGESIZE;
  } else {
    num_pages = lastosaddress / MEM_PAGESIZE + 1;
  }
  
  //PCW: Mark all OS occupied pages' reference counter as 1
  for (i=0; i < num_pages; i++){
    phypage_referencecounter[num_pages] = 1;
  }

  if (num_pages % 32 == 0) {
    pages_to_shift = num_pages / 32;
  } else {
    pages_to_shift = num_pages / 32 + 1;
  }

  for (i = 0; i < pages_to_shift; i++) {
    if (num_pages >= 32) {
      freemap[i] = 0;
    } else {
      freemap[i] = freemap[i] >> num_pages;
    }
    num_pages = num_pages - 32;
  }

  dbprintf ('m', "freemap information: .......\n");
  for (i = 0; i < MEM_MAX_PAGES / 32; i++) {
    dbprintf ('m', "freemap at index%d have value of %d: \n", i, freemap[i]); 
  }
}


//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//  Translate a user address (in the process referenced by pcb)
//  into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
//@Michael
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
  uint32 page_offset;
  uint32 page_index;
  uint32 pagetable_entry;
  
  page_offset = addr & (MEM_ADDRESS_OFFSET_MASK);
  page_index = addr >> MEM_L1FIELD_FIRST_BITNUM;
  pagetable_entry = pcb->pagetable[page_index];

  if((pagetable_entry & MEM_PTE_VALID) == 0) {
    return MemoryPageFaultHandler(pcb);
  }
  return (uint32)((pagetable_entry&(~MEM_ADDRESS_OFFSET_MASK)) + page_offset);
}


//----------------------------------------------------------------------
//
//  MemoryMoveBetweenSpaces
//
//  Copy data between user and system spaces.  This is done page by
//  page by:
//  * Translating the user address into system space.
//  * Copying all of the data in that page
//  * Repeating until all of the data is copied.
//  A positive direction means the copy goes from system to user
//  space; negative direction means the copy goes from user to system
//  space.
//
//  This routine returns the number of bytes copied.  Note that this
//  may be less than the number requested if there were unmapped pages
//  in the user range.  If this happens, the copy stops at the
//  first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int   bytesCopied = 0;  // Running counter
  int   bytesToCopy;      // Used to compute number of bytes left in page to be copied

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);

    // If we could not translate address, exit now
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);
    
    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//  These two routines copy data between user and system spaces.
//  They call a common routine to do the copying; the only difference
//  between the calls is the actual call to do the copying.  Everything
//  else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference. 
// Feel free to edit.
//---------------------------------------------------------------------
//@Michael
int MemoryPageFaultHandler(PCB *pcb) {
  uint32 vpagenum;
  uint32 stackpagenum;
  int ppagenum;
  uint32 addr;
  addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
  dbprintf('m', "Student - MemoryPageFaultHandler: Page fault adress is: %x\n", addr); 
  vpagenum = addr >> MEM_L1FIELD_FIRST_BITNUM;
  stackpagenum = pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER] >> MEM_L1FIELD_FIRST_BITNUM;
  dbprintf('m', "Student - MemoryPageFaultHandler: vpagenum is: %d, and stackpagenum is: %d\n", vpagenum, stackpagenum); 
  // segfault if the faulting address is not part of the stack
  if (vpagenum < stackpagenum) { 
     dbprintf('m', "addr = %x\nsp = %x\n", addr, pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]);
     printf("FATAL ERROR (%d): segmentation fault at page address %x\n", GetCurrentPid(pcb), addr);
     ProcessKill(); 
     return MEM_FAIL; 
   }
   ppagenum = MemoryAllocPage(); 
   pcb->pagetable[vpagenum] = MemorySetupPte(ppagenum); 
   dbprintf('m', "Returning from page fault handler\n"); 
   return MEM_SUCCESS; 
}


//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

//------------------------------------------------------------
// @Michael
// Find first avaliable page in the free map (linear scan or binary search), then mark page in use in free map
// @Parameter: void
// @Return: int - physical page index start from 0, -1 if no more physical pages avaliable
//------------------------------------------------------------
int MemoryAllocPage(void) {
  //return -1;
  //0 - already in-used, 1 - available
  uint32 bit_offset;
  uint32 page_index;
  int result;

  page_index = 0;
  while(freemap[page_index] == 0){
    page_index +=1;
    if (page_index >= (MEM_MAX_PAGES>>5)){
      return MEM_FAIL;
    } 
  } 
  bit_offset = 0;
  while((freemap[page_index] & (MEM_FREEMAP_MASK >> bit_offset)) == 0){
    bit_offset++;
  }
  
  //Calculate the physical page index
  result = (page_index * 32) + bit_offset;
  phypage_referencecounter[result] = 1; //PCW: record the reference counter by 1 when allocate the page
  if(result > ((MemoryGetSize()-1) >> MEM_L1FIELD_FIRST_BITNUM)) return MEM_FAIL;
  //Record that this page is allocated
  freemap[page_index] = freemap[page_index] & invert(MEM_FREEMAP_MASK >> bit_offset);
  dbprintf('m', "Return physical page index %d\n", result); 
  dbprintf('m', "PID (%d) Phypage (%d) ReferenceCounter (%d): \n", GetCurrentPid(), result, phypage_referencecounter[result]);
  return result;
}


//------------------------------------------------------------
// @Michael
// Convert physical page to pte
// @Parameter: unit32(page) - physical page index
// @Return: uint32 - pte
//------------------------------------------------------------
uint32 MemorySetupPte (uint32 page) {
  dbprintf('m', "MemoryAllocPte (%d): function started\n", GetCurrentPid());
  //return -1;
  // uint32 pte;
  // pte = (page << MEM_L1FIELD_FIRST_BITNUM) | MEM_PTE_VALID;
  // return pte;
  return (uint32)((page << MEM_L1FIELD_FIRST_BITNUM) | MEM_PTE_VALID);
}


//------------------------------------------------------------
// @Michael
// Mark page available in free map
// @Parameter: unit32(page) - physical page index
// @Return: void
//------------------------------------------------------------
void MemoryFreePage(uint32 page) {
  // 0 - already in-used, 1 - available
  uint32 page_index;
  uint32 bit_offset;
  dbprintf('m', "PID (%d) MemoryFreePage: function started\n", GetCurrentPid());
  if (phypage_referencecounter[page] >= 1){
    phypage_referencecounter[page] -= 1; //PCW: decrementing the reference counter by 1 
  }
  dbprintf('m', "PID (%d) Phypage (%d) ReferenceCounter (%d): \n", GetCurrentPid(), page, phypage_referencecounter[page]);
  //Return if the reference counter still greater than 1 which means there's still some processes or process using this page
  if (phypage_referencecounter[page] > 0){
    return;
  }
  //Find the index and offset in freemap according to the page number, then set the value to 1
  page_index = page / 32;
  bit_offset = page % 32;
  freemap[page_index] = freemap[page_index] | (1 << bit_offset);
}

//------------------------------------------------------------
// @PCW
// Incrementing the reference counter by 1 when a new process share this pyhsical page
// @Parameter: unit32(page) - physical page index
// @Return: void
//------------------------------------------------------------
void MemorySharePage(uint32 page) {
  phypage_referencecounter[page] += 1; //PCW: incrementing the reference counter by 1
  dbprintf('m', "PID (%d) Phypage (%d) ReferenceCounter (%d): \n", GetCurrentPid(), page, phypage_referencecounter[page]);
  return;
}

//------------------------------------------------------------
// @PCW
// Get the reference counter of a pyhsical page
// @Parameter: unit32(page) - physical page index
// @Return: void
//------------------------------------------------------------
void MemoryGetPageRC(uint32 page) {
  dbprintf('m', "PID (%d) Phypage (%d) ReferenceCounter (%d): \n", GetCurrentPid(), page, phypage_referencecounter[page]);
  return;
}

//------------------------------------------------------------
// @PCW
// Copy-on-write ROP_ACCESS handler
// @Parameter: unit32(page) - physical page index
// @Return: void
//------------------------------------------------------------
void MemoryRopAccessHandler(PCB *pcb) {
  uint32 addr;
  uint32 vpagenum;
  uint32 ppagenum;
  uint32 newpage;
  
  //The page which cause the exception will be stored in the PROCESS_STACK_FAULT register in the currentSavedFrame of the PCB
  addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
  dbprintf('m', "Student - MemoryRopAccessHandler: RopAccess fault adress is: %x\n", addr); 
  vpagenum = (addr >> MEM_L1FIELD_FIRST_BITNUM);
  ppagenum = ((pcb->pagetable[vpagenum])/MEM_PAGESIZE);
  dbprintf('m', "Student - MemoryRopAccessHandler: vpagenum is: %d, and ppagenum is: %d\n", vpagenum, ppagenum); 
  // Copy-on-write happens when the reference counter greater than 1
  if (phypage_referencecounter[ppagenum] > 1) {
    dbprintf('m', "Student - MemoryRopAccessHandler: reference counter is: %d\n", phypage_referencecounter[ppagenum]); 
    //Allocate a new page
    newpage = MemoryAllocPage();
    //The current page copy byte-by-byte to a new page
    bcopy((char *)addr,(char *)(newpage * MEM_PAGESIZE),MEM_PAGESIZE);
    //The corresponding PTE of the current process is replaced with the new PTE,
    pcb->pagetable[vpagenum] = MemorySetupPte(newpage);
    //Remember to decrement the reference counter by 1 after the copy
    phypage_referencecounter[ppagenum] -= 1;
  }
  //If there is exactly one process using this page, it should be simply marked as read/write.
  else if (phypage_referencecounter[ppagenum] == 1) {
    pcb->pagetable[vpagenum] &= ~(MEM_PTE_READONLY);
  }
}