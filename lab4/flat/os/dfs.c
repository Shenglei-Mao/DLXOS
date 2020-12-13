#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"

//TODO: Check if need to be configurable
static dfs_inode inodes[DFS_INODE_MAX_NUM]; // all inodes
static lock_t lock_inodes;
static dfs_superblock sb; // superblock
static uint32 fbv[DFS_FBV_MAX_NUM_WORDS]; // Free block vector
static lock_t lock_fbv;

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }


// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

// STUDENT: put your file system level functions below.
// Some skeletons are provided. You can implement additional functions.

// Note: function start with Michael (Yes) is fully working, can't say 100%.

///////////////////////////////////////////////////////////////////
// Non-inode functions first
///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use.
// Michael (Yes)
//-----------------------------------------------------------------
void DfsModuleInit() {
// Student
// You essentially set the file system as invalid and then open 
// using DfsOpenFileSystem().
  if ((lock_inodes = LockCreate()) == INVALID_LOCK) {
    printf("Fatal Error: LockCreate fail\n");
    GracefulExit();
  }
  if ((lock_fbv = LockCreate()) == INVALID_LOCK) {
    printf("Fatal Error: LockCreate fail\n");
    GracefulExit();
  }
	if (sizeof(inodes[0]) != 96) {
		printf("FATAL ERROR: Inode size is not 96!\n");
  	GracefulExit();	
	}
	DfsInvalidate();
  if (DfsOpenFileSystem() == DFS_FAIL) {
    printf("Warning: File system not exist, unless you use fdisk, it is a bug\n");
  }
  printf("dfs module init success\n");
}

//-----------------------------------------------------------------
// DfsInavlidate marks the current version of the filesystem in
// memory as invalid.  This is really only useful when formatting
// the disk, to prevent the current memory version from overwriting
// what you already have on the disk when the OS exits.
// Michael (Yes)
//-----------------------------------------------------------------
void DfsInvalidate() {
// This is just a one-line function which sets the valid bit of the 
// superblock to 0.
	sb.valid = 0;
}

//Michael (Yes)
void DfsValidate() {
	sb.valid = 1;
}

//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on 
// failure.
// Michael (Yes)
//-------------------------------------------------------------------
int DfsOpenFileSystem() {
//Basic steps:
// Check that filesystem is not already open
  if (sb.valid == 1) {
    printf("File system already opened! Not gonna open again!\n");
    return DFS_SUCCESS;
  }
// Read superblock from disk.  Note this is using the disk read rather 
// than the DFS read function because the DFS read requires a valid 
// filesystem in memory already, and the filesystem cannot be valid 
// until we read the superblock. Also, we don't know the block size 
// until we read the superblock, either.

// Copy the data from the block we just read into the superblock in memory

// All other blocks are sized by virtual block size:
// Read inodes
// Read free block vector
// Change superblock to be invalid, write back to disk, then change 
// it back to be valid in memory
  
  //Read Super block from disk to memory
  if (readSuperBlock() == DFS_FAIL) {
    printf("fail to read super block\n");
    return DFS_FAIL;
  }
  printf("Read super block done\n");
  if (readInode() == DFS_FAIL) {
    printf("fail to read inode\n");
    return DFS_FAIL;
  }
  printf("Read inode done\n");
  if (readFBV() == DFS_FAIL) {
    printf("fail to read fbv\n");
    return DFS_FAIL;
  }
  printf("Read fbv done\n");
  DfsInvalidate();
  if (writeBackSuperBlock() == DFS_FAIL) {
    printf("fail to write back super block\n");
    return DFS_FAIL;
  }
  printf("Write super block done\n");
  DfsValidate();
  printf("fs open success\n");
  return DFS_SUCCESS;
}

//Michael (Yes)
int dataStartBlock() {
  return sb.fdisk_fbv_block_start + fbvRequiredBlocks() ;
}

//Michael (Yes)
int fbvRequiredBlocks() {
  if (DFS_FBV_MAX_NUM_WORDS * 4 % sb.fdisk_blocksize != 0) {
    printf("Fatal Error: FBV is not dividable\n");
  }
  return DFS_FBV_MAX_NUM_WORDS * 4 / sb.fdisk_blocksize;
}

//Michael (Yes)
int readFBV() {
  int *ptr;
  disk_block db[fsBlockStartToPhysicalBlockStart(dataStartBlock()) - fsBlockStartToPhysicalBlockStart(sb.fdisk_fbv_block_start)];
  int i;
  for (i = fsBlockStartToPhysicalBlockStart(sb.fdisk_fbv_block_start); i < fsBlockStartToPhysicalBlockStart(dataStartBlock()); i++) {
    if (DiskReadBlock(i,&db[i-fsBlockStartToPhysicalBlockStart(sb.fdisk_fbv_block_start)]) == DFS_FAIL) {
      return DFS_FAIL;
    }
  }
  ptr = (int *)(&db);
  for (i = 0; i < DFS_FBV_MAX_NUM_WORDS; i++) {
    fbv[i] = ptr[i];
    // printf("Fbv %d is %d\n", i, fbv[i]);
  }
  return DFS_SUCCESS;
}

//Michael (Yes)
int fdiskFactor() {
  if (sb.fdisk_blocksize % DISK_BLOCKSIZE != 0) {
    printf("FATAL ERROR: file system disk zize not a multiple of physical disk size");
    GracefulExit();
  }
  return sb.fdisk_blocksize / DISK_BLOCKSIZE;
}

//Michael (Yes)
int fsBlockStartToPhysicalBlockStart(int fs_block_start) {
  return fdiskFactor() * fs_block_start;
}

//Michael (Yes)
int readInode() {
  int i;
  disk_block* ptr_block;
  disk_block db[fsBlockStartToPhysicalBlockStart(sb.fdisk_fbv_block_start) - fsBlockStartToPhysicalBlockStart(sb.fdisk_inode_block_start)];
  dfs_inode* ptr_inode;

  ptr_block = &db;
  for (i = fsBlockStartToPhysicalBlockStart(sb.fdisk_inode_block_start); i < fsBlockStartToPhysicalBlockStart(sb.fdisk_fbv_block_start); i++) {
    if (DiskReadBlock(i, &db[i-fsBlockStartToPhysicalBlockStart(sb.fdisk_inode_block_start)]) == DISK_FAIL) {
      return DFS_FAIL;
    }
  }
  ptr_inode = (dfs_inode *) ptr_block;
  for(i = 0; i < sb.fdisk_num_inode; i++) {
    inodes[i] = ptr_inode[i];
    // printf("Inode %d is %d\n", i, inodes[i]);
  }
  // printf("Inode 0 status: %d\n", inodes[0].direct_block_translations[1]);
  return DFS_SUCCESS;
}

//Michael (Yes)
int readSuperBlock() {
  disk_block db;
  dfs_superblock* disk_sb;
  if (DiskReadBlock(PHYSICAL_DISK_SUPER_BLOCK, &db) == DISK_FAIL) {
    return DFS_FAIL; 
  }
  disk_sb = (dfs_superblock*)db.data;
  sb.valid = disk_sb->valid;
  sb.fdisk_blocksize = disk_sb->fdisk_blocksize;
  sb.fdisk_num_blocks = disk_sb->fdisk_num_blocks;
  sb.fdisk_inode_block_start = disk_sb->fdisk_inode_block_start;
  sb.fdisk_num_inode = disk_sb->fdisk_num_inode;
  sb.fdisk_fbv_block_start = disk_sb->fdisk_fbv_block_start;
  printf("ReadSuperBlock: sb.fdisk_blocksize is: %d\n", sb.fdisk_blocksize);
  printf("ReadSuperBlock: sb.fdisk_num_blocks is: %d\n", sb.fdisk_num_blocks);
  printf("ReadSuperBlock: sb.fdisk_inode_block_start is: %d\n", sb.fdisk_inode_block_start);
  printf("ReadSuperBlock: sb.fdisk_num_inode is: %d\n", sb.fdisk_num_inode);
  printf("ReadSuperBlock: sb.fdisk_fbv_block_start is: %d\n", sb.fdisk_fbv_block_start);
  return DFS_SUCCESS;
}

//Michael (Yes)
int writeBackSuperBlock() {
  if (DiskWriteBlock(PHYSICAL_DISK_SUPER_BLOCK, (disk_block*)(&sb)) == DISK_FAIL) {
    return DISK_FAIL;
  }
  return DFS_SUCCESS;
}

//--------------------------------------------------------------------------------------
// @Michael: Above all are working right now, tested
//--------------------------------------------------------------------------------------

//Michael (Yes)
int writeBackInode() {
  int i;
  disk_block* ptr_disk_block;
  ptr_disk_block = (disk_block*) inodes;
  for (i = fsBlockStartToPhysicalBlockStart(sb.fdisk_inode_block_start); i < fsBlockStartToPhysicalBlockStart(sb.fdisk_fbv_block_start); i++) {
    if (DiskWriteBlock(i, ptr_disk_block) == DISK_FAIL) {
      return DFS_FAIL;
    }
    ptr_disk_block = ptr_disk_block + 1;
  }
  return DFS_SUCCESS;
}


//Michael (Yes)
int writeBackFBV() {
  int i;
  disk_block* ptr;
  ptr = (disk_block *) (&fbv[0]);
  for (i = fsBlockStartToPhysicalBlockStart(sb.fdisk_fbv_block_start); i < fsBlockStartToPhysicalBlockStart(dataStartBlock()); i++) {
    if (DiskWriteBlock(i, ptr) == DISK_FAIL) {
      return DFS_FAIL;
    }
    ptr = ptr + 1;
  }
  return DFS_SUCCESS;
}


//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
// Michael (Yes)
//-------------------------------------------------------------------
int DfsCloseFileSystem() {

  if (sb.valid == 0) {
    printf("DFS close successfully without flush\n");
    return DFS_SUCCESS;
  }

  if (writeBackFBV() == DFS_FAIL) {
    printf("Fatal Error: Fail to write back FBV\n");
    return DISK_FAIL;
  }
  if (writeBackInode() == DFS_FAIL) {
    printf("Fatal Error: Fail to write back Inodes\n");
    return DISK_FAIL;
  }
  if (writeBackSuperBlock() == DFS_FAIL) {
    printf("Fatal Error: Fail to write back superblock\n");
    return DISK_FAIL;
  }
  DfsInvalidate();
  printf("DFS close successfully\n");
  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------
//Written by PCW
//Debugged and tested by Michael
//Michael (Yes)
//return: fs block num
uint32 DfsAllocateBlock() {
// Check that file system has been validly loaded into memory
// Find the first free block using the free block vector (FBV), mark it in use
// Return handle to block
  int vector_index;
  int bit_offset;
  int blocknum;
  // Check that file system has been validly loaded into memory
  if (sb.valid == 0){
    printf("DfsAllocateBlock FATAL ERROR: File System is not valid!\n");
    return DFS_FAIL;
  }
  // Find the first free block using the free block vector (FBV), mark it in use
  // Use lock whenever allocate or deallocate file system blocks using the free block vector

  // Michael Comment: Why using while loop here to get lock? LockHandleAcquire will get blocked until get lock!
  // while (LockHandleAcquire(dfs_lock_fbv) != SYNC_SUCCESS);

  if (LockHandleAcquire(lock_fbv) != SYNC_SUCCESS) {
    printf("Fatal Error: lock acquire failed\n");
    return DFS_FAIL;
  }

  // 0-used, 1-free 
  for (vector_index = 0; vector_index < DFS_FBV_MAX_NUM_WORDS; vector_index++){
    // Find the first vector that hasn't been fully used (fully used = 0)
    if (fbv[vector_index] != 0){
      break;
    }
    // If there isn't any usable block after profiled all the vectors, give an error
    if (vector_index == DFS_FBV_MAX_NUM_WORDS - 1){
      printf("FATAL ERROR: DfsAllocateBlock No available block!\n");
      return DFS_FAIL;
    }
  }

  bit_offset = 0;
  while ((fbv[vector_index] & (1 << bit_offset)) == 0){
    bit_offset++;
  }
  // Record that this page is allocated
  dbprintf('d', "DfsAllocateBlock: FBV before allocated %d\n", fbv[vector_index]);
  fbv[vector_index] = fbv[vector_index] & invert(1 << bit_offset);
  dbprintf('d', "DfsAllocateBlock: FBV after allocated %d\n", fbv[vector_index]);

  // Calculate the block number as return value (block num start from 0)
  blocknum = (vector_index * 32) + bit_offset;
  dbprintf('d', "DfsAllocateBlock: Allocated Block num: %d\n", blocknum);
  LockHandleRelease(lock_fbv);
  return blocknum;
}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------
//Written by PCW
//Debugged and tested by Michael
//Michael (Yes)
//return DFS_SUCCESS or FAIL
int DfsFreeBlock(uint32 blocknum) {
  int vector_index;
  int bit_offset;
  int zeros[sb.fdisk_blocksize / 4];
  dfs_block *ptr;
  int i;
  // Check that file system has been validly loaded into memory
  if (sb.valid == 0){
    printf("DfsFreeBlock FATAL ERROR: File System is not valid!\n");
    return DFS_FAIL;
  }

  //Michael Comment: Same here, while is needless!
  //while(LockHandleAcquire(dfs_lock_fbv) != SYNC_SUCCESS);
  if (LockHandleAcquire(lock_fbv) != SYNC_SUCCESS) {
    printf("Fatal Error: lock acquire failed\n");
    return DISK_FAIL;
  }
  vector_index = blocknum / 32;
  bit_offset = blocknum % 32;
  // 0-used, 1-free  
  // if ((fbv[vector_index] & (1 << bit_offset)) != 0) {
  //   printf("Fatal Error: DfsFreeBlock block %d is not in use\n", blocknum);
  //   return DFS_FAIL;
  // }
  fbv[vector_index] = fbv[vector_index] | (1 << bit_offset);
  dbprintf('d', "DfsFreeBlock: Free Block num: %d\n", blocknum);
  //clear actual content
  for (i = 0; i < sb.fdisk_blocksize / 4; i++) {
    zeros[i] = 0;
  }
  ptr = (dfs_block *) zeros;
  if (DfsWriteBlock(blocknum, ptr) == DFS_FAIL) {
    printf("Fatal Error: Fail to write back zeros in DfsFreeBlock\n");
    return DFS_FAIL;
  }
  LockHandleRelease(lock_fbv);
  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.
//-----------------------------------------------------------------
//Written by PCW
//Debugged and tested by Michael
//Michael (Yes)
int DfsReadBlock(uint32 blocknum, dfs_block *b) {
  disk_block db;
  int multiple;
  int phy_blocknum;
  int phy_blockzise;
  int i;
  int results;
  results = 0;
  // Check that file system has been validly loaded into memory
  if (sb.valid == 0){
    printf("DfsReadBlock FATAL ERROR: File System is not valid!\n");
    return DFS_FAIL;
  }

  phy_blockzise = DiskBytesPerBlock();
  multiple = sb.fdisk_blocksize / phy_blockzise;
  phy_blocknum = multiple * blocknum;
  // The index need to -1
  // For example when multiple = 3:
  // dsf_index   phy_index
  // 0           0, 1, 2
  // 1           3, 4, 5
  // 2           6, 7, 8
  for (i = phy_blocknum; i < (phy_blocknum + multiple); i++){
    if (DiskReadBlock(i, &db) == DISK_FAIL){
      return DFS_FAIL;
    }
    //Michael Comment: You did totally wrong at bcopy, I corrected
    bcopy(db.data, &(b->data[(i-phy_blocknum)*phy_blockzise]), phy_blockzise);
    results += phy_blockzise;
  }
  return results;
}

//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------
//Michael (Yes)
int DfsWriteBlock(uint32 blocknum, dfs_block *b){
  //input: fs-blocknum
  int i;
  int total_size;
  int cur_size;
  disk_block* ptr;
  ptr = (disk_block *)b;
  total_size = 0;
  //Printf("Start blocks is: %d\n", fsBlockStartToPhysicalBlockStart(blocknum));
  for (i = 0; i < fdiskFactor(); i++) {
    if ((cur_size = DiskWriteBlock(i + fsBlockStartToPhysicalBlockStart(blocknum), ptr)) == DISK_FAIL) {
      return DFS_FAIL;
    }
    total_size = total_size + cur_size;
    ptr = ptr + 1;
  }
  return total_size;
}

//Michael (Yes)
int physicalToFsdisk(uint32 blocknum) {
  //Assume 0 based
  int factor = fdiskFactor();
  return blocknum / factor;
}

////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for 
// the given filename. If the filename is found, return the handle 
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------
//Written by PCW
//Debugged and tested by Michael
//Michael (Yes)
uint32 DfsInodeFilenameExists(char *filename) {
  int i;
  // Check that file system has been validly loaded into memory
  if (sb.valid == 0){
    printf("DfsInodeFilenameExists FATAL ERROR: File System is not valid!\n");
    return DFS_FAIL;
  }
  if (LockHandleAcquire(lock_inodes) != SYNC_SUCCESS) {
    printf("Fatal Error: lock acquire failed\n");
    return DFS_FAIL;
  }
  // Profile all the inodes
  for (i = 0; i < sb.fdisk_num_inode; i++){
    // Only compare if the inode is inused 
    if (inodes[i].inuse == 1){
      if (dstrncmp(inodes[i].file_name, filename, dstrlen(inodes[i].file_name)) == 0){
        // If the filename is found, return the handle of the inode
        return i;
      }
    }
  }
  LockHandleRelease(lock_inodes); 
  // If it is not found, return DFS_FAIL
  return DFS_FAIL;
}

// Michael (Yes)
int numInuseInode() {
  int i;
  int acc;
  acc = 0;
  for (i = 0; i < sb.fdisk_num_inode; i++){
    if (inodes[i].inuse == 1){
      acc = acc + 1;
    }
  }  
  return acc;
}

//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------
//Written by PCW
//Debugged and tested by Michael
//Michael (Yes)
uint32 DfsInodeOpen(char *filename) {
  int inode_handle;
  int i;
  // Check that file system has been validly loaded into memory
  if (sb.valid == 0){
    printf("DfsInodeOpen FATAL ERROR: File System is not valid!\n");
    return DFS_FAIL;
  }

  if (dstrlen(filename) > 40) {
    printf("FATAL ERROR: file name too long\n");
    return DFS_FAIL;
  }
  // Michael Comment: I feel read write lock work best in this senario, 
  // unfortunately we don't have read write lock support in this project!

  // If the filename exists, return the handle of the inode
  inode_handle = DfsInodeFilenameExists(filename);
  if (inode_handle != DFS_FAIL){
    printf("DfsInodeOpen: file alreay exist!\n");
    return inode_handle;
  }

  // If the filename does not, allocate a new inode for it and return its handle
  // Michael Comment: Don't use while loop for lock!
  // while(LockHandleAcquire(dfs_lock_fbv) != SYNC_SUCCESS);
  if (LockHandleAcquire(lock_inodes) != SYNC_SUCCESS) {
    printf("Fatal Error: lock acquire failed\n");
    return DFS_FAIL;
  }

  // Profile all the inodes
  for (i = 0; i < sb.fdisk_num_inode; i++){
    // Find the first free inode 
    if (inodes[i].inuse == 0){
      // Set this inode as inused
      inodes[i].inuse = 1;
      // Set the file size
      inodes[i].file_size = 0;
      // Copy the filename to inode
      dstrncpy(inodes[i].file_name, filename, dstrlen(filename));
      // return the handle of the newly allocated inode
      inode_handle = i;
      break;
    }
  }
  LockHandleRelease(lock_inodes);
  return inode_handle;
}

//Michael (Yes)
int deleteAllInode() {
  int i;
  for (i = 0; i < sb.fdisk_num_inode; i++) {
    if (DfsInodeDelete(i) == DFS_FAIL) {
      return DFS_FAIL;
    }
  }
  return DFS_SUCCESS;
}

//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------
//Written by PCW
//Debugged and tested by Michael
//Michael (Yes)
int DfsInodeDelete(uint32 handle) {
  int i, j;
  int blocknum;
  int indirect_blocknum;
  int double_indirect_blocknum;
  dfs_block buffer_level_1;
  dfs_block buffer_level_2;
  dfs_block buffer_level_3;
  int buffer_1_size;
  int buffer_2_size;
  int buffer_3_size;
  int* ptr_1;
  int* ptr_2;
  int blocknum_2;
  // Check that file system has been validly loaded into memory
  if (sb.valid == 0){
    printf("DfsInodeDelete FATAL ERROR: File System is not valid!\n");
    return DFS_FAIL;
  }

  //while(LockHandleAcquire(dfs_lock_fbv) != SYNC_SUCCESS);
  if (LockHandleAcquire(lock_inodes) != SYNC_SUCCESS) {
    printf("Fatal Error: Fail to get lock\n");
    return DISK_FAIL;
  }

  // Check that if inode file exist
  if (inodes[handle].inuse == 0){
    // printf("DfsInodeDelete Inode doesn't exist, skipped!\n");
    return DFS_SUCCESS;
  }

  // Set the file size as 0
  inodes[handle].file_size = 0;
  // Set the file name to 0
  bzero(inodes[handle].file_name, dstrlen(inodes[handle].file_name));

  // Set the direct block translation table as 0
  for (i = 0; i < 10; i ++){
    // See if the entry exist
    if (inodes[handle].direct_block_translations[i]){
      // Get the block entry
      blocknum = inodes[handle].direct_block_translations[i];
      // Set the fbv as free
      if (DfsFreeBlock(blocknum) == DFS_FAIL) {
        printf("Fatal ERROR: free block fail in DfsInodeDelete direct\n");
        return DFS_FAIL;
      }
      // Set the block entry as 0
      inodes[handle].direct_block_translations[i] = 0;
    }
  }

  // Set the indirect block translation table as 0 
  // See if the indirect entry exist
  if (inodes[handle].indirect_block_translations){
    // Get the block entry
    indirect_blocknum = inodes[handle].indirect_block_translations;
    // Read the content from the block and put in buffer
    if (DfsReadBlock(indirect_blocknum, &buffer_level_1) == DFS_FAIL) {
      printf("Fatal Error: fail to read indirect block\n");
      return DFS_FAIL;
    }
    if (buffer_1_size == DFS_FAIL) {
      printf("FATAL_ERROR: DFS READ BLOCK FAIL inside inode delete\n");
      return DFS_FAIL;
    }
    // Profile all the content in indirect block translation table
    // Michael Comment: I rewrite all the following part
    for (i = 0; i < DFS_BLOCKSIZE / 4; i++){
      if (buffer_level_1.data[i] != 0) {
        ptr_1 = (i+((int *)buffer_level_1.data));
        blocknum = *ptr_1;
        if (blocknum != 0) {
          if (DfsFreeBlock(blocknum) == DFS_FAIL) {
            printf("FATAL ERROR: free block fail in DfsInodeDelete indirect\n");
            return DFS_FAIL;
          }
        }
      }
      // Set the block entry as 0
      // buffer_level_1.data[i] = 0;
    }
    // DfsWriteBlock(indirect_blocknum, &buffer_level_1);      
    // Set the indirect block as free
    if (DfsFreeBlock(indirect_blocknum) == DFS_FAIL) {
      printf("FATAL ERROR: free block fail in DfsInodeDelete indirect\n");
      return DFS_FAIL; 
    }
    // Set the block entry as 0
    inodes[handle].indirect_block_translations = 0;
  } 

  // Set the double indirect block translation table as 0 
  // See if the double indirect entry exist
  // Michael Comment: Rewrite part of the logic
  if (inodes[handle].double_indirect_block_translations){
    // Get the block entry
    double_indirect_blocknum = inodes[handle].double_indirect_block_translations;
    // Read the content from the block and put in buffer
    if (DfsReadBlock(double_indirect_blocknum, &buffer_level_1) == DFS_FAIL) {
      printf("Fatal Error: fail to read double indirect block\n");
      return DFS_FAIL;  
    }
    // Profile all the content in double indirect block translation table
    for (i = 0; i < DFS_BLOCKSIZE / 4; i++){
      ptr_1 = (i+((int *)buffer_level_1.data));
      blocknum = *ptr_1;
      if (blocknum != 0) {
        if (DfsReadBlock(blocknum, &buffer_level_2) == DFS_FAIL) {
          printf("FATAL ERROR: fail to read double indirect block\n");
          return DFS_FAIL;
        }
      } else {
        continue;
      }

      // Profile all the content in the next level
      for (j = 0; j < DFS_BLOCKSIZE / 4; j++){
        ptr_2 = (i+((int *)buffer_level_2.data));
        blocknum_2 = *ptr_2;
        if (blocknum_2 != 0) {
          if (DfsFreeBlock(blocknum_2) == DFS_FAIL) {
            printf("FATAL ERROR: free block fail in DfsInodeDelete double indirect 1\n");
            return DFS_FAIL; 
          }
        }
        // Set all the block entry as 0
        // buffer_level_2->data[j] = 0;
      }
      // Free the fbv
      if (blocknum != 0) {
        if (DfsFreeBlock(blocknum) == DFS_FAIL) {
          printf("FATAL ERROR: free block fail in DfsInodeDelete double indirect 2\n");
          return DFS_FAIL;  
        }
      }
      // Set all the block entry as 0
      // buffer_level_1->data[i] = 0;
    }
    // Set the indirect block as free
    if (double_indirect_blocknum != 0) {
      if (DfsFreeBlock(double_indirect_blocknum) == DFS_FAIL) {
          printf("FATAL ERROR: free block fail in DfsInodeDelete double indirect 3\n");
          return DFS_FAIL;        
      }
    }
    // Set the block entry as 0
    inodes[handle].double_indirect_block_translations = 0;
  }   
  // Set the inode as free
  inodes[handle].inuse = 0;
  LockHandleRelease(lock_inodes);
  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------
// Michael (Yes)
int calcStartIdx(int start_byte) {
  return start_byte / sb.fdisk_blocksize;
}

// Michael (Yes)
int calcStartByte(int start_byte) {
  return start_byte % sb.fdisk_blocksize;
}

// Michael (Yes)
int calcEndIdx(int start_byte, int num_bytes) {
  return (start_byte + num_bytes) / sb.fdisk_blocksize;
}

int calcEndByte(int start_byte, int num_bytes) {
  return (start_byte + num_bytes) % sb.fdisk_blocksize;
}

// Michael (Yes)
int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  //First get file block index
  int startIdx = calcStartIdx(start_byte);
  int startByte = calcStartByte(start_byte);
  int endIdx = calcEndIdx(start_byte, num_bytes);
  int endByte = calcEndByte(start_byte, num_bytes);
  int i;
  uint32 fsblock_idx;
  dfs_block block;
  char *buf = (char *)mem;

  if (sb.valid == 0){
    printf("DfsInodeFilesize FATAL ERROR: File System is not valid!\n");
    return DFS_FAIL;
  }

  for (i = startIdx; i < endIdx + 1; i++) {
    //Translate each file block index to dfs block index
    fsblock_idx = DfsInodeTranslateVirtualToFilesys(handle, i);
    if (fsblock_idx == DFS_FAIL) {
      printf("Fatal Error: DfsInodeTranslateVirtualToFilesys inside DfsInodeReadBytes. Read Fail\n");
      return DFS_FAIL;
    }
    if (fsblock_idx == UNINIT) {
      printf("Fatal Error: illegal read on un-allocated block\n");
      return DFS_FAIL;
    }

    if (DfsReadBlock(fsblock_idx, &block) == DFS_FAIL) {
      return DFS_FAIL;
    }
    if (i == startIdx) {
      bcopy(block.data + startByte, buf, sb.fdisk_blocksize - startByte);
      buf = buf + sb.fdisk_blocksize - startByte;
    } else if (i == endIdx) {
      bcopy(block.data, buf, endByte);
    } else { //copy all
      bcopy(block.data, buf, sb.fdisk_blocksize);
      buf = buf + sb.fdisk_blocksize;
    }
  }
  return num_bytes;
}


//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to 
// by mem to the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only writing part 
// of a given file system block, you'll need to read that block 
// from the disk first. Return DFS_FAIL on failure and the number 
// of bytes written on success.
//-----------------------------------------------------------------
// Michael (Yes)
int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  //First get file block index
  int startIdx = calcStartIdx(start_byte);
  int startByte = calcStartByte(start_byte);
  int endIdx = calcEndIdx(start_byte, num_bytes);
  int endByte = calcEndByte(start_byte, num_bytes);
  int i;
  uint32 fsblock_idx;
  dfs_block block;
  char *buf = (char *)mem;

  if (sb.valid == 0){
    printf("DfsInodeFilesize FATAL ERROR: File System is not valid!\n");
    return DFS_FAIL;
  }

  for (i = startIdx; i < endIdx + 1; i++) {
    //Translate each file block index to dfs block index
    fsblock_idx = DfsInodeTranslateVirtualToFilesys(handle, i);
    // printf("fsblock_idx is %d\n",fsblock_idx);
    if (fsblock_idx == DFS_FAIL) {
      printf("Fatal Error: DfsInodeTranslateVirtualToFilesys fail in DfsInodeWriteBytes\n");
      return DFS_FAIL;
    }
    if (fsblock_idx == UNINIT) {
      // printf("Start to Allocate Blocks for inode %d\n", handle);
      fsblock_idx = DfsInodeAllocateVirtualBlock(handle, i);
      if (fsblock_idx == DFS_FAIL) {
        printf("Fatal Error: Allocate Blocks inside DfsInodeWriteBytes Fail\n");
        return DFS_FAIL;
      }
      // printf("The new allocated block is %d\n", fsblock_idx);
      // printf("current vblocks num is %d\n", i);
      // printf("The translate table entry is %d\n", inodes[handle].direct_block_translations[i]);
    }
    //First Read Then Write
    if (DfsReadBlock(fsblock_idx, &block) == DFS_FAIL) {
      printf("Fatal Error: Fail to read allocated block inside DfsInodeWriteBytes\n");
      return DFS_FAIL;
    }
    if (i == startIdx) {
      bcopy(buf, block.data + startByte, sb.fdisk_blocksize - startByte);
      buf = buf + sb.fdisk_blocksize - startByte;
      if (DfsWriteBlock(fsblock_idx, &block) == DFS_FAIL) {
        printf("Fatal Error: Fail to write back block block inside DfsInodeWriteBytes\n");
        return DFS_FAIL;
      }
    } else if (i == endIdx) {
      bcopy(buf, block.data, endByte);
      if (DfsWriteBlock(fsblock_idx, &block) == DFS_FAIL) {
        printf("Fatal Error: Fail to write back block block inside DfsInodeWriteBytes\n");
        return DFS_FAIL;
      }
    } else { //copy all
      bcopy(buf, block.data, sb.fdisk_blocksize);
      buf = buf + sb.fdisk_blocksize;
      if (DfsWriteBlock(fsblock_idx, &block) == DFS_FAIL) {
        printf("Fatal Error: Fail to write back block block inside DfsInodeWriteBytes\n");
        return DFS_FAIL;
      }
    }
  }
  if (inodes[handle].file_size < start_byte + num_bytes) {
    inodes[handle].file_size = start_byte + num_bytes;
  }
  return num_bytes;
}


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------
//Written by PCW
//Debugged and tested by Michael
//Michael (Yes)
uint32 DfsInodeFilesize(uint32 handle) {
  // Check that file system has been validly loaded into memory
  if (sb.valid == 0){
    printf("DfsInodeFilesize FATAL ERROR: File System is not valid!\n");
    return DFS_FAIL;
  }
  // Check that if inode file exist
  if (inodes[handle].inuse == 0){
    printf("DfsInodeFilesize FATAL ERROR: Inode file doesn't exist!\n");
    return DFS_FAIL;
  }
  return inodes[handle].file_size;
}


//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block 
// for the given inode, storing its blocknumber at index 
// virtual_blocknumber in the translation table. If the 
// virtual_blocknumber resides in the indirect address space, and 
// there is not an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file system 
// block number on success.
//-----------------------------------------------------------------

// void blockInit(dfs_block block) {
//   // int *ptr;
//   // ptr = (int *)block.data;
//   // for (i = 0; i < sb.fdisk_blocksize / sizeof(uint32); i++) {
//   //   ptr[i] = UNINIT;
//   // }
//   // return;
// }

// Michael (Yes)
int total_direct_blocks() {
  return 10;
}

// Michael (Yes)
int total_indirect_blocks() {
  return sb.fdisk_blocksize / sizeof(uint32);
}

// Michael (Yes)
int total_double_indirect_blocks() {
  return total_indirect_blocks() * total_indirect_blocks();
}

// Michael (Yes)
int inodeMaxBlocks() {
  return total_direct_blocks() + total_indirect_blocks() + total_double_indirect_blocks();
}

// Michael (Yes)
int atDirect(int vblock) {
    if (vblock < 10) {
      return 1;
    }
    return 0;
}

// Michael (Yes)
int atSingleIndirect(int vblock) {
  if (vblock < 10 + (sb.fdisk_blocksize / sizeof(uint32))) {
    return 1;
  }
  return 0;
}

// Michael (Yes)
int get_level1_index_for_double_indirect(int vblock) {
  int result;
  result = vblock - total_direct_blocks() - total_indirect_blocks();
  result = result / total_indirect_blocks();
  if (result < 0) {
    printf("Fatal Error: at double indirect block, level 1 map's index < 0\n");
  }
  return result;
}

// Michael (Yes)
// return the correct index on level 2 map
int get_level2_index_for_double_indirect(int vblock) {
  int result;
  result = vblock - total_direct_blocks() - total_indirect_blocks();
  result = result % total_indirect_blocks();
  if (result < 0) {
    printf("Fatal Error, at double indirect block, level 2 map's index < 0\n");
  }
  return result;
}

// Michael (Yes)
uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {
  uint32 dfsHandler0;
  uint32 dfsHandler1;
  uint32 dfsHandler2;
  dfs_block block_level1;
  dfs_block block_level2;
  int *ptr;
  int lvl_1_idx;
  int lvl_2_idx;

  if (sb.valid == 0){
    printf("DfsInodeFilesize FATAL ERROR: File System is not valid!\n");
    return DFS_FAIL;
  }

  if (virtual_blocknum > inodeMaxBlocks()) {
    printf("Fatal Error: virtual_blocknum exceed the limit in DfsInodeAllocateVirtualBlock\n");
    return DISK_FAIL;
  }

  if (atDirect(virtual_blocknum)) {
    if (inodes[handle].direct_block_translations[virtual_blocknum] != UNINIT) {
      printf("Fatal Error: DfsInodeAllocateVirtualBlock you can't re-allocate a block!\n");
      return DFS_FAIL; 
    }
    if ((dfsHandler0 = DfsAllocateBlock()) == DFS_FAIL) {
      printf("Fatal Error: unable to allocate fs block in DfsInodeAllocateVirtualBlock\n");
      return DFS_FAIL;
    }
    inodes[handle].direct_block_translations[virtual_blocknum] = dfsHandler0;
    // printf("here vblock %d\n", virtual_blocknum);
    // printf("here inode.direct_block_translations[virtual_blocknum] is %d\n", inodes[handle].direct_block_translations[virtual_blocknum]);
    return dfsHandler0;
  } else if (atSingleIndirect(virtual_blocknum)) {
    //allocate map if un-allocated and get block index for the translation table
    if (inodes[handle].indirect_block_translations == UNINIT) {
      if ((dfsHandler0 = DfsAllocateBlock()) == DFS_FAIL) {
        printf("Fatal Error: unable to allocate fs block in DfsInodeAllocateVirtualBlock\n");
        return DFS_FAIL;
      }
      inodes[handle].indirect_block_translations = dfsHandler0;
    } else {
      dfsHandler0 = inodes[handle].indirect_block_translations;
    }

    //first read and then write to store the actual data block index into map (translation table)
    //ie. update indirect map
    if (DfsReadBlock(dfsHandler0, &block_level1) == DFS_FAIL) {
      printf("Fatal Error: unable to read fs block in DfsInodeAllocateVirtualBlock\n");
      return DFS_FAIL;
    }
    ptr = (int *)block_level1.data;
    
    //re-alloc check
    if (ptr[virtual_blocknum - 10] != UNINIT) {
      printf("Fatal Error: re-allocate a block is not allowed\n");
      return DFS_FAIL;
    }

    //allocate actual data block and get its index
    if ((dfsHandler1 = DfsAllocateBlock()) == DFS_FAIL) {
      printf("Fatal Error: unable to allocate fs block in DfsInodeAllocateVirtualBlock\n");
      return DFS_FAIL;
    }
    
    //update memory copy
    ptr[virtual_blocknum - 10] = dfsHandler1;

    //write back to disk
    if (DfsWriteBlock(dfsHandler0, &block_level1) == DFS_FAIL) {
      printf("Fatal Error: unable to write back fs block in DfsInodeAllocateVirtualBlock\n");
      return DFS_FAIL;
    }

    //return actual block index
    return dfsHandler1;
  } else {
    lvl_1_idx = get_level1_index_for_double_indirect(virtual_blocknum);
    lvl_2_idx = get_level2_index_for_double_indirect(virtual_blocknum);
    //set up handle0 for level 1 map
    if (inodes[handle].double_indirect_block_translations == UNINIT) {
      if ((dfsHandler0 = DfsAllocateBlock()) == DFS_FAIL) {
          return DFS_FAIL;
      } 
      inodes[handle].double_indirect_block_translations = dfsHandler0;
    } else {
      dfsHandler0 = inodes[handle].double_indirect_block_translations;
    }

    //read level 1 map into memory
    if (DfsReadBlock(dfsHandler0, &block_level1) == DFS_FAIL) {
      return DFS_FAIL;
    }
    ptr = (int *)block_level1.data;
    
    //set up handle1 for level 2 map
    if (ptr[lvl_1_idx] == UNINIT) {
      if ((dfsHandler1 = DfsAllocateBlock()) == DFS_FAIL) {
        return DFS_FAIL;
      }
      //write back to update level 1 map at disk
      ptr[lvl_1_idx] = dfsHandler1; 
      if (DfsWriteBlock(dfsHandler0, &block_level1) == DFS_FAIL) {
        return DFS_FAIL;
      }
    } else {
      dfsHandler1 = ptr[lvl_1_idx];
    }

    //read level 2 map into memory
    if (DfsReadBlock(dfsHandler1, &block_level2) == DFS_FAIL) {
      return DFS_FAIL;
    }
    ptr = (int *)block_level2.data;

    //re-alloc check
    if (ptr[lvl_2_idx] != UNINIT) {
      printf("Fatal Error: you can't re-allocate block again!\n");
      return DFS_FAIL;
    }

    //finally allocate the actual block
    if ((dfsHandler2 = DfsAllocateBlock()) == DFS_FAIL) {
        return DFS_FAIL;
    }

    //update memory
    ptr[lvl_2_idx] = dfsHandler2;

    //flash to disk
    if (DfsWriteBlock(dfsHandler1, &block_level2) == DFS_FAIL) {
      return DFS_FAIL;
    }

    //return actual block index
    return dfsHandler2;
  }
}

//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------
// Michael (Yes)
uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) {
  dfs_block block_level1;
  dfs_block block_level2;
  int *ptr;
  int idx_level1;
  int idx_level2;

  if (sb.valid == 0){
    printf("DfsInodeFilesize FATAL ERROR: File System is not valid!\n");
    return DFS_FAIL;
  }

  if (virtual_blocknum > inodeMaxBlocks()) {
    printf("Faltal Error: virtual_blocknum exceed the limit\n");
    return DISK_FAIL;
  }

  if (atDirect(virtual_blocknum)) {
    // printf("inode handle is:%d\n", handle);
    // printf("vlock inside translate is %d\n", virtual_blocknum);
    // printf("entry is %d\n", inodes[handle].direct_block_translations[virtual_blocknum]);
    return inodes[handle].direct_block_translations[virtual_blocknum];
  } else if (atSingleIndirect(virtual_blocknum)) {
    if (inodes[handle].indirect_block_translations == UNINIT) {
      inodes[handle].indirect_block_translations = DfsAllocateBlock();
      if (inodes[handle].indirect_block_translations == DFS_FAIL) {
        printf("Faltal Error: DfsAllocateBlock inside DfsInodeTranslateVirtualToFilesys fail\n");
        return DFS_FAIL;
      }
    }
    if (DfsReadBlock(inodes[handle].indirect_block_translations, &block_level1) == DISK_FAIL) {
      printf("Fatal Error: Fail to read single indirect data block\n");
      return DFS_FAIL;
    }
    ptr = (int*)block_level1.data;
    return ptr[virtual_blocknum - 10];
  } else {
    idx_level1 = get_level1_index_for_double_indirect(virtual_blocknum);
    idx_level2 = get_level2_index_for_double_indirect(virtual_blocknum);

    //level 1 map check
    if (inodes[handle].double_indirect_block_translations == UNINIT) {
      inodes[handle].double_indirect_block_translations = DfsAllocateBlock();
      if (inodes[handle].double_indirect_block_translations == DFS_FAIL) {
        printf("Faltal Error: DfsAllocateBlock inside DfsInodeTranslateVirtualToFilesys fail\n");
        return DFS_FAIL;
      }
    }

    //read first indirect block
    if (DfsReadBlock(inodes[handle].double_indirect_block_translations, &block_level1) == DISK_FAIL) {
      printf("Fatal Error: Fail to read single indirect data block\n");
      return DFS_FAIL;
    }
    ptr = (int*)block_level1.data;

    //level 2 map check
    if (ptr[idx_level1] == UNINIT) {
      ptr[idx_level1] = DfsAllocateBlock();
      if (ptr[idx_level1] == DFS_FAIL) {
        printf("Faltal Error: DfsAllocateBlock inside DfsInodeTranslateVirtualToFilesys fail\n");
        return DFS_FAIL;
      }
    }

    //read double indirect block
    if (DfsReadBlock(ptr[idx_level1], &block_level2) == DISK_FAIL) {
      printf("Fatal Error: Fail to read single indirect data block\n");
      return DFS_FAIL;
    }
    ptr = (int *)block_level2.data;
    return ptr[idx_level2];
  }
}
