#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.
static lock_t file_lock;
static file_descriptor file[DFS_INODE_MAX_NUM];

// STUDENT: put your file-level functions here

// -----------------------------------------------------------
// FileModuleInit: Initialize the file descriptor
// -----------------------------------------------------------
void FileModuleInit(){
    int i;
    for (i = 0; i < DFS_INODE_MAX_NUM; i++){
        FileDescriptorInit(i);
        printf("FileModuleInit: FileDescriptorInit #%d!\n", i);
    }
}


// -----------------------------------------------------------
// FileOpen: open the given filename with one of three possible modes:
// "r", "w", or "rw". Return FILE_FAIL on failure (e.g., when a 
// process tries to open a file that is already open for another process), 
// and the handle of a file descriptor on success. Remember to use
// locks whenever you allocate a new file descriptor.
// -----------------------------------------------------------
int FileOpen(char *filename, char *mode){
    int inode_handle;
    int file_handle;
    int mode_num = 0;
    int i;

    // Get the inode handle according to file name
    inode_handle = DfsInodeFilenameExists(filename);
    // Get the mode
    mode_num = FileMode(mode);

    // Check if the inode exist, if yes means the file should exist
    // Get the file_handle and check if the file is used by another process
    if (inode_handle != DFS_FAIL){
        file_handle = FileHandle(inode_handle);

        if (file_handle == DFS_FAIL) {
            // if in 'w' or 'rw' mode: Creat a file according to the file name
            if (mode_num == FILE_FAIL){
                printf("FileOpen FATAL ERROR: Invalid mode!\n");
                return FILE_FAIL;
            } else if (mode_num == FILE_R || mode_num == FILE_W || mode_num == FILE_RW){
                // inode_handle = DfsInodeOpen(filename);
                // if (inode_handle == DFS_FAIL){
                //     printf("FileOpen FATAL ERROR: Write mode but cannot creat file!\n");
                //     return FILE_FAIL;
                // }
                // Set up a new file descriptor
                file_handle = FileFindDescriptor();
                if (file_handle == FILE_FAIL){
                    // Return FILE_FAIL if no usable file descriptor
                    printf("FileOpen FATAL ERROR: No usable file descriptor!\n");
                    return FILE_FAIL;
                }
                // if (if (GetCurrentPid() != file[file_handle].user_pid)){
                //     printf("FileOpen FATAL ERROR: File not accessible!\n");
                //     return FILE_FAIL;
                // }
            }
        } 
    }
    // If the inode does not exist, do actions according to the mode
    else if (inode_handle == DFS_FAIL){
        // If in 'r' mode: Return FILE_FAIL
        // if in 'w' or 'rw' mode: Creat a file according to the file name
        if (mode_num == FILE_FAIL){
            printf("FileOpen FATAL ERROR: Invalid mode!\n");
            return FILE_FAIL;
        }
        else if (mode_num == FILE_R){
            printf("FileOpen FATAL ERROR: Read mode and file inode does not exist!\n");
            return FILE_FAIL;
        }
        else if (mode_num == FILE_W || mode_num == FILE_RW){
            inode_handle = DfsInodeOpen(filename);
            if (inode_handle == DFS_FAIL){
                printf("FileOpen FATAL ERROR: Write mode but cannot creat file!\n");
                return FILE_FAIL;
            }
            // Set up a new file descriptor
            file_handle = FileFindDescriptor();
            if (file_handle == FILE_FAIL){
                // Return FILE_FAIL if no usable file descriptor
                printf("FileOpen FATAL ERROR: No usable file descriptor!\n");
                return FILE_FAIL;
            }
        }
    }
    // We get 2 important results: inode_handle and file_handle
    // printf("FileOpen: Mode: %s!\n", mode);
    // printf("FileOpen: Mode: %d!\n", mode_num);
    // Open the file and set up the file descriptor
    if (LockHandleAcquire(file_lock) != SYNC_SUCCESS);
    file[file_handle].inuse = 1;
    dstrncpy(file[file_handle].file_name, filename, dstrlen(filename));
    file[file_handle].inode_handle = inode_handle;
    file[file_handle].current_position = 0;
    file[file_handle].end_of_file = 0;
    file[file_handle].mode = mode;
    file[file_handle].user_pid = GetCurrentPid();
    if (LockHandleRelease(file_lock) != SYNC_SUCCESS);
    // printf("FileOpen: mode %s!\n", file[file_handle].mode);
    // printf("file handle is %d\n", file_handle);
    return file_handle;
}


// -----------------------------------------------------------
// FileClose: close the given file descriptor handle. 
// Return FILE_FAIL on failure, and FILE_SUCCESS on success.
// -----------------------------------------------------------
int FileClose(int handle){
    // Check the inuse flag and PID
    if (FileAccessible(handle) == FILE_FAIL){
        printf("FileClose FATAL ERROR: File not accessible!\n");
        return FILE_FAIL;
    }
    // Close the file descriptor
    if (LockHandleAcquire(file_lock) != SYNC_SUCCESS);
    FileDescriptorInit(handle);
    if (LockHandleRelease(file_lock) != SYNC_SUCCESS);
    return FILE_SUCCESS;
}


// -----------------------------------------------------------
// FileRead: read num_bytes from the open file descriptor 
// identified by handle. Return FILE_FAIL on failure or upon 
// reaching end of file, and the number of bytes read on success. 
// If end of file is reached, the end-of-file flag in the file 
// descriptor should be set.
// -----------------------------------------------------------
int FileRead(int handle, void *mem, int num_bytes){
    int mode;
    int inode_handle = -1;
    int num_bytes_read = 0;
    int current_position = 0;

    // Check the inuse flag and PID
    if (FileAccessible(handle) == FILE_FAIL){
        printf("FileRead FATAL ERROR: File not accessible!\n");
        return FILE_FAIL;
    }
    // Check the mode
    // printf("FileRead: Mode: %s!\n", file[handle].mode);
    mode = FileMode(file[handle].mode);
    // printf("FileRead: Mode: %d!\n", mode);
    // if (mode != FILE_R && mode != FILE_RW){
    //     printf("FileRead FATAL ERROR: Don't have permission to read!\n");
    //     return FILE_FAIL;
    // }
    if (mode == FILE_W){
        printf("FileRead FATAL ERROR: Don't have permission to read!\n");
        return FILE_FAIL;
    }
    // Check if the number of bytes to read is in normal range
    if (num_bytes <= 0 || num_bytes > FILE_MAX_READWRITE_BYTES){
        printf("FileRead FATAL ERROR: Number of bytes is not in normal range!\n");
        return FILE_FAIL;
    }
    // Check the end of file flag
    if (file[handle].end_of_file == 1){
        printf("FileRead FATAL ERROR: End of file!\n");
        return FILE_FAIL;
    }

    // Read num_bytes from file
    inode_handle = file[handle].inode_handle;
    current_position = file[handle].current_position;
    if (inode_handle >= 0 && inode_handle < DFS_INODE_MAX_NUM){
        if (current_position >= 0 && current_position < DfsInodeFilesize(inode_handle)){
            num_bytes_read = DfsInodeReadBytes(inode_handle, mem, current_position, num_bytes);
            if (num_bytes_read == DFS_FAIL){
                printf("FileRead FATAL ERROR: Read operation Fail!\n");
                return FILE_FAIL;
            }
            file[handle].current_position += num_bytes_read;
            if (file[handle].current_position > DfsInodeFilesize(inode_handle)){
                file[handle].end_of_file = 1;
                return FILE_EOF;
            }
            return num_bytes_read;
        }
        printf("FileRead FATAL ERROR: Current_Position is not in normal range!\n");
        return FILE_FAIL;
    }
    printf("FileRead FATAL ERROR: Inode_handle is not in normal range!\n");
    return FILE_FAIL;
}

// -----------------------------------------------------------
// FileWrite: write num_bytes to the open file descriptor 
// identified by handle. Return FILE_FAIL on failure,
// and the number of bytes written on success.
// -----------------------------------------------------------
int FileWrite(int handle, void *mem, int num_bytes){
    int mode;
    int inode_handle = -1;
    int num_bytes_written = 0;
    int current_position = 0;

    // Check the inuse flag and PID
    if (FileAccessible(handle) == FILE_FAIL){
        printf("FileWrite FATAL ERROR: File not accessible!\n");
        return FILE_FAIL;
    }
    // Check the mode
    // printf("FileWrite: file handle is %d\n", handle);
    // printf("FileWrite: Mode: %s\n", file[handle].mode);
    mode = FileMode(file[handle].mode);
    // printf("FileWrite: Mode: %d\n", mode);
    // if (mode != FILE_W && mode != FILE_RW){
    //     printf("FileWrite FATAL ERROR: Don't have permission to write!\n");
    //     return FILE_FAIL;
    // }
    if (mode == FILE_R){
        printf("FileWrite FATAL ERROR: Don't have permission to write!\n");
        return FILE_FAIL;
    }
    // Check if the number of bytes to write is in normal range
    if (num_bytes <= 0 || num_bytes > FILE_MAX_READWRITE_BYTES){
        printf("FileWrite FATAL ERROR: Number of bytes is not in normal range!\n");
        return FILE_FAIL;
    }
    // Wirte num_bytes to file
    // What if num_bytes + current_position > max_size_of_file ?
    inode_handle = file[handle].inode_handle;
    current_position = file[handle].current_position;
    if (inode_handle >= 0 && inode_handle < DFS_INODE_MAX_NUM){
        //if (current_position >= 0 && current_position < DfsInodeFilesize(inode_handle)){
        if (current_position >= 0) {
            num_bytes_written = DfsInodeWriteBytes(inode_handle, mem, current_position, num_bytes);
            if (num_bytes_written == DFS_FAIL){
                printf("FileWrite FATAL ERROR: Write operation Fail!\n");
                return FILE_FAIL;
            }
            return num_bytes_written;
        }
        printf("FileWrite FATAL ERROR: Current_Position is not in normal range!\n");
        return FILE_FAIL;
    }
    printf("FileWrite FATAL ERROR: Inode_handle is not in normal range!\n");
    return FILE_FAIL;
}


// -----------------------------------------------------------
// FileSeek: seek num_bytes within the file descriptor identified
// by handle, from the location specified by from_where. 
// There are three possible values for from_where: 
// FILE_SEEK_CUR (seek relative to the current position),
// FILE_SEEK_SET (seek relative to the beginning of the file),
// and FILE_SEEK_END (seek relative to the end of the file).
// Any seek operation will clear the eof flag.
// -----------------------------------------------------------
int FileSeek(int handle, int num_bytes, int from_where){
    // Check if the handle is accessible
    if (FileAccessible(handle) == FILE_FAIL){
        // printf("here handle %d\n", handle);
        // printf("Here, my file handle inode status is %d\n", file[handle].inode_handle);
        // printf("FileSeek FATAL ERROR: File not accessible!\n");
        return FILE_FAIL;
    }
    // Calculate the new current_position using 'from_where' and old current_position
    if (from_where == FILE_SEEK_CUR){
        file[handle].current_position += num_bytes; 
    }
    else if (from_where == FILE_SEEK_SET){
        file[handle].current_position = num_bytes;
    }
    else if (from_where == FILE_SEEK_END){
		// num_bytes can be negative according to piazza discussion
        file[handle].current_position = DfsInodeFilesize(file[handle].inode_handle) + num_bytes;
    }

    // Check if the current position is in normal range
	// If the current position > file size, the specific 
	// processing will be done in low layer function 
    if (file[handle].current_position < 0){
        return FILE_FAIL;
    }
    // Clear end of file flag
    file[handle].end_of_file = 0;
    return FILE_SUCCESS;
}


// -----------------------------------------------------------
// FileDelete: Delete the file specified by filename. Return 
// FILE_FAIL on failure, and FILE_SUCCESS on success.
// -----------------------------------------------------------
int FileDelete(char *filename){
    int inode_handle;
    int file_handle;

    // Get the inode handle according to file name
    inode_handle = DfsInodeFilenameExists(filename);
    // Two things to check: 
	// 1. If the inode exist; 
	// 2. If the file is accessible
    if (inode_handle != DFS_FAIL){
        file_handle = FileHandle(inode_handle);
        // printf("Here, my inode handle is %d\n", inode_handle);
        // printf("Here, my file handle is %d\n", file_handle);
        // printf("Here, my file handle inode status is %d\n", file[file_handle].inode_handle);
        if (file_handle == FILE_FAIL) {
            // printf("File Delete Fail, not exist!\n");
            return FILE_FAIL;
        }
        if (FileAccessible(file_handle) == FILE_FAIL){
            printf("FileDelete FATAL ERROR: File not accessible!\n");
            return FILE_FAIL;
        }
        // Got the inode_handle and file_handle
        // Delete the file inode
        if (DfsInodeDelete(inode_handle) != DFS_SUCCESS){
            printf("FileDelete FATAL ERROR: Cannot delete File!\n");
            return FILE_FAIL;
        }
        // Delete the file descriptor, use lock
        if (LockHandleAcquire(file_lock) != SYNC_SUCCESS);
        FileDescriptorInit(file_handle);
        if (LockHandleRelease(file_lock) != SYNC_SUCCESS);
        return FILE_SUCCESS;
    }
    printf("FileDelete: File is not exist!\n");
    return FILE_SUCCESS;
}

// -----------------------------------------------------------
// FileDescriptorInit: Delete file descriptor given file handle
// -----------------------------------------------------------
void FileDescriptorInit(int file_handle){
    // free 0, used 1
    file[file_handle].inuse = 0;
    bzero(file[file_handle].file_name, FILE_MAX_FILENAME_LENGTH);
    file[file_handle].inode_handle = -1;
    file[file_handle].current_position = 0; // init 0
    file[file_handle].end_of_file = 0; //0 not end, 1 end
    file[file_handle].mode = '\0';
    file[file_handle].user_pid = -1;
}


// -----------------------------------------------------------
// FileFindDescriptor: Find a new file descriptor, return
// file handle on success or FILE_FAIL on fail.
// -----------------------------------------------------------
int FileFindDescriptor(){
    int i;
    int file_handle;

    for (i = 0; i < DFS_INODE_MAX_NUM; i++){
        if (file[i].inuse == 0){
            file_handle = i;
            return file_handle;
        }
    }
    printf("FileFindDescriptor FATAL ERROR: No usable file descriptor!\n");
    return FILE_FAIL; 
}

// -----------------------------------------------------------
// FileMode: Find the file mode given char mode
// -----------------------------------------------------------
int FileMode(char *mode){
    if (mode[0] == 'r'){
        if (mode[1] == 'w'){
            return FILE_RW;
        }
        return FILE_R;
    }
    else if (mode[0] == 'w'){
        return FILE_W;
    }
    else {
        return FILE_FAIL;
    }
}

// -----------------------------------------------------------
// FileHandle: Find the file handle given inode handle
// -----------------------------------------------------------
int FileHandle(int inode_handle){
    int i;
    for (i = 0; i < DFS_INODE_MAX_NUM; i ++){
        if (file[i].inode_handle == inode_handle){
            return i;
        }
        // printf("FileHandle FATAL ERROR: Cannot find File Handle!\n");
        return FILE_FAIL;
    }
}

// -----------------------------------------------------------
// FileAccessible: Find if the file is accessible given file handle
// -----------------------------------------------------------
int FileAccessible(int file_handle){
    // Check if the file handle exist
    if (file_handle != FILE_FAIL){
        // file[file_handle].inuse: free 0, used 1
        if (file[file_handle].inuse != 1) {
            printf("FileAccessible: File in use\n");
            return FILE_FAIL;
        }
        // } else if (GetCurrentPid() != file[file_handle].user_pid) {
        //     printf("FileAccessible: pid not match, you are not creater\n");
        //     return FILE_FAIL;
        // }
        return FILE_SUCCESS;
    }
    printf("FileAccessible FATAL ERROR: File Handle not exist!\n");
    return FILE_FAIL;
}


