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
	char* buff, subString, block;
	char* granfather, dad, son, next, granPath, sons;
	int i, c, position, indexBlock, indexSector, indexInode, noChar;
	int indexInodeSon, indexInodeDad, posCharStart;

	if (strlen(path)>MAX_PATHNAME_LEN)
		return -1;
	
    	printf("Dir_Create %s\n", path);
	
	// save the inode into the inode table
	for(i=0; i<BLOCK_SIZE/SECTOR_SIZE; i++) // visit the inode bitmap
	{
		indexInode=0;
		Disk_Read(DIM_BLOCK/SECTOR_SIZE+i, sector->data); // read bitmap inode
		while(c!="0" && indexInode<SECTOR_SIZE) // while c is not equals to 0 and index inode is less than number inode into the sector
		{
			c = sector->data[indexInodeSon]; // read index inode char
			indexInodeSon++; // increment index
		}

		if(c=="0") // find the free inode
			break; // exit from for
	}

	if(c!="0") // if not exist inode free
	{
		osErrno = E_CREATE;
		return -1;
	}
	else
		indexInodeSon -= 1; // correct the more increment of index

	// else I found the index of free inode
	indexInodeSon += i*SECTOR_SIZE; // calculate the true value of free index inode
	sector->data[indexInodeSon] = "1"; // set the inode as busy
	Disk_Write(BLOCK_SIZE/SECTOR_SIZE+i, sector->data); // write the edit sector into the disk

	// write the inode into the inode table
	indexBlock = (int)indexInodeSon*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
	indexSector = (int)indexInodeSon*sizeof(Inode)/SECTOR_SIZE+indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
	indexInode = (int)(indexInodeSon*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

	// take the granfather's path
	granfather = strtok(path, "/");

	if (granfather != NULL) // root case /
	{
		dad = strtok(NULL, "/");

		if (dad != NULL) // /home/thomas gran=home dad=thomas
		{
			strcat(granPath, "/"); // /

			son = strtok(NULL, "/");

			if (son != NULL) // /home/thomas/Scrivania gran=home dad=thomas son=Scrivania
			{
				strcat(granPath, gran); // /home

				next = strtok(NULL, "/"); // /home/thomas/Scrivania/pippo gran=home dad=thomas son=Scrivania next=pippo

				while (next != NULL)
				{
					gran = dad;
					dad = son;
					son = next;
					strcat(granPath, "/");
					strcat(granPath, gran);
					next = strtok(NULL, "/"); // read the next token
				}
			}

			sons = (void*) calloc(1, size);
			if (Dir_Read(granPath, sons, MAX_BLOCK_FILE*(INDEX_SIZE+MAX_FILENAME_LEN)) != 0)
				return -1;

			indexInodeDad = atoi(strstr(sons, dad)+MAX_FILENAME_LEN); // find the dad's inode
		}
		else // /home
			indexInodeDad = 0; // dad inode found

		indexBlock = (int)indexInodeDad*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
		indexSector = (int)indexInodeDad*sizeof(Inode)/SECTOR_SIZE+indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
		indexInode = (int)(indexInodeDad*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

		Disk_Read(indexSector, sector->data);
		strcpy(block, sector->data);
	
		if (indexInode+sizeof(Inode)>SECTOR_SIZE)
		{
			Disk_Read(indexSector+1, sector->data);
			strcat(block, sector->data);
		}	

		// read the dad's inode
		posCharStart=indexInode;
		snprintf(dadInode->type, 1, block+posCharStart); // read the type of inode
		posCharStart+=1; // add the char readden
		snprintf(dadInode->name, MAX_FILENAME_LEN, block+posCharStart); // read the name of file (directory)
		posCharStart+=MAX_FILENAME_LEN;
		snprintf(dadInode->size, MAX_FILE_SIZE_LEN, block+posCharStart); // read the size of file (directory)
		posCharStart+=MAX_FILE_SIZE_LEN;
		snprintf(dadInode->blocks, MAX_BLOCK_FILE*INDEX_SIZE, block+posCharStart); // read all block of inode

		snprintf(buff, INDEX_SIZE, "%04d", indexInodeSon);
		strcat(dadInode->blocks+atoi(dadInode->size), buff); // copy the index son into the dad's block

		snprintf(dadInode->size, MAX_FILE_SIZE_LEN, "%05d", atoi(dadInode->size)+INDEX_SIZE); // edit the size of dad's inode

		char* inodeInfo = (char*) calloc(1, sizeof(Inode)); // create the inode

		strcat(inodeInfo, dadInode->type);
		strcat(inodeInfo, dadInode->name);
		strcat(inodeInfo, dadInode->size);
		strcat(inodeInfo, dadInode->blocks);

		if(indexInode+sizeof(Inode)<SECTOR_SIZE) // if the inode is less than size free into the inode
			strcpy(sector->data+indexInode, inodeInfo); // write the inode into the sector
		else
		{
			for(i=0, noChar=indexInode; i<sizeof(Inode); i++, noChar++)
			{
				if(i+indexInode<SECTOR_SIZE) // if the sector is full
				{
					Disk_Write(indexSector, sector->data); // write the first sector
					indexSector++; // increment the sector
					Disk_Read(indexSector, sector->data); // read the new sector
					noChar = 0; // reset the number char written
				}

				sector->data[noChar] = inodeInfo[i]; // write the char into the sector
			}
		}

		Disk_Write(indexSector, sector->data); // write the sector into the disk
	} // end root case

	// creation a directory
	Inode* dir = (Inode*) calloc(1, sizeof(Inode));
	dir->type = "d"; // set the type as directory
	strcpy(dir->name, son); // copy the name of directory
	strcpy(dir->size, "00000"); // set the size of the directory

	Disk_Read(indexSector, sector->data); // read a sector that it contains the free inode
	
	noChar=0; // reset the number of char written into the inode block
	char* inodeInfo = (char*) calloc(1, sizeof(Inode)); // create the inode

	inodeInfo[indexInode+noChar] = dir->type; // write the dir type
	noChar++;

	strcpy(inodeInfo+noChar, dir->name);
	noChar+=MAX_FILENAME_LEN;

	strcpy(inodeInfo+noChar, "00000");
	noChar+=5;

	if(indexInode+sizeof(Inode)<SECTOR_SIZE) // if the inode is less than size free into the inode
		strcpy(sector->data+indexInode, inodeInfo); // write the inode into the sector
	else
	{
		for(i=0, noChar=indexInode; i<sizeof(Inode); i++, noChar++)
		{
			if(i+indexInode<SECTOR_SIZE) // if the sector is full
			{
				Disk_Write(indexSector, sector->data); // write the first sector
				indexSector++; // increment the sector
				Disk_Read(indexSector, sector->data); // read the new sector
				noChar = 0; // reset the number char written
			}

			sector->data[noChar] = inodeInfo[i]; // write the char into the sector
		}
	}

	Disk_Write(indexSector, sector->data); // write the sector into the disk
    
    	return 0;
}

int Dir_Size(char *path) {
	printf("Dir_Size\n");
	Inode* inode = (Inode*) calloc(1, sizeof(Inode));
	Sector* sector = (Sector*) calloc(1, sizeof(Sector));
	char* dadPath, block;
	char* sons;
	char* token, next, name;
	int i, size = MAX_BLOCK_FILE*(MAX_FILENAME_LEN+INDEX_SIZE), indexBlock, indexSector, indexInode, posCharStart;
	
	if (strlen(path)>MAX_PATHNAME_LEN) // control the pathname length
		return -1;

	token = strtok(path, "/");
	if (token != NULL)
	{
		next = strtok(NULL, "/");

		while (next != NULL)
		{
			strcat(dadPath, "/");
			strcat(dadPath, token);
			token = next;
			next = strtok(NULL, "/"); // read the next token
		}
	}
	else
		dadPath = "/";

	sons = (void*) calloc(1, size);

	if (Dir_Read(dadPath, sons, size)!=0)
		return -1;

	sons = (char*) sons;
	i=0;
	while (i<size && strcmp(name, token)!=0)
	{
		snprintf(name, MAX_FILENAME_LEN, sons+i);
		snprintf(indexInode, INDEX_SIZE, sons+MAX_FILENAME_LEN+i);
		i+=MAX_FILENAME_LEN+INDEX_SIZE;
	}

	if (strcmp(name, token)!=0) // not found the son
		return -1;

	indexBlock = (int)indexInode*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
	indexSector = (int)indexInode*sizeof(Inode)/SECTOR_SIZE+indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
	indexInode = (int)(indexInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

	Disk_Read(indexSector, sector->data);
	strcpy(block, sector->data);
	
	if (indexInode+sizeof(Inode)>SECTOR_SIZE)
	{
		Disk_Read(indexSector+1, sector->data);
		strcat(block, sector->data);
	}	

	// read the inode
	posCharStart=indexInode;
	snprintf(inode->type, 1, block+posCharStart); // read the type of inode
	posCharStart+=1; // add the char readden
	snprintf(inode->name, MAX_FILENAME_LEN, block+posCharStart); // read the name of file (directory)
	posCharStart+=MAX_FILENAME_LEN;
	snprintf(inode->size, MAX_FILE_SIZE_LEN, block+posCharStart); // read the size of file (directory)
	posCharStart+=MAX_FILE_SIZE_LEN;
	snprintf(inode->blocks, MAX_BLOCK_FILE*INDEX_SIZE, block+posCharStart); // read all block of inode

	printf("The size of %s is %s\n", path, inode->size);
	
	return 0;
}

int Dir_Read(char *path, void *buffer, int size) {
	printf("Dir_Read\n");
	Sector* sector = (Sector*) calloc(1, sizeof(Sector));
	Inode* dadInode = (Inode*) calloc(1, sizeof(Inode));
	Inode* sonInode = (Inode*) calloc(1, sizeof(Inode));
	char* block;
	char* token, buff;
	char* sons;
	char son[MAX_FILENAME_LEN+INDEX_SIZE];
	int posCharStart=0, i, indexBlock, indexSector, indexInode, noChar;

	if (sizeof(path)>MAX_PATHNAME_LEN) // control the path length
		return -1;

	token = strtok(path, "/"); // divide the path
	indexInode = 0;

	if (token == NULL) // case that the path is equals to root path
	{
		indexBlock = (int)indexInode*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
		indexSector = (int)indexInode*sizeof(Inode)/SECTOR_SIZE+indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
		indexInode = (int)(indexInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

		Disk_Read(indexSector, sector->data);
		strcpy(block, sector->data);
	
		if (indexInode+sizeof(Inode)>SECTOR_SIZE)
		{
			Disk_Read(indexSector+1, sector->data);
			strcat(block, sector->data);
		}	

		// read the inode
		posCharStart=indexInode;
		snprintf(dadInode->type, 1, block+posCharStart); // read the type of inode
		posCharStart+=1; // add the char readden
		snprintf(dadInode->name, MAX_FILENAME_LEN, block+posCharStart); // read the name of file (directory)
		posCharStart+=MAX_FILENAME_LEN;
		snprintf(dadInode->size, MAX_FILE_SIZE_LEN, block+posCharStart); // read the size of file (directory)
		posCharStart+=MAX_FILE_SIZE_LEN;
		snprintf(dadInode->blocks, MAX_BLOCK_FILE*INDEX_SIZE, block+posCharStart); // read all block of inode
	}

	while (token != NULL) // finchè ho token
	{
		indexBlock = (int)indexInode*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
		indexSector = (int)indexInode*sizeof(Inode)/SECTOR_SIZE+indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
		indexInode = (int)(indexInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

		Disk_Read(indexSector, sector->data);
		strcpy(block, sector->data);
		
		if (indexInode+sizeof(Inode)>SECTOR_SIZE)
		{
			Disk_Read(indexSector+1, sector->data);
			strcat(block, sector->data);
		}	

		// read the inode
		posCharStart=indexInode;
		snprintf(dadInode->type, 1, block+posCharStart); // read the type of inode
		posCharStart+=1; // add the char readden
		snprintf(dadInode->name, MAX_FILENAME_LEN, block+posCharStart); // read the name of file (directory)
		posCharStart+=MAX_FILENAME_LEN;
		snprintf(dadInode->size, MAX_FILE_SIZE_LEN, block+posCharStart); // read the size of file (directory)
		posCharStart+=MAX_FILE_SIZE_LEN;
		snprintf(dadInode->blocks, MAX_BLOCK_FILE*INDEX_SIZE, block+posCharStart); // read all block of inode

		i = 0;
		while (i<atoi(dadInode->size) && strcmp(sonInode->name, token)!=0) // while visit the all inode or found the directory
		{
			snprintf(buff, INDEX_SIZE, dadInode->blocks+i); // read the index
			indexInode = atoi(buff); // convert the index

			indexBlock = (int)indexInode*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
			indexSector = (int)indexInode*sizeof(Inode)/SECTOR_SIZE+indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
			indexInode = (int)(indexInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

			Disk_Read(indexSector, sector->data);
			strcpy(block, sector->data);
		
			if (indexInode+sizeof(Inode)>SECTOR_SIZE)
			{
				Disk_Read(indexSector+1, sector->data);
				strcat(block, sector->data);
			}

			// read the son inode
			posCharStart=indexInode;
			snprintf(sonInode->type, 1, block+posCharStart); // read the type of inode
			posCharStart+=1; // add the char readden
			snprintf(sonInode->name, MAX_FILENAME_LEN, block+posCharStart); // read the name of file (directory)
			posCharStart+=MAX_FILENAME_LEN;
			snprintf(sonInode->size, MAX_FILE_SIZE_LEN, block+posCharStart); // read the size of file (directory)
			posCharStart+=MAX_FILE_SIZE_LEN;
			snprintf(sonInode->blocks, MAX_BLOCK_FILE*INDEX_SIZE, block+posCharStart); // read all block of inode

			i+=INDEX_SIZE;
		}

		if (strcmp(sonInode->name, token)!=0) // if not found the directory
		{
			return -1; // return an error
		}
		else // if found it
		{
			dadInode = sonInode; // change the inode
			token = strtok(NULL, "/"); // charge the new token
		}
	}

	// found the directory inode
	sons = (char*) calloc(1, size); // create the buffer for sons
	i = 0;
	while (i*INDEX_SIZE<atoi(dadInode->size) && i*INDEX_SIZE<size) // visit all inode
	{
		snprintf(buff, INDEX_SIZE, dadInode->blocks+i*INDEX_SIZE); // read the index
		indexInode = atoi(buff); // convert the index

		indexBlock = (int)indexInode*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
		indexSector = (int)indexInode*sizeof(Inode)/SECTOR_SIZE+indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
		indexInode = (int)(indexInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

		Disk_Read(indexSector, sector->data);
		strcpy(block, sector->data);
	
		if (indexInode+sizeof(Inode)>SECTOR_SIZE)
		{
			Disk_Read(indexSector+1, sector->data);
			strcat(block, sector->data);
		}

		// read the son inode
		posCharStart=(int)(indexInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table
		snprintf(sonInode->type, 1, block+posCharStart); // read the type of inode
		posCharStart+=1; // add the char readden
		snprintf(sonInode->name, MAX_FILENAME_LEN, block+posCharStart); // read the name of file (directory)
		posCharStart+=MAX_FILENAME_LEN;
		snprintf(sonInode->size, MAX_FILE_SIZE_LEN, block+posCharStart); // read the size of file (directory)
		posCharStart+=MAX_FILE_SIZE_LEN;
		snprintf(sonInode->blocks, MAX_BLOCK_FILE*INDEX_SIZE, blockposCharStart); // read all block of inode

		for (noChar=0; noChar<MAX_FILENAME_SIZE+INDEX_SIZE; noChar++) // write all bytes
			if(noChar<strlen(sonInode->name))
				son[noChar] = sonInode->name[noChar]; // write the name of the son
			else if(noChar>MAX_FILENAME_SIZE)
				son[noChar] = buff[noChar-MAX_FILENAME_SIZE]; // write the index inode
			else
				son[noChar] = '0'; // write 0

		strcat(sons, son); // copy the son

		i++;
	}

	*buffer = (void) sons;

	return 0;
}

int Dir_Unlink(char *path) {				// TODO read father dir and grandfather dir

	Sector* sector = (Sector*) calloc(1, sizeof(Sector));
	Inode* dadInode = (Inode*) calloc(1, sizeof(Inode));
	Inode* sonInode = (Inode*) calloc(1, sizeof(Inode));
	char* fatherData[32*20], sonIndex[5];
	char* token, tmp, sonPos, sonName, block;
	int indexSonInode, indexSonSector, indexSonBlock, i;
	Sector* inodeBitmap = (Sector*) calloc(1, sizeof(Sector));

	if (sizeof(path)>MAX_PATHNAME_LEN) // control the path length
		return -1;

	sonName = strtok(path, "/");	
	while(sonName!=NULL){				// search for the dir name					 
		tmp = sonName;
		sonName = strtok(NULL,"/");
	}

	sonName = tmp;						//save the son name
	if (sonName==NULL) {
		osErrno = osErrno = E_ROOT_DIR;
		return -1;
	}

	sonPos =strstr(fatherData, sonName);	// calculate the son's position in fatherdata to find index

	if(sonPos ==  NULL) {			// the directory not exist
		return -1;				
	}
	else {							// copy the son's inode index
		i=0;
		while(i<INDEX_SIZE){
			sonIndex[i] = fatherData[sonPos+strlen(sonName)+i]
		}
	}

	indexSonInode = atoi(sonIndex);
	indexSonBlock = (int)indexSonInode*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
	indexSonSector = (int)indexSonInode*sizeof(Inode)/SECTOR_SIZE+indexSonBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
	indexSonInode = (int)(indexSonInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table
	
	if(Disk_Read(indexSonSector, sector->data))
		return -1;		//read son's inode info
	strcpy(block, sector->data);

	if (indexSonInode+sizeof(Inode)>SECTOR_SIZE)   // if inode is splitted between two sectors
		{
			if(Disk_Read(indexSonInode+1, sector->data))
				return -1;
			strcat(block, sector->data);
		}

	// read son's informations
	posCharStart=(int)(indexSonInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table
	snprintf(sonInode->type, 1, block+posCharStart); // read the type of inode
	posCharStart+=1; // add the char readden
	snprintf(sonInode->name, MAX_FILENAME_LEN, block+posCharStart); // read the name of file (directory)
	posCharStart+=MAX_FILENAME_LEN;
	snprintf(sonInode->size, MAX_FILE_SIZE_LEN, block+posCharStart); // read the size of file (directory)
	

	if(atoi(sonInode->size)!=0){		// if the son isn't empty
		osErrno = E_DIR_NOT_EMPTY;
		return -1;
	}

	// TODO read Grandfather to find father inode and read father inode info, free inode bitmap cell

	posIndextodelete = strstr(dadInode->blocks, sonIndex);
	for(i=0; i<INDEX_SIZE; i++)											// delete father "link" to the son
		dadInode->blocks[i+sonIndex] = dadInode->blocks[size + 1 - INDEX_SIZE + i];

	size -=INDEX_SIZE; 

	// delete inode of the son

	char* inodeInfo = (char*) calloc(1, sizeof(Inode));
	if(indexSonInode+sizeof(Inode)<SECTOR_SIZE) // if the inode is less than size free into the inode
		strcpy(sector->data+indexSonInode, inodeInfo); // write the inode into the sector
	else
	{
		for(i=0, noChar=indexSonInode; i<sizeof(Inode); i++, noChar++)
		{
			if(i+indexSonInode<SECTOR_SIZE) // if the sector is full
			{
				if(Disk_Write(indexSonSector, sector->data))
					return -1;; // write the first sector
				indexSonSector++; // increment the sector
				if(Disk_Read(indexSonSector, sector->data))
					return -1; // read the new sector
				noChar = 0; // reset the number char written
			}

			sector->data[noChar] = inodeInfo[i]; // write the char into the sector
		}
	}

	indexSonInode = atoi(sonIndex);
	if(indexSonInode/sizeof(Inode)<512){

		if(Disk_Read(1*BLOCK_SIZE/SECTOR_SIZE, inodeBitmap->data))
			return -1;
		inodeBitmap->data[indexSonInode] = 0;
		if(Disk_Write(1*BLOCK_SIZE/SECTOR_SIZE, inodeBitmap->data))
			return -1;
	}
	else {
		if(Disk_Read(1*BLOCK_SIZE/SECTOR_SIZE+1, inodeBitmap->data))
			return -1;
		inodeBitmap->data[indexSonInode%SECTOR_SIZE] = 0;
		if(Disk_Write(1*BLOCK_SIZE/SECTOR_SIZE+1, inodeBitmap->data))
			return -1;
	}
	printf("Dir_Unlink\n");
	return 0;
	
}
