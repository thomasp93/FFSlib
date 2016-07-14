#include "dummydisk.c"
#include "ffslib.h"

// global errno value here
int osErrno;
////////////////////////////////////////////////////////////////////////
// funzione che crea il tutto, va chiamata una volta sola all'inizio
int FS_Init(char *path) {
	Block* b = (Block*) calloc(1, sizeof(Block)); // create block
	Group* group;
	GroupDescriptor* groupDescriptor;
	ListSector* tmp;
	char* s;
	char* c;
	char buff[INDEX_SIZE];
    int create, nGroups=(SECTOR_SIZE*NUM_SECTORS)/(DIM_BLOCK*DIM_GROUP), i, nC=0, g=0, nS=1;
    
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
    b->nCharWrite = 0;
    b->sectors = NULL;

    for(i=0; i<DIM_BLOCK/SECTOR_SIZE; i++) // creation list sector
    {
    	tmp = (ListSector*) calloc(1, sizeof(ListSector));
		tmp->sector = (Sector*) calloc(1, sizeof(Sector));
		tmp->next = b->sectors;
    	b->sectors = tmp;
    }

    if(create==0) // set disk
    {
		// set the boot block
		BootBlock* bb = (BootBlock*) calloc(1, sizeof(BootBlock));
		bb->typeFS = TYPE_FILESYSTEM;
		bb->startGroupsBlock = (int*) calloc(nGroups, sizeof(int));
		
		for (i=0; i<nGroups; i++)
			bb->startGroupsBlock[i] = (1+i*DIM_GROUP)*(DIM_GROUP*DIM_BLOCK); // calculate the idex of group

		s = (char*) calloc(1, SECTOR_SIZE*sizeof(char));
		strcat(s, bb->typeFS);
		nC += strlen(bb->typeFS);

		tmp = b->sectors;
		while(tmp!=NULL) 
		{
			while((nC+INDEX_SIZE)<(nS*SECTOR_SIZE) && g<nGroups) // while the new index is less than free char into sector
			{
				snprintf(buff, INDEX_SIZE, "%04d", bb->startGroupsBlock[g]);
				strcat(s, buff);
				g++;
				nC+=INDEX_SIZE;
			}
			i=0;
			while(nC==(nS*SECTOR_SIZE) && g<nGroups) // insert char to char into sector while it's full
			{
				snprintf(buff, INDEX_SIZE, "%04d", bb->startGroupsBlock[g]);
				strcat(s, &buff[i]);
				i++;
				nC++;
			}

			while(nC<(nS*SECTOR_SIZE) && !(g<nGroups))
			{
				strcat(s, "0");
				nC++;
			}
			
			strcpy(tmp->sector->data, s); // copy the sector create
			Disk_Write(nS-1, b->sectors->sector->data); // write the sector of the block into the disk
			nS++;
			tmp=tmp->next; // pass to the next sector
			s=(char*) malloc(SECTOR_SIZE*sizeof(char)); // create a char array lenght as the sector
		}

		// set all the super blocks and rispective group blocks
		// set a default block
		b->nCharWrite = 0;
		b->sectors = NULL;

		for(i=0; i<DIM_BLOCK/SECTOR_SIZE; i++) // creation list sector
		{
			tmp = (ListSector*) calloc(1, sizeof(ListSector));
			tmp->sector = (Sector*) calloc(1, sizeof(Sector));
			tmp->next = b->sectors;
			b->sectors = tmp;
		}
		
		for(i=0; i<nGroups; i++)
		{
			s = (char*) calloc(1, SECTOR_SIZE*sizeof(char));
			c = (char*) calloc(1, SECTOR_SIZE*sizeof(char));
			strcpy(s, TYPE_FILESYSTEM);
			Disk_Write(bb->startGroupsBlock[i], s); // write the super block

			snprintf(buff, INDEX_SIZE, "%d", i); // index of group
			strcpy(c, buff);
			
			snprintf(buff, INDEX_SIZE, "0%d", bb->startGroupsBlock[i]);
			strcat(c, buff); // begin index of group
			
			if(bb->startGroupsBlock[i]+DIM_BLOCK-1 > NUM_SECTORS/DIM_BLOCK)
				snprintf(buff, INDEX_SIZE, "0%d", NUM_SECTORS/DIM_BLOCK);
			else
				snprintf(buff, INDEX_SIZE, "0%d", bb->startGroupsBlock[i]+DIM_BLOCK-1);
			strcat(c, buff); // end index of group

			snprintf(buff, INDEX_SIZE, "0%d", bb->startGroupsBlock[i]+3);
			strcat(c, buff); // index of inode bitmap

			snprintf(buff, INDEX_SIZE, "0%d", bb->startGroupsBlock[i]+2);
			strcat(c, buff); // index of data block bitmap

			snprintf(buff, INDEX_SIZE, "0%d", i*20);
			strcat(c, buff); // begin index of range inode table

			snprintf(buff, INDEX_SIZE, "0%d", (i+1)*20-1);
			strcat(c, buff); // end index of range inode table

			snprintf(buff, INDEX_SIZE, "0%d", 20);
			strcat(c, buff); // inode free

			snprintf(buff, INDEX_SIZE, "0%d", DIM_GROUP-DIM_INODE_TABLE-4);
			strcat(c, buff); // data block free

			strcpy(s, c); // copy the string c into the string lenght as the sector
			
			Disk_Write(bb->startGroupsBlock[i]+(DIM_BLOCK/SECTOR_SIZE), s); // write group descriptor
		}

		Dir_Create("/"); // create the root directory
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

		for(i=0; i<6; i++) // control type fs
			if(TYPE_FILESYSTEM[i]!=b->sectors->sector->data[i]) // fs not exactly
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
    if( newiopointer > (openFiles->fileOpen[fd]->info->size) || newiopointer < 0 ){ //verify that the iopointer does not cross the file bounds
        osErrno = E_SEEK_OUT_OF_BOUNDS;
        return -1;
    } else {
        openFiles->fileOpen[fd]->iopointer = newiopointer;
        printf("FS_Seek\n");
        return 0;
    }    
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
