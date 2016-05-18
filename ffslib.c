#include "ffslib.h"
#include "dummydisk.c"

// global errno value here
int osErrno;

////////////////////////////////////////////////////////////////////////

int FS_Init(char *path) {
    
    printf("FS_Init %s\n", path);

    // create disk
    if (Disk_Create() == -1) {
	    printf("Disk_Create() failed\n");
	    osErrno = E_GENERAL;
	    return -1;
    }

    // create data-structure to handle files and directories

    return 0;
}


////////////////////////////////////////////////////////////////////////
// File ops
////////////////////////////////////////////////////////////////////////

int File_Create(char *file) {
    printf("FS_Create\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////

int File_Open(char *file) {
    printf("FS_Open\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////

int File_Read(int fd, void *buffer, int size) {
    printf("FS_Read\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////

int File_Write(int fd, void *buffer, int size) {
    printf("FS_Write\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////

int File_Seek(int fd, int offset) {
    printf("FS_Seek\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////

int File_Close(int fd) {
    printf("FS_Close\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////

int File_Unlink(char *file) {
    printf("FS_Unlink\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////
// directory ops
////////////////////////////////////////////////////////////////////////

int Dir_Create(char *path) {
    printf("Dir_Create %s\n", path);
    return 0;
}

int Dir_Size(char *path) {
    printf("Dir_Size\n");
    return 0;
}

int Dir_Read(char *path, void *buffer, int size) {
    printf("Dir_Read\n");
    return 0;
}

int Dir_Unlink(char *path) {
    printf("Dir_Unlink\n");
    return 0;
}
