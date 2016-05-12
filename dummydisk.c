//
// Disk.c
//
// Emulates a very simple disk

#include <string.h>

#include "dummydisk.h"

/***************************************************************
 *     DO NOT MODIFY THIS FILE // NON MODIFICARE QUESTO FILE
 ****************************************************************/

////////////////////////////////////////////////////////////////////////

// the disk in memory (static makes it private to the file)
static Sector* disk;

// used to see what happened w/ disk ops
Disk_Error_t diskErrno; 

// used for statistics
// static int lastSector = 0;
// static int seekCount = 0;

////////////////////////////////////////////////////////////////////////
// Disk_Create: Initializes the disk area 

int Disk_Create() {
  // create the disk image and fill every sector with zeroes
  disk = (Sector *) calloc(NUM_SECTORS, sizeof(Sector));
  if(disk == NULL) {
    diskErrno = E_MEM_OP;
    return -1;
  }
  
  return 0;
}

////////////////////////////////////////////////////////////////////////
// Disk_Load: Loads a current disk image from disk into memory

int Disk_Load(char* diskname) {
  FILE* diskFile;
  
  // error check
  if (diskname == NULL) {
    diskErrno = E_INVALID_PARAM;
    return -1;
  }
  
  // open the diskFile
  if ((diskFile = fopen(diskname, "r")) == NULL) {
    diskErrno = E_OPENING_FILE;
    return -1;
  }
  
  // actually read the disk image into memory
  if ((fread(disk, sizeof(Sector), NUM_SECTORS, diskFile)) != NUM_SECTORS) {
    fclose(diskFile);
    diskErrno = E_READING_FILE;
    return -1;
  }
  
  // close
  fclose(diskFile);
  
  return 0;
}

////////////////////////////////////////////////////////////////////////
// Disk_Save: save memory image to file

int Disk_Save(char* diskname) {
  FILE* diskFile;
    
  // error check
  if (diskname == NULL) {
    diskErrno = E_INVALID_PARAM;
    return -1;
  }
    
  // open the diskFile
  if ((diskFile = fopen(diskname, "w")) == NULL) {
    diskErrno = E_OPENING_FILE;
    return -1;
  }
    
  // write the disk image to a file
  if ((fwrite(disk, sizeof(Sector), NUM_SECTORS, diskFile)) != NUM_SECTORS) {
    fclose(diskFile);
    diskErrno = E_WRITING_FILE;
    return -1;
  }
    
  // clean up and return
  fclose(diskFile);
  
  return 0;
}

////////////////////////////////////////////////////////////////////////
// Disk_Read: Reads a single sector from disk and puts it into a buffer 
// provided by the user.

int Disk_Read(int sector, char* buffer) {
  // quick error checks
  if ((sector < 0) || (sector >= NUM_SECTORS) || (buffer == NULL)) {
    diskErrno = E_INVALID_PARAM;
    return -1;
  }
    
  // copy the memory for the user
  if((memcpy((void*)buffer, (void*)(disk + sector), sizeof(Sector))) == NULL) {
    diskErrno = E_MEM_OP;
    return -1;
  }
    
  return 0;
}

////////////////////////////////////////////////////////////////////////
// Disk_Write: Writes a single sector from a buffer provided by the user 
// to disk
 
int Disk_Write(int sector, char* buffer) {
  // quick error checks
  if((sector < 0) || (sector >= NUM_SECTORS) || (buffer == NULL)) {
    diskErrno = E_INVALID_PARAM;
    return -1;
  }

  // copy the memory for the user
  if((memcpy((void*)(disk + sector), (void*)buffer, sizeof(Sector))) == NULL) {
    diskErrno = E_MEM_OP;
    return -1;
  }
  
  return 0;
}
