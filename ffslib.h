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
//int FS_Sync();

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
#define TYPE_FILESYSTEM '0x1717'
#define DIM_BLOCK 1024
#define MAX_BLOCK_FILE 32
#define MAX_FILENAME_LEN 16
#define MAX_PATHNAME_LEN 256
#define MAX_FILE_OPEN 10

// block structure
// list of sectors
typedef struct _listSector {
	Sector* sector;
	(struct _listSector)* next;
} Block;

// inode structure
typedef struct _inode {
	int type; // file or directory type
	int size; // size Byte
	Block[MAX_BLOCK_FILE] blocks; // blocks of data that contains the information of inode
} Inode;

// inode list structure
// list of inodes
typedef struct _inodeList {
	Inode* inode;
	(struct _inodeList)* next;
} InodeList;

// area structure
typedef struct _area {
	Block* superblock; // superblock
	Block* inodeBitmap; // inode bitmap
	Block* dataBitmap; // data bitmap
	InodeList* inodeList; // inode-list
	Block* dataBlocks;// data blocks
} Area;

// file structure
typedef struct _file {
	char[MAX_FILENAME_LEN] name;
	int iopointer;
	Inode* info; // inode of file
} File;

// directory structure
typedef struct _directory {
	char[MAX_FILENAME_LEN] name;
	Inode* info;
} Directory;

// open file table
typedef struct _table {
	File[MAX_FILE_OPEN] fileOpen;
} OpenFileTable;


#endif /* __FSLIB_h__ */

