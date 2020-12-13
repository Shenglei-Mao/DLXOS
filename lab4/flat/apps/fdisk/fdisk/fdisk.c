#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

dfs_superblock sb;
dfs_inode inodes[DFS_INODE_MAX_NUM];
uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0; // These are global in order to speed things up
int disksize = 0; // (i.e. fewer traps to OS to get the same number)

int FdiskWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk

//@Author: Michael (Yes)
void main (int argc, char *argv[])
{
  int i;
  int j;
  int num_filesystem_blocks;
  dfs_inode cur;
  dfs_block* ptr_dfs_block; 
  int fs_data_start;
  uint32 os[64];
	// STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
	// You need to think of the finer details. You can use bzero() to zero out bytes in memory

  //Initializations and argc check

  // Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
  // sb.valid = 0
  //superblock setup

  diskblocksize = 512; 
  disksize = 64 * 1024 * 1024;
  num_filesystem_blocks = disksize / diskblocksize;
  dfs_invalidate(); //@Michael: must use trap, no second choice, kernel sb is not same as user sb!
  sb.fdisk_blocksize = diskblocksize;
  sb.fdisk_num_blocks = num_filesystem_blocks;
  sb.fdisk_inode_block_start = FDISK_INODE_BLOCK_START;
  sb.fdisk_num_inode = FDISK_NUM_INODES;
  sb.fdisk_fbv_block_start = FDISK_FBV_BLOCK_START;






  // Make sure the disk exists before doing anything else
  if (disk_create() == DISK_FAIL) {
    Printf("Fatal Error: Disk Creation fail!\n");
    return;
  }
  Printf("Disk Create Succeed!\n");

  // Write all inodes as not in use and empty (all zeros)
  // Next, setup free block vector (fbv) and write free block vector to the disk
  // Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
  // boot record is all zeros in the first physical block, and superblock structure goes into the second physical block
  
  //Write all inodes as not in use and empty (all zeros)

  //step1 write and store all inode
  Printf("Inode size is: %d\n", sizeof(dfs_inode));
  for (i = 0; i < DFS_INODE_MAX_NUM; i++) {
    cur = inodes[i];
    cur.inuse = 0;
    cur.file_size = 0;
    cur.file_name[0] = '\0';
    for (j = 0; j < 10; j++) {
      cur.direct_block_translations[j] = UNINIT;
    }
    cur.indirect_block_translations = UNINIT;
    cur.double_indirect_block_translations = UNINIT;
  }

  ptr_dfs_block = (dfs_block *) (&inodes);
  for (i = FDISK_INODE_BLOCK_START; i < FDISK_FBV_BLOCK_START; i++) {
    if (FdiskWriteBlock(i, ptr_dfs_block + (i - FDISK_INODE_BLOCK_START)) == DISK_FAIL) {
      Printf("Fatal Error: FdiskWriteBlock fail\n");
      return;
    }
  }

  //step2 write and store fbv
  //all block before data block are used for file system meta-data
  //0 - used, 1 - free
  fs_data_start = dataStartBlock();
  Printf("data block start is: %d\n", fs_data_start);
  //1 fbv integer represent 32 blocks, 131072 fs blocks need DFS_FBV_MAX_NUM_WORDS of fbv integers 4096 fbv
  //1 int is 4 byte, total byte is 4096 * 4 = 16383 byte
  //16383 byte / 512 byte per fs block = 32 fs blocks
  for (i = 0; i < DFS_FBV_MAX_NUM_WORDS; i++) {
    fbv[i] = NOT_USE_MASK;
  }
  for (i = 0; i < fs_data_start / 32; i++) {
    fbv[i] = IN_USE_MASK;
  }
  fbv[fs_data_start / 32] = (NOT_USE_MASK << (fs_data_start % 32)) & fbv[fs_data_start / 32];
  //fbv[fs_data_start / 32] = NOT_USE_MASK;
  //Printf("Hello World\n");
  //Printf("flag is %d\n", (NOT_USE_MASK << (fs_data_start % 32)));
  //Printf("fbv end is: %d\n", fbv[fs_data_start / 32]);
  
  ptr_dfs_block = (dfs_block *) fbv;
  for (i = FDISK_FBV_BLOCK_START; i < fs_data_start; i++) {
    if (FdiskWriteBlock(i, ptr_dfs_block) == DISK_FAIL) {
      Printf("Fatal Error: FdiskWriteBlock fail\n");
      return;   
    }
    ptr_dfs_block = ptr_dfs_block + 1;
  }

  //step3 
  // setup superblock as valid filesystem and write superblock and boot record to disk: 
  // boot record is all zeros in the first physical block, and superblock structure goes into the second physical block
  //Write OS 256 bytes -> 64 int array

  for (i = 0; i < 64; i++) {
    os[i] = 0;
  }
  if (disk_write_block(0, (char *)os) == DISK_FAIL) {
    Printf("Fatal Error: OS init fail\n");
    return;
  }

  //Write superblock
  sb.valid = 1;
  if (disk_write_block(1, (char *)&sb) == DISK_FAIL) {
    Printf("Fatal Error: Superblock init fail\n");
    return;
  }

  Printf("fdisk (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);

  //dfs_invalidate(); //@Michael: signal the os when close the file system, do not wipe the dirty data in kernel memory to disk
}

//----------------------------------------------------------------------
//@Michael
//Parameter: blocknum - fs block
//Return: DISK_BLOCKSIZE when succeed,  DISK_FAIL(-1) when fail
//----------------------------------------------------------------------
int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {
  // STUDENT: put your code here
  // int i;
  // char* ptr;
  // ptr = (char *)b;
  // //Printf("Start blocks is: %d\n", fsBlockStartToPhysicalBlockStart(blocknum));
  // for (i = 0; i < fdiskFactor(); i++) {
  //   if (disk_write_block(i + fsBlockStartToPhysicalBlockStart(blocknum), ptr) == DISK_FAIL) {
  //     return DISK_FAIL;
  //   }
  //   ptr = ptr + DISK_BLOCKSIZE;
  // }
  // return DISK_SUCCESS;

    int dbsize = disk_blocksize();
  // buffer for storing the data converted from dfs_block to char
  //phsical/virtual block size ratio
  uint32 pvr = DFS_BLOCKSIZE/dbsize;
  uint32 diskblock_idx;
  int i;
  char* addr = (char*) b;
 
  for(i=0; i<pvr; i++){
    // compute the corresponding physical disk number
    diskblock_idx = pvr * blocknum + i;
    if(disk_write_block(diskblock_idx, addr) == DISK_FAIL)
    {   
        Printf("  ERROR: fail to write in physdisk block %d!\n");
        return DISK_FAIL;
    }
    addr += dbsize;
  }
  return DISK_SUCCESS;

}

int fdiskFactor() {
  if (sb.fdisk_blocksize % DISK_BLOCKSIZE != 0) {
    Printf("FATAL ERROR: file system disk zize not a multiple of physical disk size\n");
  }
  return sb.fdisk_blocksize / DISK_BLOCKSIZE;
}

int fsBlockStartToPhysicalBlockStart(int fs_block_start) {
  return fdiskFactor() * fs_block_start;
}

int dataStartBlock() {
  //64 * 1024 * 1024 / 512 = 131072 fs blocks
  //fbv: 4096 words
  //512 per block -> 16 word per block
  //256 blocks total needed for fbv
  return FDISK_FBV_BLOCK_START + fbvRequiredBlocks() ;
}

int fbvRequiredBlocks() { // 32 fs blocks
  if (DFS_FBV_MAX_NUM_WORDS * 4 % 512 != 0) {
    Printf("Fatal Error: FBV is not dividable\n");
  }
  if (DFS_FBV_MAX_NUM_WORDS * 4 / 512 != 32) { //each word is 4 byte
    Prinf("Fatal Error: FBV not take 256 blocks as we expected\n");
  }
  return DFS_FBV_MAX_NUM_WORDS * 4 / 512;
}