#include "dummydisk.c"
#include "ffslib.h"

// global errno value here
int osErrno;

////////////////////////////////////////////////////////////////////////
// funzione che crea il tutto, va chiamata una volta sola all'inizio
int FS_Init(char *path) {
    int create, nGroups, i;
    
    printf("FS_Init %s\n", path);

    // open disk
    if (Disk_Load(path) == -1) { // load the disk
    	if (diskErrno == E_OPENING_FILE) { // if the path not exist
		if (Disk_Create() == -1) { // create the disk
			printf("Disk_Create() failed\n"); // with error
			osErrno = E_GENERAL;
			return -1;
		}
		else // disk create
			create = 0; // set variable
	}
	else { // other error of the load disk
		printf("Disk_Load(*path) failed\n");
		osErrno = E_GENERAL;
		return -1;
	}
    }
    else create = 1; // disk load

    // create data-structure to handle files and directories
    Block* b = (Block*) calloc(1, sizeof(Block)); // create block
    b->nCharWrite = 0;
    b->sectors = NULL;
    ListSector* tmp;

    for(i=0; i<DIM_BLOCK/SECTOR_SIZE; i++) // creation list sector
    {
    	tmp = (ListSector*) calloc(1, sizeof(ListSector));
	tmp->sector = (Sector*) calloc(1, sizeof(Sector));
	tmp->next = b->sectors;
    	b->sectors = tmp;
    }


    if(create==0) // set disk
    {
	    	
    }
    else // read exist disk
    {
    	i=0; // first block for get super block

	tmp = b->sectors;
    	while (tmp!=NULL)
	{ 
    		Disk_Read(i, tmp->sector->data); // read sector
		tmp = tmp->next;
		i++;
	}

	for(i=0; i<6; i++)
		if(TYPE_FILESYSTEM[i]!=b->sectors->sector->data[i])
		{
			printf("File system not exactly!!");
			osErrno = E_GENERAL;
			return -1;
		}

	printf("Funziona");
    }

    printf("Non funziona");

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
