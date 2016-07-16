#ifndef __FSLIB_h__
#define __FSLIB_h__

////////////////////////////////////////////////////////////////////////

/***************************************************************
 *     DO NOT MODIFY THIS FILE // NON MODIFICARE QUESTO FILE
 ****************************************************************/
    
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// used for errors
extern int osErrno;
    
// error types - don't change anything about these!! (even the order!)
typedef enum {
    E_GENERAL,      // general
    E_CREATE, 
    E_NO_SUCH_FILE, 
    E_TOO_MANY_OPEN_FILES, 
    E_BAD_FD, 
    E_NO_SPACE, 
    E_FILE_TOO_BIG, 
    E_SEEK_OUT_OF_BOUNDS, 
    E_FILE_IN_USE, 
    E_BUFFER_TOO_SMALL, 
    E_DIR_NOT_EMPTY,
    E_ROOT_DIR,
} FS_Error_t;
    
// File system generic call
int FS_Create(char *path);
int FS_Sync();

// file ops
int File_Create(char *file);
int File_Open(char *file);
int File_Read(int fd, void *buffer, int size);
int File_Write(int fd, void *buffer, int size);
int File_Seek(int fd, int offset);
int File_Close(int fd);
int File_Unlink(char *file);

// directory ops
int Dir_Create(char *path);
int Dir_Size(char *path);
int Dir_Read(char *path, void *buffer, int size);
int Dir_Unlink(char *path);

////////////////////////////////////////////////////////////////////////
// Strutture Dati per rappresentare Files e Directories
    
////////////////////////////////////////////////////////////////////////

// GLOBAL VARIABLES
#define TYPE_FILESYSTEM "0x1717"
#define BLOCK_SIZE 1024
#define MAX_BLOCK_FILE 32
#define MAX_FILENAME_LEN 16
#define MAX_PATHNAME_LEN 256
#define MAX_FILE_OPEN 32
#define MAX_INODE 1024
#define MAX_FILE_SIZE_LEN 5 // lenght of char for file or directory size
#define INDEX_SIZE 4 // number of address' digit

// block structure
// list of sectors or block
typedef struct _listSector {
	Sector* sector;
	struct _listSector* next;
} Block;

// inode structure
typedef struct _inode {
	char type; // file or directory type
	char name[MAX_FILENAME_LEN]; // name of directory or file
	char size[MAX_FILE_SIZE_LEN]; // size Byte
	char blocks[MAX_BLOCK_FILE*INDEX_SIZE]; // blocks of data that contains the information of inode
} Inode;

// group structure
typedef struct _group {
	Block* superblock; // superblock
	Block* inodeBitmap; // inode bitmap
	Block* dataBitmap; // data bitmap
	Block* inodeTable[DIM_INODE_TABLE]; // inode-list
	Block* dataBlocks[DIM_GROUP-DIM_INODE_TABLE-4];// data blocks
} Group;

// file structure
typedef struct _file {
	int fileDescriptor;
	int iopointer; // iopointer of the file is set to 0 by default
	Inode* info; // inode of file
} File;

// open file table
typedef struct _table {
	File* fileOpen[MAX_FILE_OPEN];
} OpenFileTable;

OpenFileTable* openFiles; // table of file open

#endif /* __FSLIB_h__ */

