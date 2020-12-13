#ifndef __FILES_SHARED__
#define __FILES_SHARED__

#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3

#define FILE_MAX_FILENAME_LENGTH 76

#define FILE_MAX_READWRITE_BYTES 4096

typedef struct file_descriptor {
  // STUDENT: put file descriptor info here
  int inuse; //0-free, 1-used (4 byte)
  char file_name[FILE_MAX_FILENAME_LENGTH]; // same length as it in dfs
  int inode_handle;
  unsigned int current_position; // Initial value 0 
  int end_of_file; // Initial value 0, end of file 1
  char *mode; // 'r': 1, 'w': 2, 'wr': 3
  int user_pid;
} file_descriptor;

#define FILE_FAIL -1
#define FILE_EOF -1
#define FILE_SUCCESS 1

#endif
