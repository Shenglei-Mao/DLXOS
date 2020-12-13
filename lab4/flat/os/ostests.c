#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"
#include "files.h"

void printBuffer(int* buffer, int len) {
	int i;
	for (i = 0; i < len; i++) {
		printf("buffer at %d, value is: %d\n", i, buffer[i]);
	}
	return;
}

int sameBuffer(int* buffer1, int* buffer2, int len) {
	int i;
	for (i = 0; i < len; i++) {
		if (buffer1[i] != buffer2[i]) {
			return 0;
		}
	}
	return 1;
}

int ifZeros(int* buf, int len) {
	int i;
	for (i = 0; i < len; i++) {
		if (buf[i] != 0) {
			return 0;
		}
	}
	return 1;
}

void initBuffer(int* buf, int len) {
  int i;
  for (i = 0; i < len; i++) {
  	buf[i] = i % 10;
  }	
}

void RunOSTests() {
  int i;
  int buf_disk_write[128];
  int buf_disk_read[128];
  int buf_nonalign_disk_write[110];
  int buf_nonalign_disk_read[110];
  int result;
  int result_next;
  int result_realloc;
  int fail_flag = 0;

  char* filename = "file";
  char* postfix;
  uint32 handle;
// STUDENT: run any os-level tests here
//----------------------------------------------------------------
// Test For DFS Layer
//---------------------------------------------------------------- 
  //----------------------------------------------------------------
  // @Michael (Testcase1: DFS reopen a fs, should not open twice, should get message in output log)
  //----------------------------------------------------------------
  // DfsOpenFileSystem();
  // printf("Testcase1 Passed\n"); //Manually Tested

  // //----------------------------------------------------------------
  // // @Michael (Testcase2: FS Close Function Test, should get close success message)
  // //----------------------------------------------------------------
  // DfsCloseFileSystem();
  // // Reopen after close the fs system (check log for message)
  // DfsOpenFileSystem();
  // printf("Testcase2 Passed\n"); //Manually Tested

  // //----------------------------------------------------------------
  // // @Michael (Testcase3: Alignment Write Read Testing Block 250, and also check blockscript)
  // //----------------------------------------------------------------
  // // Note: Physical Block 74 - 137 (All inclusive) is reserved for FBV, Data Block Start from 138
  // // 512 byte -> 128 int
  // // init physical block buffer
  // for (i = 0; i < 128; i++) {
  // 	buf_disk_write[i] = i % 10;
  // }
  // DfsWriteBlock(250, (dfs_block* )buf_disk_write);
  
  // DfsReadBlock(250, (dfs_block* )buf_disk_read);
  // //printBuffer(buf_disk_read, 128);

  // if (!(sameBuffer(buf_disk_write, buf_disk_read, 128))) {
  // 	printf("Testcase3 Failed\n");
  // } else {
  // 	printf("Testcase3 Passed\n");
  // }

  // //----------------------------------------------------------------
  // // @Michael (Testcase4: Non-Alignment Write Read Testing
  // //----------------------------------------------------------------
  // for (i = 0; i < 110; i++) {
  // 	buf_nonalign_disk_write[i] = i % 10;
  // }
  // DfsWriteBlock(250, (dfs_block* )buf_nonalign_disk_write);
  
  // DfsReadBlock(250, (dfs_block* )buf_nonalign_disk_read);

  // if (!(sameBuffer(buf_nonalign_disk_write, buf_nonalign_disk_read, 110))) {
  // 	printf("Testcase4 Failed\n");
  // } else {
  // 	printf("Testcase4 Passed\n");
  // }

  // //----------------------------------------------------------------
  // // @Michael (Testcase5: Persistent Testing for read write)
  // //----------------------------------------------------------------
  // DfsCloseFileSystem();
  // DfsOpenFileSystem();
  // DfsReadBlock(250, (dfs_block* )buf_nonalign_disk_read);
  // if (!(sameBuffer(buf_nonalign_disk_write, buf_nonalign_disk_read, 110))) {
  // 	printf("Testcase5 Failed\n");
  // } else {
  // 	printf("Testcase5 Passed\n");
  // }

  // //----------------------------------------------------------------
  // // @Michael (Testcase6: DFSAllocate and DFSFREE testing)
  // //----------------------------------------------------------------
  // // 37 inode start, 32 fs block for fbv
  // // 37 + 31 = 68, 69 is data start block
  // // safe if not messed up with the meta-data block
  // result = DfsAllocateBlock();
  // if (result < 69) {
  // 	printf("Testcase6 Failed. Block is %d\n", result);
  // 	fail_flag = 1;
  // }
  // printf("The first allocated fs block is %d\n", result);
  // result_next = DfsAllocateBlock();
  // if (result_next != result + 1) {
  // 	printf("Testcase6 Failed. Block is %d\n", result);
  // 	fail_flag = 1;
  // }
  // //free two allocate blocks
  // DfsFreeBlock(result);
  // result_realloc = DfsAllocateBlock();
  // if (result_realloc != result) {
  // 	printf("Testcase6 Failed\n");
  // 	fail_flag = 1;
  // }
  // DfsFreeBlock(result);
  // DfsFreeBlock(result_next);
  // result_realloc = DfsAllocateBlock();
  // if (result_realloc != result) {
  // 	printf("Testcase6 Failed\n");
  // 	fail_flag = 1;
  // }
  // result_realloc = DfsAllocateBlock();
  // if (result_realloc != result_next) {
  // 	printf("Testcase6 Failed");
  // 	fail_flag = 1;
  // }
  // if (!fail_flag) {
  // 	printf("Testcase6 Passed\n");
  // }
  // fail_flag = 0;
  // //back to factory disk
  // DfsFreeBlock(result);
  // DfsFreeBlock(result_next);

  // //----------------------------------------------------------------
  // // @Michael (Testcase7: Persistent Testing for alloc and free)
  // //----------------------------------------------------------------
  // result = DfsAllocateBlock();
  // DfsCloseFileSystem();
  // DfsOpenFileSystem();
  // result_next = DfsAllocateBlock();
  // if (result_next != result + 1) {
  // 	printf("Testcase7 Failed\n");
  // } else {
  // 	printf("Testcase7 passed\n");
  // }
  // //back to factory disk
  // DfsFreeBlock(result);
  // DfsFreeBlock(result_next);

  // //----------------------------------------------------------------
  // // @Michael (Testcase8: free a non allocate block and result fail)
  // //----------------------------------------------------------------
  // if (DfsFreeBlock(result) != -1) {
  // 	printf("Testcase8 Failed\n");
  // } else {
  // 	printf("Above fatal error is triggered on purpose. Testcase8 Passed\n");
  // }

  // //----------------------------------------------------------------
  // // @Michael (Testcase9: Disk Free Should wipe out all the current content)
  // // That's because it may be used later for indirect and two level blocks
  // //----------------------------------------------------------------
  // result = DfsAllocateBlock();
  // DfsWriteBlock(result, (dfs_block* )buf_disk_write);
  // DfsFreeBlock(result);
  // DfsReadBlock(result, (dfs_block* )buf_disk_read);
  // if (!ifZeros(buf_disk_read, 128)) {
  // 	printf("Testcase9 Failed\n");
  // } else {
  // 	printf("Testcase9 Passed\n");
  // }



//----------------------------------------------------------------
// @Michael: All Tests for DFS Layer Passed 
//---------------------------------------------------------------- 

//----------------------------------------------------------------
// @Michael: Test For Inode Layer
//---------------------------------------------------------------- 

  //----------------------------------------------------------------
  // @Michael (Testcase10: filename exist, open and delete test)
  // first max 192 inode, allocate 1 file and free 1 file and check for exist
  // and then loop over 200 times
  // check number of inuse inode should be 0
  // this testcase does not actually test success free on disk block
  //----------------------------------------------------------------
 //  printf("Testcase10 init done\n");
 //  for (i = 0; i < 200; i++) {
	// if ((handle = DfsInodeOpen(filename)) == -1) {
	// 	printf("Testcase10 Failed\n");
	// 	fail_flag = 1;
	// }
	// // printf("Inode handle is: %d\n", handle);
	// if (DfsInodeFilenameExists(filename) != handle) {
	// 	printf("Testcase10 Failed at first Exist\n");
	// 	fail_flag = 1;	
	// }
	// if (DfsInodeDelete(handle) == -1) {
	// 	printf("Testcase10 Failed\n");
	// 	fail_flag = 1;
	// }
	// if ((handle = DfsInodeFilenameExists(filename)) != -1) {
	// 	printf("Testcase10 Failed at second Exsit, file exist at %d\n", handle);
	// 	fail_flag = 1;	
	// }
 //  }
 //  if (!fail_flag) {
 //  	printf("Testcase10 Passed\n");
 //  }
 //  fail_flag = 0;



  inode_read_write_test();

}

void inode_read_write_test() {
  int inode_handle;
  int buffer_write[1024];
  int buffer_read[1024];
  int fail_flag;
  char *filename = "test_file";
  int result;
  int file_handle;
  char* read_only = "r";
	char* write_only = "w";
  char* read_write = "rw";
  int i;

  fail_flag = 0;
  initBuffer(buffer_write, 1024);
  //----------------------------------------------------------------
  // @Michael (Testcase11: DfsInodeReadBytes and DfsInodeWriteBytes 
  //           Testing on direct blocks, small files testing)
  // Double run it could also test persistent for FBV and INODE
  // Yes, Persistent test passed, but since we have previous free
  // testcase, it will wipe out fs block 69, write will re-allocate,
  // which is the correct behaviour
  //----------------------------------------------------------------
  // For small file, only using virtual block 0-9 (both inclusive)
 //  if ((inode_handle = DfsInodeOpen(filename)) == -1) {
	// printf("Testcase11 Failed at inode open\n");
	// fail_flag = 1;
 //  }
 //  //359 int, 356 * 4 = 1436 byte, 1436 / 512 = 2.8 fs blocks
 //  //lets make start byte 512 byte, from 1st fs block
 //  if (DfsInodeWriteBytes(inode_handle, (void *)buffer_write, 512, 1436) != 1436) {
	// printf("Testcase11 Failed at inode write\n");
	// fail_flag = 1;
 //  }
 //  if (DfsInodeReadBytes(inode_handle, (void *)buffer_read, 512, 1436) != 1436) {
	// printf("Testcase11 Failed at inode read\n");
	// fail_flag = 1;
 //  }
 //  if (!sameBuffer(buffer_write, buffer_read, 359)) {
	// printf("Testcase11 Failed read is not what I write to\n");
	// fail_flag = 1;
 //  }

 //  if (!fail_flag) {
 //  	printf("Testcase11 passed\n");	
 //  }
 //  fail_flag = 0;


  //----------------------------------------------------------------
  // @Michael (Testcase12: Persistent Testing on Inode relavent info)
  //----------------------------------------------------------------
 //  DfsCloseFileSystem();
 //  DfsOpenFileSystem();
 //  if (DfsInodeReadBytes(inode_handle, (void *)buffer_read, 512, 1436) != 1436) {
	// printf("Testcase12 Failed at inode read\n");
	// fail_flag = 1;
 //  }
 //  if (!sameBuffer(buffer_write, buffer_read, 359)) {
	// printf("Testcase12 Failed read is not what I write to\n");
	// fail_flag = 1;
 //  }
 //  if (!fail_flag) {
 //  	printf("Testcase12 passed\n");	
 //  }
 //  fail_flag = 0;


 //  //----------------------------------------------------------------
 //  // @Michael (Testcase13: file size test)
 //  //----------------------------------------------------------------
 //  if ((result = DfsInodeFilesize(inode_handle)) != 512 + 1436) {
	// printf("Testcase13 failed, filesize is %d. But is it is not your first time run the testing code, then Testcase13 passed! Since we are incrementally write to this file:)\n", result);	
 //  } else {
 // 	 printf("Testcase13 passed\n");	
 //  }


  //----------------------------------------------------------------
  // @Michael (Testcase13: DfsInodeReadBytes and DfsInodeWriteBytes 
  //           Testing on indirect blocks, mid-size files testing)
  //----------------------------------------------------------------
  
 //  if ((inode_handle = DfsInodeOpen(filename)) == -1) {
	// printf("Testcase13 Failed at inode open\n");
	// fail_flag = 1;
 //  }
 //  //359 int, 356 * 4 = 1436 byte, 1436 / 512 = 2.8 fs blocks
 //  //lets make start byte 512 byte, from 1st fs block
 //  if (DfsInodeWriteBytes(inode_handle, (void *)buffer_write, 512 * 20, 1436) != 1436) {
	// printf("Testcase13 Failed at inode write\n");
	// fail_flag = 1;
 //  }
 //  if (DfsInodeReadBytes(inode_handle, (void *)buffer_read, 512 * 20, 1436) != 1436) {
	// printf("Testcase13 Failed at inode read\n");
	// fail_flag = 1;
 //  }
 //  if (!sameBuffer(buffer_write, buffer_read, 359)) {
	// printf("Testcase13 Failed read is not what I write to\n");
	// fail_flag = 1;
 //  }

 //  if (!fail_flag) {
 //  	printf("Testcase13 passed\n");	
 //  }
 //  fail_flag = 0;

  //----------------------------------------------------------------
  // @Michael (Testcase14: DfsInodeReadBytes and DfsInodeWriteBytes 
  //           Testing on double indirect blocks, large files testing)
  //----------------------------------------------------------------
  
 //  if ((inode_handle = DfsInodeOpen(filename)) == -1) {
	// printf("Testcase14 Failed at inode open\n");
	// fail_flag = 1;
 //  }
 //  //359 int, 356 * 4 = 1436 byte, 1436 / 512 = 2.8 fs blocks
 //  //lets make start byte 512 byte, from 1st fs block
 //  if (DfsInodeWriteBytes(inode_handle, (void *)buffer_write, 512 * (total_direct_blocks() + total_indirect_blocks() + 20), 1436) != 1436) {
	// printf("Testcase14 Failed at inode write\n");
	// fail_flag = 1;
 //  }
 //  if (DfsInodeReadBytes(inode_handle, (void *)buffer_read, 512 * (total_direct_blocks() + total_indirect_blocks() + 20), 1436) != 1436) {
	// printf("Testcase14 Failed at inode read\n");
	// fail_flag = 1;
 //  }
 //  if (!sameBuffer(buffer_write, buffer_read, 359)) {
	// printf("Testcase14 Failed read is not what I write to\n");
	// fail_flag = 1;
 //  }

 //  if (!fail_flag) {
 //  	printf("Testcase14 passed\n");	
 //  }
 //  fail_flag = 0;

//----------------------------------------------------------------
// @Michael: All Tests for INODE Layer Passed
//---------------------------------------------------------------- 


//----------------------------------------------------------------
// @Michael: Tests for File Layer (First test in kernel, then run in user prog!)
//---------------------------------------------------------------- 
  // FileModuleInit();

//----------------------------------------------------------------
// TO PCW
// Testcase15: (FileOpen Test by two different process)
// Open two files by two different process, second process should fail 
// (Remember how fork works, but you can also directly implement it as user prog)
//---------------------------------------------------------------- 



//----------------------------------------------------------------
// TO PCW
// Testcase16: (File Write and Read Test)
// First read then write
// small size file (within 10 * 512 byte)
//----------------------------------------------------------------
if (file_handle = FileOpen(filename, read_write) == -1){
  printf("Testcase16 Failed at open file\n");
	fail_flag = 1;
} 
if (FileWrite(file_handle, (void *)buffer_write, 1436) != 1436){
  printf("Testcase16 Failed at write\n");
	fail_flag = 1;
}

if (FileRead(file_handle, (void *)buffer_read, 1436) != 1436){
  printf("Testcase16 Failed at read\n");
	fail_flag = 1;
} 
FileSeek(file_handle, -1436, FILE_SEEK_CUR);
if (!sameBuffer(buffer_write, buffer_read, 359)) { //int len 1436 / 4 = 359
  printf("Testcase16 Failed at buffer comparison\n");
  fail_flag = 1; 
}
if (!fail_flag) {
  printf("Testcase16 passed\n");	
}
fail_flag = 0;

//----------------------------------------------------------------
// TO PCW
// Testcase17: (File Write, Read)
// mid size file (between 10 * 512 byte - 138 * 512 byte)
//----------------------------------------------------------------
if (FileDelete(filename) == -1){
  printf("Testcase17 Failed at delete file\n");
	fail_flag = 1;
} 
if (file_handle = FileOpen(filename, read_write) == -1){
  printf("Testcase17 Failed at open file\n");
	fail_flag = 1;
} 
for (i = 0; i < 2; i++) {
  if (FileWrite(file_handle, (void *)buffer_write, 4096) != 4096){
    printf("Testcase17 Failed at write\n");
  	fail_flag = 1;
  }

  if (FileRead(file_handle, (void *)buffer_read, 4096) != 4096){
    printf("Testcase17 Failed at read\n");
  	fail_flag = 1;
  }
  FileSeek(file_handle, -4096, FILE_SEEK_CUR);
  if (!sameBuffer(buffer_write, buffer_read, 1024)) { //1024 int size
    printf("Testcase17 Failed at buffer comparison\n");
    fail_flag = 1; 
  }
}

//try to write to indirect blocks
for (i = 0; i < 2; i++) {
  if (FileWrite(file_handle, (void *)buffer_write, 4096) != 4096){
    printf("Testcase17 Failed at write\n");
    fail_flag = 1;
  }

  if (FileRead(file_handle, (void *)buffer_read, 4096) != 4096){
    printf("Testcase17 Failed at read\n");
    fail_flag = 1;
  }
  if (i == 1) {
    FileSeek(file_handle, -4096, FILE_SEEK_CUR);
    if (!sameBuffer(buffer_write, buffer_read, 1024)) { //1024 int size
      printf("Testcase17 Failed at buffer comparison\n");
      fail_flag = 1; 
    }
  }
}

if (!fail_flag) {
  printf("Testcase17 passed\n");	
}
fail_flag = 0;

//----------------------------------------------------------------
// TO PCW
// Testcase18: (File Write, Read)
// big size file (above 138 * 512 byte)
// 138 * 512 / 4096 = 17.25
//----------------------------------------------------------------
if (FileDelete(filename) == -1){
  printf("Testcase18 Failed at delete file\n");
	fail_flag = 1;
} 
if (file_handle = FileOpen(filename, read_write) == -1){
  printf("Testcase18 Failed at open file\n");
	fail_flag = 1;
} 
//try to write to indirect blocks
for (i = 0; i < 20; i++) {
  if (FileWrite(file_handle, (void *)buffer_write, 4096) != 4096){
    printf("Testcase18 Failed at write\n");
    fail_flag = 1;
  }

  if (FileRead(file_handle, (void *)buffer_read, 4096) != 4096){
    printf("Testcase18 Failed at read\n");
    fail_flag = 1;
  }
  if (i == 1) {
    FileSeek(file_handle, -4096, FILE_SEEK_CUR);
    if (!sameBuffer(buffer_write, buffer_read, 1024)) { //1024 int size
      printf("Testcase18 Failed at buffer comparison\n");
      fail_flag = 1; 
    }
  }
}
if (!fail_flag) {
  printf("Testcase18 passed\n");	
}
fail_flag = 0;

//----------------------------------------------------------------
// TO PCW
// Testcase19: (File Seek Test)
// move the cursor to an arbitrary place then start write
// read what you write, compare the two buffer (of course you need
// to compare from write with read buffer and offset, ie how much you
// shifted and try cover both positive seek and negtive seek case)
//---------------------------------------------------------------- 
  // testcase19();

//----------------------------------------------------------------
// TO PCW
// Testcase20: (File Close Test)
// any operation on close file is not allowed and should not flush
// into disk
//---------------------------------------------------------------- 
  testcase20();

//----------------------------------------------------------------
// TO PCW
// Testcase21: (Authorization Test)
// write on read only file, read on write only file
//---------------------------------------------------------------- 
  testcase21();

}

void testcase19() {
  // int file_handle;
  // char* filename = "brand_new_file";
  // char* read_write = "rw";
  // int fail_flag = 0;
  // int buffer_write[1024];
  // int buffer_read[1024];

  // if (file_handle = FileOpen(filename, read_write) == -1){
  // printf("Testcase16 Failed at open file\n");
	// fail_flag = 1;
  // } 
  // if (FileWrite(file_handle, (void *)buffer_write, 1436) != 1436){
  //   printf("Testcase16 Failed at write\n");
  //   fail_flag = 1;
  // }
  // if (FileRead(file_handle, (void *)buffer_read, 1436) != 1436){
  //   printf("Testcase16 Failed at read\n");
  //   fail_flag = 1;
  // } 
  // FileSeek(file_handle, -1436, FILE_SEEK_CUR);
  // if (!sameBuffer(buffer_write, buffer_read, 359)) { //int len 1436 / 4 = 359
  //   printf("Testcase16 Failed at buffer comparison\n");
  //   fail_flag = 1; 
  // }
  // if (!fail_flag) {
  //   printf("Testcase16 passed\n");	
  // }
  // fail_flag = 0;
}

void testcase20() {
  char* filename = "brand_new_file";
	int file_handler;
  char* trash_buffer;
	char *mode = "rw";
	int fail_flag;

  fail_flag = 0;
	file_handler = FileOpen(filename, mode);
	if (file_handler == -1) {
		printf("Testcase20 Failed at open file\n");
		fail_flag = 1;	
	}
	if (FileClose(file_handler) == -1) {
		printf("Testcase20 Failed at close file\n");
		fail_flag = 1;
	}
	if (FileWrite(file_handler, trash_buffer, 10) != -1) {
		printf("Testcase20 Failed at write file after it is closed\n");
		fail_flag = 1;
	}
	if (FileRead(file_handler, trash_buffer, 10) != -1) {
		printf("Testcase20 Failed at read file after it is closed\n");
		fail_flag = 1;
	}
	if (FileSeek(file_handler, 1436, FILE_SEEK_CUR) != -1) {
		printf("Testcase20 Failed at seek file after it is closed\n");
		fail_flag = 1;
	}
	if (FileDelete(filename) != -1) {
		printf("Testcase20 Failed at delete file after it is closed\n");
		fail_flag = 1;
	}
	if (!fail_flag) {
		printf("Testcase20 Passed\n");
	}
  fail_flag = 0;
}

void testcase21() {
	char* filename = "another_brand_new_file";
	char* read_only = "r";
	char* write_only = "w";
	char* trash_buffer;
	int file_handler;
	int fail_flag;

	fail_flag = 0;
	file_handler = FileOpen(filename, write_only);
  if (file_handler == -1) {
    printf("Testcase21 Failed 1\n");
    fail_flag = 1;  
  }
	if (FileWrite(file_handler, trash_buffer, 10) == -1) {
		printf("Testcase21 Failed 2\n");
		fail_flag = 1;
	}
	if (FileRead(file_handler, trash_buffer, 10) != -1) {
		printf("Testcase21 Failed 3\n");
		fail_flag = 1;
	} 
  if (FileClose(file_handler) == -1) {
    printf("Testcase21 Failed 4\n");
    fail_flag = 1;
  }

  file_handler = FileOpen(filename, read_only);
  if (file_handler == -1) {
    printf("Testcase21 Failed 5\n");
    fail_flag = 1;  
  }
  if (FileWrite(file_handler, trash_buffer, 10) != -1) {
    printf("Testcase21 Failed 6\n");
    fail_flag = 1;
  }
  if (FileRead(file_handler, trash_buffer, 10) == -1) {
    printf("Testcase21 Failed 7\n");
    fail_flag = 1;
  } 
  if (FileClose(file_handler) == -1) {
    printf("Testcase21 Failed 8\n");
    fail_flag = 1;
  }
	if (!fail_flag) {
		printf("Testcase21 Passed\n");
	}
  fail_flag = 0;
}