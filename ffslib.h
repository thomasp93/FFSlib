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
#define TYPE_FILESYSTEM "0x1717"
#define DIM_BLOCK 1024
#define MAX_BLOCK_FILE 32
#define MAX_FILENAME_LEN 16
#define MAX_PATHNAME_LEN 256
#define MAX_FILE_OPEN 32
#define DIM_GROUP 512
#define DIM_INODE_TABLE 3

// block structure
// list of sectors
typedef struct _listSector {
	Sector* sector;
	struct _listSector* next;
} ListSector;

// block structur
typedef struct _block {
	ListSector* sectors;
	int nCharWrite;
} Block;

// inode structure
typedef struct _inode {
	int type; // file or directory type
	char name[MAX_FILENAME_LEN]; // name of directory or file
	int size; // size Byte
	Block blocks[MAX_BLOCK_FILE]; // blocks of data that contains the information of inode
} Inode;

// inode list structure
// list of inodes
typedef struct _inodeList {
	Inode* inode;
	struct _inodeList* next;
} InodeList;

typedef struct _bootBlock {
	char* typeFS;
	int* startGroupsBlock;
} BootBlock;

// group descriptor structure
typedef struct _groupDescriptor {
	int index;
	int rangeBlockStart;
	int rangeBlockEnd;
	int bitmapInodeIndex; // begin of the inode bitmap
	int bitmapDataBlockIndex; // begin of the data block bitmap
	int InodeTableIndex; // begin of the inode table
	int nInodeFree; // number of the inode free
	int nBlockFree; // number of the data block free
} GroupDescriptor;

// group structure
typedef struct _group {
	Block superblock; // superblock
	Block groupDescriptor; // group descriptor
	Block inodeBitmap; // inode bitmap
	Block dataBitmap; // data bitmap
	Block inodeTable[DIM_INODE_TABLE]; // inode-list
	Block dataBlocks[DIM_GROUP-DIM_INODE_TABLE-4];// data blocks
} Group;

// file structure
typedef struct _file {
	int iopointer; // iopointer of the file is set to 0 by default
	Inode* info; // inode of file
} File;

// directory structure
typedef struct _directory {
	Inode* info; // inode of directory
} Directory;

// open file table
typedef struct _table {
	File fileOpen[MAX_FILE_OPEN];
} OpenFileTable;


#endif /* __FSLIB_h__ */

