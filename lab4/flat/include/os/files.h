#ifndef __FILES_H__
#define __FILES_H__

#include "dfs.h"
#include "files_shared.h"

//#define FILE_MAX_OPEN_FILES 15
#define FILE_R 1
#define FILE_W 2
#define FILE_RW 3

void FileModuleInit();
int FileOpen(char *filename, char *mode);
int FileClose(int handle);
int FileRead(int handle, void *mem, int num_bytes);
int FileWrite(int handle, void *mem, int num_bytes);
int FileSeek(int handle, int num_bytes, int from_where);
int FileDelete(char *filename);
int FileMode(char *mode);
int FileHandle(int inode_handle);
int FileAccessible(int file_handle);
void FileDescriptorInit(int file_handle);
int FileFindDescriptor();

#endif
