#include "dummydisk.c"
#include "ffslib.h"

// global errno value here
int osErrno;

////////////////////////////////////////////////////////////////////////
// funzione che crea il tutto, va chiamata una volta sola all'inizio
int FS_Init(char *path) {
	Block* block = (Block*) calloc(1, sizeof(Block)); // create block
	Block* tmp; // temporary block
	Sector* sector;
	char* s; // temporary string
    int i, create, noChar=0, noSectorVisited=1;
    
    printf("FS_Init %s\n", path);

    // open disk
    if (Disk_Load(path) == -1) { // load the disk with error
    	if (diskErrno == E_OPENING_FILE) // if the disk not exist
			create = 0; // set variable for set a default disk
		else { // other error of the load disk
			printf("Disk_Load(*path) failed\n");
			osErrno = E_GENERAL;
			return -1;
		}
    }
    else create = 1; // disk load correctly

    // create data-structure to handle files and directories
    
    // set a default block
    block->sector = NULL;

    for(i=0; i<DIM_BLOCK/SECTOR_SIZE; i++) // creation list sector
    {
    	tmp = (Block*) calloc(1, sizeof(Block));
		tmp->sector = (Sector*) calloc(1, sizeof(Sector));
		tmp->next = block->sector;
    	block->sector = tmp;
    }

    if(create==0) // set disk
    {
		s = (char*) calloc(1, SECTOR_SIZE*sizeof(char));
		strcat(s, TYPE_FILESYSTEM);
		noChar += strlen(s);

		tmp = block->sector;
		while(tmp!=NULL) // visit all sector into the block
		{
			while(noChar<(noSectorVisited*SECTOR_SIZE)) // write 0 while the sector is full
			{
				strcat(s, "0");
				noChar++;
			}
			
			strcpy(tmp->sector->data, s); // copy the sector create
			Disk_Write(noSectorVisited-1, tmp->sector->data); // write the sector of the block into the disk
			noSectorVisited++; // increment of one the visit sector
			tmp = tmp->next; // pass to the next sector
			s = (char*) calloc(1, SECTOR_SIZE*sizeof(char)); // create a char array lenght as the sector
		}

		// set a sector with all zero
		sector = (Sector*) calloc(1, sizeof(Sector));
		for(i=0; i<SECTOR_SIZE; i++)
			sector->data[i] = "0"; 
		
		for(i=1; i<NUM_SECTORS; i++) // set all sector with zeros
			Disk_Write(i, sector->data);

		Dir_Create("/"); // create the root directory
    }
    else // read exist disk
    {
    	i=0; // first block for get super block

		tmp = block->sector;
    	while (tmp!=NULL)
		{ 
    		Disk_Read(i, tmp->sector->data); // read sector
			tmp = tmp->next;
			i++;
		}

		for(i=0; i<strlen(TYPE_FILESYSTEM); i++) // control type fs
			if(TYPE_FILESYSTEM[i]!=block->sector->data[i]) // fs not exactly
			{
				printf("File system not exactly!!");
				osErrno = E_GENERAL;
				return -1;
			}
	}

	printf("Disk load correctly");
	openFiles = (OpenFileTable*) malloc(sizeof(OpenFileTable));

	return 0;
}

int FS_Sync(char* diskname){
    int i=0;
    while(i < MAX_FILE_OPEN){
        File_Close(i);
        i++;
    }
    
    if(Disk_Save(diskname) == -1){
        return -1;
    } else {
        return 0;
    } 
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
    int newiopointer = openFiles->fileOpen[fd]->iopointer + offset; // calculate the new io pointer
    
    if (newiopointer > atoi(openFiles->fileOpen[fd]->info->size) || newiopointer < 0) //verify that the iopointer does not cross the file bounds
    {
        osErrno = E_SEEK_OUT_OF_BOUNDS;
        return -1;
    }
    
    openFiles->fileOpen[fd]->iopointer = newiopointer;
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
	Sector* sector = (Sector*) calloc(1, sizeof(Sector));
	char* buff, subString;
	int i, c, position, indexSector, indexInode;
	
    printf("Dir_Create %s\n", path);

	// creation a directory
	Inode* dir = (Inode*) calloc(1, sizeof(Inode));
	dir->type = "d"; // set the type as directory
	strcpy(dir->name, ""); // TODO calculate the name of directory trought the path
	strcpy(dir->size, "00000"); // set the size of the directory
	
	// save the inode into the inode table
	Disk_Read(DIM_BLOCK/SECTOR_SIZE, sector->data); // read bitmap inode
	while(c!="0" && indexInode<MAX_INODE) // while c is not equals to 0 and index inode is less than number inode into the inode table
	{
		c = sector->data[indexInode]; // read index inode char
		indexInode++; // increment index
	}

	if(indexInode>=20) // if not exist inode free
	{
		osErrno = E_CREATE;
		return -1;
	}

	// else I found the index of free inode
	indexInode--; // set the true index
	sector->data[indexInode] = "1"; // set the inode as busy
	Disk_Write(bb->startGroupsBlock[i]+3*(DIM_BLOCK/SECTOR_SIZE), sector->data); // write the edit sector into the disk

	// write the inode into the inode table
	indexSector = (int)indexInode*150/SECTOR_SIZE; // index of sector into the inode table
	indexInode = (int)indexInode*150-SECTOR_SIZE*indexSector+20*i; // recalculate the index of inode into the inode table

	Disk_Read(bb->startGroupsBlock[i]+4+indexSector, sector->data); // read a sector that it contains the free inode
	
	nC=0; // reset the number of char write into the inode block
	if(nC==0) // type of inode
		sector->data[indexInode+nC] = dir->type; // write the dir type
	else if(nC > 0 && nC < MAX_FILENAME_LEN) // name of directory
	{
		strcpy(sector->data+indexInode+nC, dir->name);
		nC+=MAX_FILENAME_LEN;
	}
	else if(nC > MAX_FILENAME_LEN && nC < MAX_FILENAME_LEN+5) // dimension of directory
	{
		strcpy(sector->data+indexInode+nC, "00000");
		nC+=5;
	}
	else
		while(nC<150) // while the inode sector is full
		{
			sector->data[indexInode+nC] = "0"; // set 0 
			nC++;
		}
	
	Disk_Write(bb->startGroupsBlock[i]+4+indexSector, sector->data); // write the new sector with the new inode
    
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
    char* buffer[1024];
    char* tmp;
    Disk_Read((DIM_BLOCK/SECTOR_SIZE*1*2), tmp);
    strcat(buffer, tmp);
    Disk_Read((DIM_BLOCK/SECTOR_SIZE*1*2)+1, tmp);
    strcat(buffer, tmp);
    tmp = (char *) calloc(5, sizeof(char));
    i=30;
    while(i<35){
    	tmp[i-30]= buffer[i];
    	i++;
    }
    int indexinodeT = atoi(tmp);
    char* inodeinfo = (char*) calloc(150, sizeof(char));
    bzero(buffer, 512);
    Disk_Read(indexinodeT*(DIM_BLOCK/SECTOR_SIZE), buffer);
    i=0;
    while (i<150){
    	inodeinfo[i] = buffer[i];
    }
    
    printf("Dir_Unlink\n");
    return 0;
}
