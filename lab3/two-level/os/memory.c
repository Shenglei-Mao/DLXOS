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
static L2 l2table[MEM_GLOBAL_L2TABLE_SIZE];
static uint32 freemap[MEM_MAX_PAGES / 32];

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
  int j;
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


  dbprintf ('m', "Start to init global L2 table: .......\n");
  for (i = 0; i < MEM_GLOBAL_L2TABLE_SIZE; i++) {
    l2table[i].use = 0;
    for (j = 0; j < MEM_L2TABLE_SIZE; j++) {
      l2table[i].table[j] = 0;
    }
  }
  dbprintf ('m', "Global L2 table init done: .......\n");
}


//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//  Translate a user address (in the process referenced by pcb)
//  into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
//@Michael TODO
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
  // uint32 page_offset;
  // uint32 page_index;
  // uint32 pagetable_entry;
  
  // page_offset = addr & (MEM_ADDRESS_OFFSET_MASK);
  // page_index = addr >> MEM_L1FIELD_FIRST_BITNUM;
  // pagetable_entry = pcb->pagetable[page_index];

  // if((pagetable_entry & MEM_PTE_VALID) == 0) {
  //   return MemoryPageFaultHandler(pcb);
  // }
  // return (uint32)((pagetable_entry&(~MEM_ADDRESS_OFFSET_MASK)) + page_offset);


  uint32 page_offset;
  uint32 l1_page_idx;
  uint32 l2_page_idx;
  uint32* ptr;
  uint32 pagetable_entry;
  // printf("inside MemoryTranslateUserToSystem\n");
  page_offset = addr & (MEM_ADDRESS_OFFSET_MASK);
  l1_page_idx = addr >> MEM_L1FIELD_FIRST_BITNUM;
  l2_page_idx = (addr >> MEM_L2FIELD_FIRST_BITNUM) & MEM_L2_MASK;
  if (pcb->l1_table[l1_page_idx] == NULL) {
    return MemoryPageFaultHandler(pcb); //Should I return 0 or MEM_FAIL? Based on caller, more like 0
  }
  ptr = pcb->l1_table[l1_page_idx];
  pagetable_entry = ptr[l2_page_idx];

  if((pagetable_entry & MEM_PTE_VALID) == 0) {
    return MemoryPageFaultHandler(pcb);
  }
  // printf("physical address is%d\n", (uint32)((pagetable_entry&(~MEM_ADDRESS_OFFSET_MASK)) + page_offset));
  dbprintf("m", "physical address is: %d\n", (uint32)((pagetable_entry&(~MEM_ADDRESS_OFFSET_MASK)) + page_offset));
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
//@Michael TODO
int MemoryPageFaultHandler(PCB *pcb) {

  uint32 vpagenum;
  uint32 stackpagenum;
  int ppagenum;
  uint32 addr;
  int l1_page;
  int l2_page;
  uint32* ptr;
  //get the virtual address
  addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
  printf("memory fault occurs\n");
  dbprintf('m', "Student - MemoryPageFaultHandler: Page fault adress is: %x\n", addr); 
  vpagenum = addr >> MEM_L2FIELD_FIRST_BITNUM;
  stackpagenum = (pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]-8) >> MEM_L2FIELD_FIRST_BITNUM;
  dbprintf('m', "Student - MemoryPageFaultHandler: vpagenum is: %d, and stackpagenum is: %d\n", vpagenum, stackpagenum); 
  // segfault if the faulting address is not part of the stack
  if (vpagenum < stackpagenum) { 
    dbprintf('m', "addr = %x\nsp = %x\n", addr, pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]);
    printf("FATAL ERROR (%d): segmentation fault at page address %x\n", GetCurrentPid(pcb), addr);
    ProcessKill(); 
    return MEM_FAIL; 
  }
  //Now assign page
  l1_page = addr >> MEM_L1FIELD_FIRST_BITNUM;
  l2_page = vpagenum & MEM_L2_MASK;
  ppagenum = MemoryAllocPage();
  if (pcb->l1_table[l1_page] == NULL) {
    pcb->l1_table[l1_page] = AllocL2Page();
    ptr = pcb->l1_table[l1_page];
    ptr[l2_page] = MemorySetupPte(ppagenum);
  } else {
    ptr = pcb->l1_table[l1_page];
    ptr[l2_page] = MemorySetupPte(ppagenum);
  }
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
  if(result > ((MemoryGetSize()-1) >> MEM_L2FIELD_FIRST_BITNUM)) return MEM_FAIL;
  //Record that this page is allocated
  freemap[page_index] = freemap[page_index] & invert(MEM_FREEMAP_MASK >> bit_offset);
  dbprintf('m', "Return physical page index %d\n", result); 
  return result;
}


//------------------------------------------------------------
// @Michael
// Convert physical page index to pte
// @Parameter: unit32(page) - physical page index
// @Return: uint32 - pte
//------------------------------------------------------------
uint32 MemorySetupPte (uint32 page) {
  dbprintf('m', "MemoryAllocPte (%d): function started\n", GetCurrentPid());
  //return -1;
  // uint32 pte;
  // pte = (page << MEM_L1FIELD_FIRST_BITNUM) | MEM_PTE_VALID;
  // return pte;
  return (uint32)((page << MEM_L2FIELD_FIRST_BITNUM) | MEM_PTE_VALID);
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
  dbprintf('m', "MemoryFreePage (%d): function started\n", GetCurrentPid());
  if (page == 0){
    return;
  }
  //Find the index and offset in freemap according to the page number, then set the value to 1
  page_index = page / 32;
  bit_offset = page % 32;
  // freemap[page_index] = freemap[page_index] | (1 << bit_offset); //possible bug?
  freemap[page_index] = freemap[page_index] | (MEM_FREEMAP_MASK >> bit_offset); //possible bug?
}

//------------------------------------------------------------
// @Michael
// Mark L2page in use in global l2 page table pool
// @Parameter: void
// @Return: physical address of L2 page
//------------------------------------------------------------
//Return Actual Physical Address of L1 page
uint32 AllocL2Page(void) { 
  int i;
  for (i = 0; i < MEM_GLOBAL_L2TABLE_SIZE; i++) {
    if (l2table[i].use == 0) {
      l2table[i].use = 1;
      return (&l2table[i]);
    }
  }
  return MEM_FAIL;
}

void FreeL2Page(uint32 l2page) {
  L2* ptr = (L2*)l2page;
  ptr->use = 0;
  return;
}

