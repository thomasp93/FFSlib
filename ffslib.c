#include "dummydisk.c"
#include "ffslib.h"

// global errno value here
int osErrno;
static int nGroups=(SECTOR_SIZE*NUM_SECTORS)/(DIM_BLOCK*DIM_GROUP);
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
    int create;
    
    int i, nC=0, g=0, nS=1;
    
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
			bb->startGroupsBlock[i] = (1+i*DIM_GROUP)*(DIM_BLOCK/SECTOR_SIZE); // calculate the idex of group

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
			while(nC<(nS*SECTOR_SIZE) && g<nGroups) // insert char to char into sector while it's full
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
			s=(char*) calloc(1,SECTOR_SIZE*sizeof(char)); // create a char array lenght as the sector
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

			snprintf(buff, INDEX_SIZE, "%04d", i); // index of group
			strcpy(c, buff);
			
			snprintf(buff, INDEX_SIZE, "%04d", bb->startGroupsBlock[i]);
			strcat(c, buff); // begin index of group
			
			if(bb->startGroupsBlock[i]+DIM_BLOCK-1 > NUM_SECTORS/DIM_BLOCK)
				snprintf(buff, INDEX_SIZE, "%04d", NUM_SECTORS/DIM_BLOCK);
			else
				snprintf(buff, INDEX_SIZE, "%04d", bb->startGroupsBlock[i]+DIM_BLOCK-1);
			strcat(c, buff); // end index of group

			snprintf(buff, INDEX_SIZE, "%04d", bb->startGroupsBlock[i]+3);
			strcat(c, buff); // index of inode bitmap

			snprintf(buff, INDEX_SIZE, "%04d", bb->startGroupsBlock[i]+2);
			strcat(c, buff); // index of data block bitmap

			snprintf(buff, INDEX_SIZE, "%04d", i*20);
			strcat(c, buff); // begin index of range inode table

			snprintf(buff, INDEX_SIZE, "%04d", (i+1)*20-1);
			strcat(c, buff); // end index of range inode table

			snprintf(buff, INDEX_SIZE, "%04d", 20);
			strcat(c, buff); // inode free

			snprintf(buff, INDEX_SIZE, "%04d", DIM_GROUP-DIM_INODE_TABLE-4);
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
	Sector* sector = (Sector*) calloc(1, sizeof(Sector));
	char* buff, subString;
	int i, c, position;
	
    printf("Dir_Create %s\n", path);

	// creation a directory
	Inode* dir = (Inode*) calloc(1, sizeof(Inode));
	dir->type = "d";
	strcpy(dir->name, name);
	
	snprintf(buff, 6, "%06d", 0);
	strcpy(dir->size, buff);

	dir->blocks = (Block*) calloc(MAX_BLOCK_FILE, sizeof(Block));
	
	// save the inode into the inode table
	BootBlock* bb = (BootBlock*) calloc(1, sizeof(BootBlock)); // create boot block
	bb->typeFS = TYPE_FILESYSTEM; // set the type of fs
	bb->startGroupsBlock = (int*) malloc(nGroups*sizeof(int)); // set the begin index of groups
	position = strlen(bb->typeFS); // set the position
	for(i=0; i<nGroups; i++) // load the index of groups
	{
		Disk_Read(0, sector->data); // read sector of boot block
		subString = (char*) malloc(INDEX_SIZE*sizeof(char)); // create subString
		while(c<INDEX_SIZE)
		{
			sub[c] = sector->data[position+c-1]; // read a char into the sector in position 
			c++; // increment c
		}
		sub[c] = "\0";
		bb->startGroupsBlock[i] = atoi(sub); // convert the string in a int
	}

	GroupDescriptor* gd = (GroupDescriptor*) malloc(sizeof(GroupDescriptor)); // load the group descriptor
	i=0; // reset index i
	do
	{
		Disk_Read(bb->startGroupsBlock[i]+1*(DIM_BLOCK/SECTOR_SIZE), sector->data); // read the group descriptor of the visit group
		position=28; // position of begin index of nInodeFree
		subString = (char*) malloc(INDEX_SIZE*sizeof(char)); // create subString
		while(c<INDEX_SIZE)
		{
			sub[c] = sector->data[position+c-1]; // read a char into the sector in position 
			c++; // increment c
		}
		sub[c] = "\0";
		gd->nInodeFree = atoi(sub); // convert the string in a int
		i++;
	} while(gd->nInodeFree>0 && i<nGroups); // find the group with inode free

	if(!(i<nGroups)) // if not exist inode free
	{
		osErrno = E_CREATE;
		return -1;
	}

	// else exist inode free
	i--; // deincrement index
	Disk_Read(bb->startGroupsBlock[i]+3*(DIM_BLOCK/SECTOR_SIZE), sector->data); // read bitmap inode
	while(c!=NULL && indexInode<20) // while c is not equals to 0 and index inode is less than number inode into the inode table
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
	indexInode = (int)indexInode-SECTOR_SIZE*indexSector; // recalculate the index of inode

	
	
	
    
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
