//
// Disk.h
//
// Emulates a very simple disk

/***************************************************************
 *     DO NOT MODIFY THIS FILE // NON MODIFICARE QUESTO FILE
 ****************************************************************/

#ifndef __Disk_H__
#define __Disk_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// disk parameters
#define SECTOR_SIZE  512     // sector size in bytes
#define NUM_SECTORS  4096    // numer of sectors

// disk errors
typedef enum {
  E_MEM_OP,
  E_INVALID_PARAM,
  E_OPENING_FILE,
  E_WRITING_FILE,
  E_READING_FILE,
} Disk_Error_t;

typedef struct sector {
  char data[SECTOR_SIZE];
} Sector;

extern Disk_Error_t diskErrno; // used to see what happened w/ disk ops

int Disk_Init();
int Disk_Save(char* file);
int Disk_Load(char* file);
int Disk_Write(int sector, char* buffer);
int Disk_Read(int sector, char* buffer);

#endif // __Disk_H__
