#ifndef __DFS_H__
#define __DFS_H__

#include "dfs_shared.h"

#define PHYSICAL_DISK_SUPER_BLOCK 1
#define UNINIT 0

// @Michael

//DFS Related Function
void DfsModuleInit();

void DfsInvalidate();
void DfsValidate();

int DfsOpenFileSystem();
int readSuperBlock();
int readInode();
int readFBV();
int writeBackSuperBlock();
int fbvRequiredBlocks();

int DfsCloseFileSystem();
int writeBackInode();
int writeBackFBV();

//DFS Layer
int DfsWriteBlock(uint32 blocknum, dfs_block *b); //-1 on fail, total byte on success
int DfsReadBlock(uint32 blocknum, dfs_block *b); //-1 on fail, total byte on success
uint32 DfsAllocateBlock(); //-1 on fail, fs block num otherwise, only play with fbv, not gonna init content
int DfsFreeBlock(uint32 blocknum); //-1 on fail, DFS_SUCCESS otherwise, play with fbv and wipe out all the content

//Inode Layer
uint32 DfsInodeFilenameExists(char *filename); //-1 on fail, inode handle
uint32 DfsInodeOpen(char *filename);
int DfsInodeDelete(uint32 handle); //-1 on fail, DFS_SUCCESS 
uint32 DfsInodeFilesize(uint32 handle);
int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes); //-1 on fail, num_byte is success
int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes); //-1 on fail, num_byte is success
uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum);
uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum);

//Multiple Helper Functions
//DFS Layer
int fdiskFactor(); //at our lab, this is 2, 512MB(fs) / 256MB(physical)
int fsBlockStartToPhysicalBlockStart(int fs_block_start); //as name suggested
int dataStartBlock();
int physicalToFsdisk(uint32 blocknum); //input: a physical block, output: start of fs block index
//Inode Layer
int numInuseInode();
int deleteAllInode();
int inodeMaxBlocks();
int atDirect(int vblock); //return true(1) if the virtual block is direct block
int atSingleIndirect(int vblock); //return true(1) if the virtual block is single direct block
int total_direct_blocks();
int total_indirect_blocks();
int total_double_indirect_blocks();
int get_level1_index_for_double_indirect(int vblock);
int get_level2_index_for_double_indirect(int vblock);

#endif
