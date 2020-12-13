#ifndef __DFS_SHARED__
#define __DFS_SHARED__

//-----------------------------------------------------------------------------------------
//@Michael:
// Be sure that you minimize the amount of information that is shared between the os 
// and the user programs through dfs_shared.h if that information could be better shared 
// through the superblock.
//-----------------------------------------------------------------------------------------

typedef struct dfs_superblock {
  // STUDENT: put superblock internals here
  //----------------------------------------------------------------------------------------
  // The following items should be stored in the superblock (you may add more if you like):
  // a valid indicator for the file system
  // the file system block size
  // the total number of file system blocks
  // the starting file system block number for the array of inodes
  // the number of inodes in the inodes array
  // the starting file system block number for the free block vector.
  //----------------------------------------------------------------------------------------
	int valid; //0-invalid, 1-valid
	int fdisk_blocksize; //512 Bytes (should store unit in bytes)
	int fdisk_num_blocks; //64 * 1024 * 1024 / 512;
	int fdisk_inode_block_start; //1
	int fdisk_num_inode; //192
	int fdisk_fbv_block_start; //37
} dfs_superblock;

#define DFS_BLOCKSIZE 512  // unit: byte, Must be an integer multiple of the disk blocksize

typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;

typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 96 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 96 bytes.
  //--------------------------------------------------	
  // an in use indicator to tell if an inode is free or in use
  // the size of the file this inode represents (i.e. the maximum byte that has been written to this file)
  // the filename, which is just a string
  // a table of direct address translations for the first 10 virtual blocks
  // a block number of a file system block on the disk which holds a table of indirect address translations for the virtual blocks beyond the first 10.
  // a block number of a file system block on the disk which holds a table of double-indirect address translations for the virtual blocks beyond the first 10 and blocks under single-indirect tables.
  //---------------------------------------------------
  //@Michael: TODO check that the compiler take 0 optimization for padding a specific field in the struct
  int inuse; //0-free, 1-used (4 byte)
  int file_size; // (4 byte) 
  char file_name[40]; // Need the max length of file name (1 char 1 byte)
  int direct_block_translations[10]; //4 byte * 10 = 40 byte
  int indirect_block_translations;  //4 byte
  int double_indirect_block_translations;	//4 byte
} dfs_inode;

#define DFS_MAX_FILESYSTEM_SIZE 0x4000000  // 64MB
//TODO: Should DFS number of inode, fbv max number of word configurable?
#define DFS_INODE_MAX_NUM 192
#define DFS_FBV_MAX_NUM_WORDS DFS_MAX_FILESYSTEM_SIZE / DFS_BLOCKSIZE / 32 //4096 words

#define DFS_FAIL -1
#define DFS_SUCCESS 1

#endif
