#include "usertraps.h"
#include "misc.h"
#include "files_user.h"

void printBuffer(int* buffer, int len) {
	int i;
	for (i = 0; i < len; i++) {
		Printf("buffer at %d, value is: %d\n", i, buffer[i]);
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

void main (int argc, char *argv[])
{
  int i;
  int file_handle = 0;
  char *filename = "files_layer_test";
  int fail_flag = 0;
  char* read_only = "r";
  char* write_only = "w";
  char* read_write = "rw";
  int buffer_write[1024];
  int buffer_read[1024];

//----------------------------------------------------------------
// TO PCW
// Testcase16: (File Write and Read Test)
// First read then write
// small size file (within 10 * 512 byte)
//----------------------------------------------------------------
if (file_handle = file_open(filename, read_write) == -1){
  Printf("Testcase16 Failed at open file\n");
	fail_flag = 1;
} 
if (file_write(file_handle, (void *)buffer_write, 1436) != 1436){
  Printf("Testcase16 Failed at write\n");
	fail_flag = 1;
}
if (file_read(file_handle, (void *)buffer_read, 1436) != 1436){
  Printf("Testcase16 Failed at read\n");
	fail_flag = 1;
} 
file_seek(file_handle, -1436, FILE_SEEK_CUR);
if (!sameBuffer(buffer_write, buffer_read, 359)) { //int len 1436 / 4 = 359
  Printf("Testcase16 Failed at buffer comparison\n");
  fail_flag = 1; 
}

if (!fail_flag) {
  Printf("Testcase16 passed\n");	
}
fail_flag = 0;

//----------------------------------------------------------------
// TO PCW
// Testcase17: (File Write, Read)
// mid size file (between 10 * 512 byte - 138 * 512 byte)
//----------------------------------------------------------------
if (file_delete(filename) == -1){
  Printf("Testcase17 Failed at delete file\n");
  fail_flag = 1;
} 
if (file_handle = file_open(filename, read_write) == -1){
  Printf("Testcase17 Failed at open file\n");
	fail_flag = 1;
} 
Printf("Testcase17 file handle is %d\n", file_handle);
for (i = 0; i < 2; i++) {
  if (file_write(file_handle, (void *)buffer_write, 4096) != 4096){
    Printf("Testcase17 Failed at write #%d\n", i);
  	fail_flag = 1;
  }

  if (file_read(file_handle, (void *)buffer_read, 4096) != 4096){
    Printf("Testcase17 Failed at read #%d\n", i);
  	fail_flag = 1;
  }
  file_seek(file_handle, -4096, FILE_SEEK_CUR);
  if (!sameBuffer(buffer_write, buffer_read, 1024)) { //1024 int size
    Printf("Testcase17 Failed at buffer comparison #%d\n", i);
    fail_flag = 1; 
  }
}

//try to write to indirect blocks
for (i = 0; i < 2; i++) {
  if (file_write(file_handle, (void *)buffer_write, 4096) != 4096){
    Printf("Testcase17 Failed at indirect blocks write #%d\n", i);
    fail_flag = 1;
  }

  if (file_read(file_handle, (void *)buffer_read, 4096) != 4096){
    Printf("Testcase17 Failed at indirect blocks read #%d\n", i);
    fail_flag = 1;
  }
  if (i == 1) {
    file_seek(file_handle, -4096, FILE_SEEK_CUR);
    if (!sameBuffer(buffer_write, buffer_read, 1024)) { //1024 int size
      Printf("Testcase17 Failed at indirect blocks buffer comparison #%d\n", i);
      fail_flag = 1; 
    }
  }
}

if (!fail_flag) {
  Printf("Testcase17 passed\n");	
}
fail_flag = 0;
//----------------------------------------------------------------
// TO PCW
// Testcase18: (File Write, Read)
// big size file (above 138 * 512 byte)
// 138 * 512 / 4096 = 17.25
//----------------------------------------------------------------
if (file_delete(filename) == -1){
  Printf("Testcase17 Failed at delete file\n");
  fail_flag = 1;
} 
if (file_handle = file_open(filename, read_write) == -1){
  Printf("Testcase18 Failed at open file\n");
	fail_flag = 1;
} 
//try to write to indirect blocks
for (i = 0; i < 20; i++) {
  if (file_write(file_handle, (void *)buffer_write, 4096) != 4096){
    Printf("Testcase18 Failed at write #%d\n", i);
    fail_flag = 1;
  }

  if (file_read(file_handle, (void *)buffer_read, 4096) != 4096){
    Printf("Testcase18 Failed at read #%i\n", i);
    fail_flag = 1;
  }
  if (i == 1) {
    file_seek(file_handle, -4096, FILE_SEEK_CUR);
    if (!sameBuffer(buffer_write, buffer_read, 1024)) { //1024 int size
      Printf("Testcase18 Failed at buffer comparison #%d\n", i);
      fail_flag = 1; 
    }
  }
}
if (file_delete(filename) == -1){
  Printf("Testcase18 Failed at delete file\n");
	fail_flag = 1;
} 
if (!fail_flag) {
  Printf("Testcase18 passed\n");	
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
if (file_handle = file_open(filename, read_write) == -1){
  Printf("Testcase19 Failed at open file\n");
	fail_flag = 1;
} 
// Move to an arbitrary place
file_seek(file_handle, 11111, FILE_SEEK_CUR);
//try to write to indirect blocks
for (i = 0; i < 20; i++) {
  if (file_write(file_handle, (void *)buffer_write, 4096) != 4096){
    Printf("Testcase19 Failed at write #%d\n", i);
    fail_flag = 1;
  }

  if (file_read(file_handle, (void *)buffer_read, 4096) != 4096){
    Printf("Testcase19 Failed at read #%i\n", i);
    fail_flag = 1;
  }
  if (i == 1) {
    file_seek(file_handle, -4096, FILE_SEEK_CUR);
    if (!sameBuffer(buffer_write, buffer_read, 1024)) { //1024 int size
      Printf("Testcase19 Failed at buffer comparison #%d\n", i);
      fail_flag = 1; 
    }
  }
}
if (file_delete(filename) == -1){
  Printf("Testcase19 Failed at delete file\n");
	fail_flag = 1;
} 
if (!fail_flag) {
  Printf("Testcase19 passed\n");	
}
fail_flag = 0;
file_handle = 0;
//----------------------------------------------------------------
// TO PCW
// Testcase20: (File Close Test)
// any operation on close file is not allowed and should not flush
// into disk
//----------------------------------------------------------------
if (file_handle = file_open(filename, read_write) == -1) {
	Printf("Testcase20 Failed at open file\n");
	fail_flag = 1;	
}
if (file_close(file_handle) == -1) {
	Printf("Testcase20 Failed at close file\n");
	fail_flag = 1;
}
if (file_write(file_handle, buffer_write, 10) != -1) {
	Printf("Testcase20 Failed at write file after it is closed\n");
	fail_flag = 1;
}
if (file_read(file_handle, buffer_read, 10) != -1) {
	Printf("Testcase20 Failed at read file after it is closed\n");
	fail_flag = 1;
}
if (file_seek(file_handle, 1436, FILE_SEEK_CUR) != -1) {
	Printf("Testcase20 Failed at seek file after it is closed\n");
	fail_flag = 1;
}
if (file_delete(filename) != -1) {
	Printf("Testcase20 Failed at delete file after it is closed\n");
	fail_flag = 1;
}
if (file_delete(filename) != -1) {
	Printf("Testcase20 Failed at delete file after it is closed\n");
	fail_flag = 1;
} 
if (!fail_flag) {
	Printf("Testcase20 Passed\n");
}
fail_flag = 0;
file_handle = 0;

//----------------------------------------------------------------
// TO PCW
// Testcase21: (Authorization Test)
// write on read only file, read on write only file
//----------------------------------------------------------------


file_handle = file_open(filename, write_only);
if (file_handle == -1) {
  printf("Testcase21 Failed 5\n");
  fail_flag = 1;  
}
if (file_write(file_handle, buffer_write, 10) == -1) {
  Printf("Testcase21 Failed 6\n");
  fail_flag = 1;
}
if (file_read(file_handle, buffer_read, 10) != -1) {
  Printf("Testcase21 Failed 7\n");
  fail_flag = 1;
}
if (file_close(file_handle) == -1) {
  printf("Testcase21 Failed 8\n");
  fail_flag = 1;
} 

if (file_handle = file_open(filename, read_only) == -1) {
	Printf("Testcase21 Failed 1\n");
	fail_flag = 1;	
}
if (file_write(file_handle, buffer_write, 10) != -1) {
	Printf("Testcase21 Failed 2\n");
	fail_flag = 1;
}
if (file_read(file_handle, buffer_read, 10) == -1) {
	Printf("Testcase21 Failed 3\n");
	fail_flag = 1;
} 
if (file_close(file_handle) == -1) {
	Printf("Testcase21 Failed 4\n");
	fail_flag = 1;
}

if (!fail_flag) {
	Printf("Testcase21 Passed\n");
}
fail_flag = 0;
file_handle = 0;
}
