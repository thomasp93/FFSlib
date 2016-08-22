#include "dummydisk.c"
#include "ffslib.h"

// global errno value here
int osErrno;

// global variables
int INODE_TABLE_BEGIN;
int DATA_BLOCK_BEGIN;

////////////////////////////////////////////////////////////////////////
// funzione che crea il tutto, va chiamata una volta sola all'inizio
int FS_Init(char *path) {
	Block* block = (Block*) calloc(1, sizeof(Block)); // create block
	Block* tmp; // temporary block
	Sector* sector;
	char* s; // temporary string
	int i, create, noChar=0, noSectorVisited=1;

	printf("FS_Init %s\n", path);

	INODE_TABLE_BEGIN = SUPERBLOCK_INDEX+INODE_BITMAP_INDEX+DATA_BLOCK_INDEX; // set the begin index inode table
	DATA_BLOCK_BEGIN = 150*MAX_INODE+INODE_TABLE_BEGIN; // set the begin index data blocks

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
	block = NULL;

	for(i=0; i<BLOCK_SIZE/SECTOR_SIZE; i++) // creation list sector
	{
		tmp = (Block*) calloc(1, sizeof(Block)); // create a new block
		tmp->sector = (Sector*) calloc(1, sizeof(Sector)); // create a new sector
		tmp->next = block; // insert the new block in the head of the list
		block = tmp;
	}

	if(create==0) // set disk
	{
		s = (char*) calloc(1, SECTOR_SIZE*sizeof(char));
		strcat(s, TYPE_FILESYSTEM);
		noChar += strlen(s);

		tmp = block;
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
			sector->data[i] = '0'; 
		
		for(i=1; i<NUM_SECTORS; i++) // set all sector with zeros
			Disk_Write(i, sector->data);

		Dir_Create("/"); // create the root directory
	}
	else // read exist disk
	{
		i=0; // first block for get super block

		tmp = block;
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
	printf("File_Create\n");

	Sector* sector = (Sector*) calloc(1, sizeof(Sector));
	Inode* dadInode = (Inode*) calloc(1, sizeof(Inode));
	char* buff;
	char* subString;
	char* block;
	char* granfather;
	char* dad;
	char* son;
	char* next;
	char* granPath;
	char* sons;
	char* token;
	char* dadPath;
	char* name;
	char* index;
	char c;
	int i, position, indexBlock, indexSector, indexInode, noChar, size;
	int indexInodeSon, indexInodeDad, posCharStart;

	if (strlen(file)>MAX_PATHNAME_LEN)
	{
		osErrno = E_CREATE;
		return -1;
	}
	
	token = strtok(file, "/");
	if (token != NULL)
	{
		next = strtok(NULL, "/");

		if (next != NULL)
		{
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

		size = MAX_BLOCK_FILE*(INDEX_SIZE+MAX_FILENAME_LEN);
		sons = (char*) calloc(1, size);

		if (Dir_Read(dadPath, (void*)sons, size)!=0)
			return -1;

		sons = (char*) sons;
		i=0;
		do
		{
			snprintf(name, MAX_FILENAME_LEN, sons+i);
			snprintf(index, INDEX_SIZE, sons+MAX_FILENAME_LEN+i);
			i+=MAX_FILENAME_LEN+INDEX_SIZE;
		} while (i<size && strcmp(name, token)!=0);

		if (strcmp(name, token)==0) // file already created
		{
			osErrno = E_CREATE;
			return -1;
		}
	}
	else // file name not valid
	{
		osErrno = E_CREATE;
		return -1;
	}

	// save the inode into the inode table
	for(i=0; i<BLOCK_SIZE/SECTOR_SIZE; i++) // visit the inode bitmap
	{
		indexInode=0;
		Disk_Read(BLOCK_SIZE/SECTOR_SIZE+i, sector->data); // read bitmap inode
		while(c!='0' && indexInode<SECTOR_SIZE) // while c is not equals to 0 and index inode is less than number inode into the sector
		{
			c = sector->data[indexInodeSon]; // read index inode char
			indexInodeSon++; // increment index
		}

		if(c=='0') // find the free inode
			break; // exit from for
	}

	if(c!='0') // if not exist inode free
	{
		osErrno = E_CREATE;
		return -1;
	}
	else
		indexInodeSon -= 1; // correct the more increment of index

	// else I found the index of free inode
	indexInodeSon += i*SECTOR_SIZE; // calculate the true value of free index inode
	sector->data[indexInodeSon] = '1'; // set the inode as busy
	Disk_Write(BLOCK_SIZE/SECTOR_SIZE+i, sector->data); // write the edit sector into the disk

	// write the inode into the inode table
	indexBlock = (int)indexInodeSon*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
	indexSector = (int)indexInodeSon*sizeof(Inode)/SECTOR_SIZE+indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
	indexInode = (int)(indexInodeSon*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

	// take the granfather's path
	granfather = strtok(file, "/");

	if (granfather != NULL) // root case /
	{
		dad = strtok(NULL, "/");

		if (dad != NULL) // /home/thomas gran=home dad=thomas
		{
			strcat(granPath, "/"); // /

			son = strtok(NULL, "/");

			if (son != NULL) // /home/thomas/Scrivania gran=home dad=thomas son=Scrivania
			{
				strcat(granPath, granfather); // /home

				next = strtok(NULL, "/"); // /home/thomas/Scrivania/pippo gran=home dad=thomas son=Scrivania next=pippo

				while (next != NULL)
				{
					granfather = dad;
					dad = son;
					son = next;
					strcat(granPath, "/");
					strcat(granPath, granfather);
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
		dadInode->type = block[posCharStart]; // read the type of inode
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

		strcat(inodeInfo, &dadInode->type);
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
	else
		return -1; // file name not valid

	// creation a file
	Inode* fileInode = (Inode*) calloc(1, sizeof(Inode));
	fileInode->type = 'f'; // set the type as file
	strcpy(fileInode->name, son); // copy the name of file
	strcpy(fileInode->size, "00000"); // set the size of the file

	Disk_Read(indexSector, sector->data); // read a sector that it contains the free inode
	
	noChar=0; // reset the number of char written into the inode block
	char* inodeInfo = (char*) calloc(1, sizeof(Inode)); // create the inode

	inodeInfo[indexInode+noChar] = fileInode->type; // write the file type
	noChar++;

	strcpy(inodeInfo+noChar, fileInode->name);
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

////////////////////////////////////////////////////////////////////////

int File_Open(char *file) {
	printf("File_Open\n");
	File* fileType = (File*) calloc(1, sizeof(File));
	Inode* inode = (Inode*) calloc(1, sizeof(Inode));
	Sector* sector = (Sector*) calloc(1, sizeof(Sector));
	char* dad;
	char* son;
	char* next;
	char* dadPath;
	char* sons;
	char* block;
	int i=0, fd, indexInodeSon, indexBlock, indexSector, indexInode, size, posCharStart;

	// find a place into the openFileTable
	while (openFiles->fileOpen[i] != NULL && i+1<MAX_FILE_OPEN) // while the place isn't empty
		i++;

	if (openFiles->fileOpen[i]!=NULL) // don't have empty place
	{
		osErrno = E_TOO_MANY_OPEN_FILES;
		return -1;
	}
	else
		fd = i; // save the fd

	// dad path

	// take the granfather's path
	dad = strtok(file, "/");

	if (dad != NULL) // /home dad=home
	{
		strcat(dadPath, "/"); // /

		son = strtok(NULL, "/");

		if (son != NULL) // /home/thomas dad=home son=thomas
		{
			strcat(dadPath, dad); // /home

			next = strtok(NULL, "/"); // /home/thomas/Scrivania/pippo.txt dad=home son=thomas next=Scrivania

			while (next != NULL)
			{
				dad = son;
				son = next;
				strcat(dadPath, "/");
				strcat(dadPath, dad);
				next = strtok(NULL, "/"); // read the next token
			}		
		}		
	} // end root case
	else
	{
		osErrno = E_NO_SUCH_FILE;
		return -1; // file name not valid
	}

	size = MAX_BLOCK_FILE*(INDEX_SIZE+MAX_FILENAME_LEN);
	sons = (char*) calloc(1, size);
	if (Dir_Read(dadPath, (void*)sons, size) != 0) // read the dad directory
		return -1;

	indexInodeSon = atoi(strstr(sons, son)+MAX_FILENAME_LEN); // find the index of son's inode

	indexBlock = (int)indexInodeSon*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
	indexSector = (int)indexInodeSon*sizeof(Inode)/SECTOR_SIZE+indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
	indexInode = (int)(indexInodeSon*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

	Disk_Read(indexSector, sector->data);
	strcpy(block, sector->data);

	if (indexInode+sizeof(Inode)>SECTOR_SIZE)
	{
		Disk_Read(indexSector+1, sector->data);
		strcat(block, sector->data);
	}	

	// load inode

	// read the file's inode
	posCharStart=indexInode;
	inode->type = block[posCharStart]; // read the type of inode
	posCharStart+=1; // add the char readden
	snprintf(inode->name, MAX_FILENAME_LEN, block+posCharStart); // read the name of file
	posCharStart+=MAX_FILENAME_LEN;
	snprintf(inode->size, MAX_FILE_SIZE_LEN, block+posCharStart); // read the size of file
	posCharStart+=MAX_FILE_SIZE_LEN;
	snprintf(inode->blocks, MAX_BLOCK_FILE*INDEX_SIZE, block+posCharStart); // read all block of inode

	// create file
	fileType->indexInode = indexInodeSon; // write the global index of inode
	fileType->iopointer = 0; // set the iopointer with zero
	fileType->info = inode; // set the info of inode with the file inode

	// insert the file into the openFileTable
	openFiles->fileOpen[fd] = fileType;

	return fd; // return the place of file
}

////////////////////////////////////////////////////////////////////////

int File_Read(int fd, void *buffer, int size) {
	printf("File_Read\n");
	Sector* sector = (Sector*) calloc(1, sizeof(Sector));
	char* block;
	char* addressBlock;
	int noCharRead = 0, i, indexBlock, addBlock, indexSector, index;

	if (openFiles->fileOpen[fd] == NULL) // file not open
	{
		osErrno = E_BAD_FD;
		return -1;
	}

	// file open
	if(openFiles->fileOpen[fd]->iopointer+size<MAX_BLOCK_FILE*BLOCK_SIZE) // control the number of chars to read
		noCharRead = size; // read all chars
	else
		noCharRead = MAX_BLOCK_FILE*BLOCK_SIZE-openFiles->fileOpen[fd]->iopointer; // read chars until end of file

	indexBlock = openFiles->fileOpen[fd]->iopointer/BLOCK_SIZE; // calculate the index of block that the iopointer point

	snprintf(addressBlock, INDEX_SIZE, openFiles->fileOpen[fd]->info->blocks+indexBlock*INDEX_SIZE); // read the address of the data block
	addBlock=atoi(addressBlock);

	// find the block and sector index
	indexBlock = (int)addBlock+DATA_BLOCK_BEGIN; // calculate the index of block
	indexSector = (int)indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the data blocks
	index = (int)indexBlock%BLOCK_SIZE; // recalculate the index of iopointer into the data block

	if (index>SECTOR_SIZE) // if the index is more than sector size
		indexSector+=(int)index/SECTOR_SIZE; // increment the index of sector

	if (Disk_Read(indexSector, sector->data)!=0) // read the sector
		return -1;
	strcat(block, sector->data); // concatenate the sector into the block

	i=1;
	while (index+noCharRead>i*SECTOR_SIZE) // while the number of chars read isn't contains into sectors
	{
		if (Disk_Read(indexSector+i, sector->data)!=0) // read the next sector
			return -1;
		strcat(block, sector->data); // concatenate the sector into the block
		i++; // increment the number of sector readen
	}

	snprintf(buffer, noCharRead, block+index); // read the chars into the block

	return 0;
}

////////////////////////////////////////////////////////////////////////

int File_Write(int fd, void *buffer, int size) {
	int noBlock, blockPos, i, datablock, sectorPos, newSize, j, stop, noSector;
	char datablockIndex[INDEX_SIZE], datablockBitmap[BLOCK_SIZE];

	Sector* sector = (Sector *) calloc(1, sizeof(Sector));
	Sector* datablockBitmap_1 = (Sector *) calloc(1, sizeof(Sector));
	Sector* datablockBitmap_2 = (Sector *) calloc(1, sizeof(Sector));
	if(openFiles->fileOpen[fd] == NULL){
		osErrno = E_BAD_FD;
		return -1;
	}
	noBlock = (openFiles->fileOpen[fd]->iopointer)/BLOCK_SIZE;										// find the datablock index and position
	blockPos = (openFiles->fileOpen[fd]->iopointer)%BLOCK_SIZE;
	sectorPos = blockPos%SECTOR_SIZE;
	noSector = blockPos/SECTOR_SIZE;																//no of sector of the block
	if(blockPos+size < BLOCK_SIZE){

		snprintf(datablockIndex, 5, (openFiles->fileOpen[fd]->info->blocks)+noBlock*INDEX_SIZE);	//read the data block index
		datablock = atoi(datablockIndex) + DATA_BLOCK_BEGIN;									 	//find the real data block index
																
		if(Disk_Read(datablock*BLOCK_SIZE/SECTOR_SIZE+noSector, sector->data)!=0)					//read the right sector
			return -1;					
																 
		for(i=0; i<size;i++){																		//write the buffer
			if(sectorPos+i>=SECTOR_SIZE){															//if i reach the sector limit pass to next sector
				if(Disk_Write(datablock*BLOCK_SIZE/SECTOR_SIZE+noSector, sector->data)!=0)					
					return -1;
				noSector++;
				if(Disk_Read(datablock*BLOCK_SIZE/SECTOR_SIZE+noSector, sector->data)!=0)			
					return -1;
				sectorPos = 0;
			}
			sector->data[sectorPos+i] = ((char*) buffer)[i];
		}

		if(Disk_Write(datablock*BLOCK_SIZE/SECTOR_SIZE+noSector, sector->data)!=0)					//save the sector update
			return -1;	

		newSize = atoi(openFiles->fileOpen[fd]->info->size) + size;									//update the inode size
		snprintf(openFiles->fileOpen[fd]->info->size, INDEX_SIZE, "%05d", newSize);
	}
	else{																							//if the buffer needs more than a block
		i=0;
		if(Disk_Read(2*BLOCK_SIZE/SECTOR_SIZE, datablockBitmap_1->data)!=0)							//load data block bitmap to find free block if needed
			return -1;
		if(Disk_Read(2*BLOCK_SIZE/SECTOR_SIZE, datablockBitmap_2->data)!=0)
			return -1;
		strcpy(datablockBitmap, datablockBitmap_1->data);
		strcat(datablockBitmap, datablockBitmap_2->data);

		snprintf(datablockIndex, 5, (openFiles->fileOpen[fd]->info->blocks)+noBlock*INDEX_SIZE);	//read the data block index
		datablock = atoi(datablockIndex) + DATA_BLOCK_BEGIN;

		while(i<size){

			if(blockPos+i>=BLOCK_SIZE){															//if i reach the block dimension i search for the next block or a new one
				noBlock++;
				if(noBlock>MAX_BLOCK_FILE)														//if i reach max file limit 
					return -1;
				blockPos=0;																		//reset some counters
				noSector=0;
				sectorPos=0;
				snprintf(datablockIndex, 5, (openFiles->fileOpen[fd]->info->blocks)+noBlock*INDEX_SIZE);	//read the data block index
				if(strcmp(datablockIndex, "00000")!=0){														//if the file has already one block allocated
					datablock = atoi(datablockIndex) + DATA_BLOCK_BEGIN;	
				}
				else{																						//search for a new one
					for(j=0; j<BLOCK_SIZE;j++){
						if(datablockBitmap[j]==0){
							datablockBitmap[j] = 1;
							if(j/SECTOR_SIZE == 0){
								datablockBitmap_1->data[j]= '\0';
								Disk_Write(2*BLOCK_SIZE/SECTOR_SIZE, datablockBitmap_1->data);
								break;
							}
							else if(j/SECTOR_SIZE == 1)
								datablockBitmap_2->data[j]= '\0';
								Disk_Write(2*BLOCK_SIZE/SECTOR_SIZE+1, datablockBitmap_2->data);
								break;
						}
					}
					datablock = j + DATA_BLOCK_BEGIN;									// copy the new block index into the inode
					snprintf(datablockIndex, INDEX_SIZE, "%04d", datablock);
					strcat(openFiles->fileOpen[fd]->info->blocks, datablockIndex);
					
				}
			}
			if(Disk_Read(datablock*BLOCK_SIZE/SECTOR_SIZE+noSector, sector->data)!=0)					//read the right sector
						return -1;	
			for(j=sectorPos;j<SECTOR_SIZE;j++){
				sector->data[j]= ((char*) buffer)[i];
				i++;
				blockPos++;
			}
			noSector++;
		}
	}
	snprintf(openFiles->fileOpen[fd]->info->size, MAX_FILE_SIZE_LEN, "%05d", atoi(openFiles->fileOpen[fd]->info->size)+size);
	printf("File_Write\n");
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
	printf("File_Seek\n");
	return 0;   
}

////////////////////////////////////////////////////////////////////////

int File_Close(int fd) {
	printf("File_Close\n");
	Sector* sector = (Sector*) calloc(1, sizeof(Sector));
	int i, indexBlock, indexSector, indexInode, noChar;

	if (openFiles->fileOpen[fd] == NULL) // file not open
	{
		osErrno = E_BAD_FD;
		return -1;
	}

	indexInode = openFiles->fileOpen[fd]->indexInode; // take the global index of inode of the file	

	// write the inode into the inode table
	indexBlock = (int)indexInode*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
	indexSector = (int)indexInode*sizeof(Inode)/SECTOR_SIZE+indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
	indexInode = (int)(indexInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

	noChar=0; // reset the number of char written into the inode block
	char* inodeInfo = (char*) calloc(1, sizeof(Inode)); // create the inode

	inodeInfo[indexInode+noChar] = openFiles->fileOpen[fd]->info->type; // write the file type
	noChar++;

	strcpy(inodeInfo+noChar, openFiles->fileOpen[fd]->info->name);
	noChar+=MAX_FILENAME_LEN;

	strcpy(inodeInfo+noChar, openFiles->fileOpen[fd]->info->size);
	noChar+=5;

	strcpy(inodeInfo+noChar, openFiles->fileOpen[fd]->info->blocks);

	if (Disk_Read(indexSector, sector->data)!=0) // read a sector that it contains the inode of the file
	{
		osErrno = E_CREATE;
		return -1;
	}

	if(indexInode+sizeof(Inode)<SECTOR_SIZE) // if the inode is less than size of the inode
		strcpy(sector->data+indexInode, inodeInfo); // write the inode into the sector
	else
	{
		for(i=0, noChar=indexInode; i<sizeof(Inode); i++, noChar++)
		{
			if(i+indexInode<SECTOR_SIZE) // if the sector is full
			{
				if (Disk_Write(indexSector, sector->data)!=0) // write the first sector
				{
					osErrno = E_CREATE;
					return -1;
				}
				indexSector++; // increment the sector
				if (Disk_Read(indexSector, sector->data)!=0) // read the new sector
				{
					osErrno = E_CREATE;
					return -1;
				}
				noChar = 0; // reset the number char written
			}

			sector->data[noChar] = inodeInfo[i]; // write the char into the sector
		}
	}

	if (Disk_Write(indexSector, sector->data)!=0) // write the sector into the disk
	{
		osErrno = E_CREATE;
		return -1;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////

int File_Unlink(char *file) {


	Inode* fileType= (Inode *) calloc(1, sizeof(Inode));
	Inode* dir= (Inode *) calloc(1, sizeof(Inode));
	char* filename;
	char* tmp;
	char* dirpath;
	char* fatherpath;
	char dircontent[MAX_BLOCK_FILE*(MAX_FILENAME_LEN+INDEX_SIZE)];
	char* filePos;
	char* dirname;
	char* dirPos;
	char* next;

	int i, fileRelInode, fileSector, fileBlock, fileInode, charPos, noFileBlocks, dirRelInode, dirSector, dirBlock, posCharStart, posChar, dirInode, blockread;
	char fileIndex[INDEX_SIZE];
	char block[BLOCK_SIZE];
	char dirIndex[INDEX_SIZE];
	Sector* sector= (Sector*) calloc(1, sizeof(Sector));
	Sector* inodeBitmap = (Sector*) calloc(1, sizeof(Sector));
	Sector* block_bitmap_1= (Sector*) calloc(1, sizeof(Sector));
	Sector* block_bitmap_2= (Sector*) calloc(1, sizeof(Sector));
	Sector* inode_bitmap= (Sector*) calloc(1, sizeof(Sector));
	
	tmp = strtok(file, "/");
	if (tmp != NULL) // root case /
	{
		dirpath = strtok(NULL, "/");

		if (dirpath != NULL) // /home/thomas gran=home dad=thomas
		{
			strcat(fatherpath, "/"); // /

			filename = strtok(NULL, "/");

			if (filename != NULL) // /home/thomas/Scrivania gran=home dad=thomas son=Scrivania
			{
				strcat(fatherpath, tmp); // /home

				next = strtok(NULL, "/"); // /home/thomas/Scrivania/pippo gran=home dad=thomas son=Scrivania next=pippo

				while (next != NULL)
				{
					tmp = dirpath;
					dirpath = filename;
					filename = next;
					strcat(fatherpath, "/");
					strcat(fatherpath, tmp);
					next = strtok(NULL, "/"); // read the next token
				}
			}
																										// find the grandfather dir
			if(Dir_Read(fatherpath,(void *) dircontent, MAX_BLOCK_FILE*(MAX_FILENAME_LEN+INDEX_SIZE))!=0)
				return -1; // read to find dir inode
			dirPos = strstr(dircontent, dirname);														//save the position of the dirname to find inode 
			snprintf(dirIndex, INDEX_SIZE, "%04d", dirPos+MAX_FILENAME_LEN); // ??????????????????????????????????????????
		}
		else
			for (i=0; i<INDEX_SIZE; i++)
				dirIndex[i] = '0';

	}

	Dir_Read(dirpath,(void *)dircontent, MAX_BLOCK_FILE*(MAX_FILENAME_LEN+INDEX_SIZE));	//read the dir content to find the file inode
	filePos = strstr(dircontent, filename);
	if(filePos==NULL){
		return -1;
	}
	else
		snprintf(fileIndex, INDEX_SIZE, dircontent+strlen(filePos)+MAX_FILENAME_LEN); //copy the file inode index
	
	fileRelInode = atoi(fileIndex);
	fileBlock = (int)fileRelInode*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
	fileSector = (int)fileRelInode*sizeof(Inode)/SECTOR_SIZE+fileBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
	fileInode = (int)(fileRelInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table
	if(Disk_Read(fileSector, sector->data)!=0)
		return -1;
	strcpy(block, sector->data);
	if(fileInode+sizeof(Inode)>SECTOR_SIZE){					//read the inode content to erase it
		if(Disk_Read(fileSector+1, sector->data)!=0)
			return -1;
		strcat(block, sector->data);
	}

	// read the block bitmap
	if(Disk_Read(2*BLOCK_SIZE/SECTOR_SIZE,block_bitmap_1->data)!=0)
		return -1;
	if(Disk_Read(2*BLOCK_SIZE/SECTOR_SIZE+1,block_bitmap_2->data)!=0)
		return -1;


	posChar=fileInode; // recalculate the index of inode into the inode table
	fileType->type = block[posCharStart]; // read the type of inode
	posCharStart+=1; // add the char readden
	snprintf(fileType->name, MAX_FILENAME_LEN, block+posCharStart); // read the name of file (directory)
	posCharStart+=MAX_FILENAME_LEN;
	snprintf(fileType->size, MAX_FILE_SIZE_LEN, block+posCharStart); // read the size of file (directory)
	posCharStart+=MAX_FILE_SIZE_LEN;
	noFileBlocks = atoi(fileType->size)/BLOCK_SIZE;			// calculate how much blocks use the file
	for(i=0; i<noFileBlocks; i++)					// for every block of the file delete the content
	{
		snprintf(tmp, INDEX_SIZE, block+posCharStart);
		posCharStart+=INDEX_SIZE;
		if(Disk_Write(atoi(tmp)*BLOCK_SIZE/SECTOR_SIZE, sector->data)!=0)
			return -1;
		if(Disk_Write(atoi(tmp)*BLOCK_SIZE/SECTOR_SIZE+1, sector->data)!=0)
			return -1;
		if(atoi(tmp)<512)											//free the block bitmap
			block_bitmap_1->data[atoi(tmp)] = '0';
		else if(atoi(tmp)>=512 && atoi(tmp)<1024)
			block_bitmap_2->data[atoi(tmp)%SECTOR_SIZE] = '0';
		else
			return -1;
	}

	

	// free inode bitmap index
	if(fileRelInode < 512){					//if the inode index is in the first part of the bitmap
		
		if(Disk_Read(1*BLOCK_SIZE/SECTOR_SIZE, inodeBitmap->data)!=0)
			return -1;
		inodeBitmap->data[fileRelInode] = '0';
		if(Disk_Write(1*BLOCK_SIZE/SECTOR_SIZE, inodeBitmap->data)!=0)
			return -1;
	}
	else if (fileRelInode >= 512 && fileRelInode < 1024){		// if it's in the second one

		if(Disk_Read(1*BLOCK_SIZE/SECTOR_SIZE+1, inodeBitmap->data)!=0)
			return -1;
		inodeBitmap->data[fileRelInode%SECTOR_SIZE] = '0';
		if(Disk_Write(1*BLOCK_SIZE/SECTOR_SIZE+1, inodeBitmap->data)!=0)
			return -1;
	}

	



	if(Disk_Write(2*BLOCK_SIZE/SECTOR_SIZE,block_bitmap_1->data)!=0)
		return -1;
	if(Disk_Write(2*BLOCK_SIZE/SECTOR_SIZE+1,block_bitmap_2->data)!=0)
		return -1;

	dirRelInode = atoi(dirIndex);
	dirBlock = (int)dirRelInode*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
	dirSector = (int)dirRelInode*sizeof(Inode)/SECTOR_SIZE+dirBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
	dirInode = (int)(dirRelInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table
	
	if (Disk_Read(dirSector, sector->data)!=0)
		return -1;
	blockread=0;
	strcpy(block, (sector->data));
	
	if (dirInode+sizeof(Inode)>SECTOR_SIZE)
	{
		if(Disk_Read(dirSector+1, sector->data)!=0)
			return -1;
		strcat(block, sector->data);
		blockread=1;
	}	
	posCharStart=dirInode;
	dir->type = block[posCharStart]; // read the type of inode
	posCharStart+=1; // add the char readden
	snprintf(dir->name, MAX_FILENAME_LEN, block+posCharStart); // read the name of file (directory)
	posCharStart+=MAX_FILENAME_LEN;
	snprintf(dir->size, MAX_FILE_SIZE_LEN, block+posCharStart); // read the size of file (directory)
	posCharStart+=MAX_FILE_SIZE_LEN;
	snprintf(dir->blocks, MAX_BLOCK_FILE*INDEX_SIZE, block+posCharStart); // read all block of inode

	filePos = strstr(dir->blocks, fileIndex);
	for(i=0; i<INDEX_SIZE; i++)											// delete father "link" to the son
	{
		*(filePos+i) = dir->blocks[atoi(dir->size) - INDEX_SIZE + i];
		dir->blocks[atoi(dir->size)- INDEX_SIZE + i] = '0'; // reset the memory deleted
	}

	snprintf(dir->size, MAX_FILE_SIZE_LEN, "%05d", atoi(dir->size)-INDEX_SIZE); // deincrement the size of directory


	if(blockread=0){								//if i read only a sector
		snprintf(sector->data, SECTOR_SIZE, block);
		if(Disk_Write(dirSector, sector->data)!=0)
			return -1;
	}
	else{
		snprintf(sector->data, SECTOR_SIZE, block);
		if(Disk_Write(dirSector, sector->data)!=0)
			return -1;
		snprintf(sector->data, SECTOR_SIZE, block+SECTOR_SIZE);
		if(Disk_Write(dirSector+1, sector->data)!=0)
			return -1;
	}


	free(block_bitmap_1);				//free the used structures
	free(block_bitmap_2);
	free(inodeBitmap);
	free(sector);
	free(file);
	free(dir);						
	printf("File_Unlink\n");

	return 0;
}

////////////////////////////////////////////////////////////////////////
// directory ops
////////////////////////////////////////////////////////////////////////

int Dir_Create(char *path) {
	Sector* sector = (Sector*) calloc(1, sizeof(Sector));
	Inode* dadInode = (Inode*) calloc(1, sizeof(Inode));
	char* buff;
	char* subString;
	char* block;
	char* granfather;
	char* dad;
	char* son;
	char* next;
	char* granPath;
	char* gran;
	char* sons;
	int i, c, position, indexBlock, indexSector, indexInode, noChar, size;
	int indexInodeSon, indexInodeDad, posCharStart;

	if (strlen(path)>MAX_PATHNAME_LEN)
	{
		osErrno = E_CREATE;
		return -1;
	}
	
    	printf("Dir_Create %s\n", path);
	
	// save the inode into the inode table
	for(i=0; i<BLOCK_SIZE/SECTOR_SIZE; i++) // visit the inode bitmap
	{
		indexInode=0;
		if (Disk_Read(BLOCK_SIZE/SECTOR_SIZE+i, sector->data) != 0) // read bitmap inode
		{
			osErrno = E_CREATE;
			return -1;
		}

		while(c!='0' && indexInode<SECTOR_SIZE) // while c is not equals to 0 and index inode is less than number inode into the sector
		{
			c = sector->data[indexInodeSon]; // read index inode char
			indexInodeSon++; // increment index
		}

		if(c=='0') // find the free inode
			break; // exit from for
	}

	if(c!='0') // if not exist inode free
	{
		osErrno = E_CREATE;
		return -1;
	}
	else
		indexInodeSon -= 1; // correct the more increment of index

	// else I found the index of free inode
	indexInodeSon += i*SECTOR_SIZE; // calculate the true value of free index inode
	sector->data[indexInodeSon] = '1'; // set the inode as busy
	if (Disk_Write(BLOCK_SIZE/SECTOR_SIZE+i, sector->data) != 0) // write the edit sector into the disk
	{
		osErrno = E_CREATE;
		return -1;
	}

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

			size = MAX_BLOCK_FILE*(INDEX_SIZE+MAX_FILENAME_LEN);
			sons = (void*) calloc(1, size);
			if (Dir_Read(granPath, sons, size) != 0)
			{
				osErrno = E_CREATE;
				return -1;
			}

			indexInodeDad = atoi(strstr(sons, dad)+MAX_FILENAME_LEN); // find the dad's inode
		}
		else // /home
			indexInodeDad = 0; // dad inode found

		indexBlock = (int)indexInodeDad*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
		indexSector = (int)indexInodeDad*sizeof(Inode)/SECTOR_SIZE+indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
		indexInode = (int)(indexInodeDad*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

		if (Disk_Read(indexSector, sector->data)!=0)
		{
			osErrno = E_CREATE;
			return -1;
		}
		strcpy(block, sector->data);
	
		if (indexInode+sizeof(Inode)>SECTOR_SIZE)
		{
			if (Disk_Read(indexSector+1, sector->data)!=0)
			{
				osErrno = E_CREATE;
				return -1;
			}
			strcat(block, sector->data);
		}	

		// read the dad's inode
		posCharStart=indexInode;
		dadInode->type = block[posCharStart]; // read the type of inode
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

		strcat(inodeInfo, &dadInode->type);
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
					if (Disk_Write(indexSector, sector->data)!=0) // write the first sector
					{
						osErrno = E_CREATE;
						return -1;
					}
					indexSector++; // increment the sector
					if (Disk_Read(indexSector, sector->data)!=0) // read the new sector
					{
						osErrno = E_CREATE;
						return -1;
					}
					noChar = 0; // reset the number char written
				}

				sector->data[noChar] = inodeInfo[i]; // write the char into the sector
			}
		}

		if (Disk_Write(indexSector, sector->data)!=0) // write the sector into the disk
		{
			osErrno = E_CREATE;
			return -1;
		}
	} // end root case

	// creation a directory
	Inode* dir = (Inode*) calloc(1, sizeof(Inode));
	dir->type = 'd'; // set the type as directory
	strcpy(dir->name, son); // copy the name of directory
	strcpy(dir->size, "00000"); // set the size of the directory

	if (Disk_Read(indexSector, sector->data)!=0) // read a sector that it contains the free inode
	{
		osErrno = E_CREATE;
		return -1;
	}
	
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
				if (Disk_Write(indexSector, sector->data)!=0) // write the first sector
				{
					osErrno = E_CREATE;
					return -1;
				}
				indexSector++; // increment the sector
				if (Disk_Read(indexSector, sector->data)!=0) // read the new sector
				{
					osErrno = E_CREATE;
					return -1;
				}
				noChar = 0; // reset the number char written
			}

			sector->data[noChar] = inodeInfo[i]; // write the char into the sector
		}
	}

	if (Disk_Write(indexSector, sector->data)!=0) // write the sector into the disk
	{
		osErrno = E_CREATE;
		return -1;
	}
    
    	return 0;
}

int Dir_Size(char *path) {
	printf("Dir_Size\n");
	Inode* inode = (Inode*) calloc(1, sizeof(Inode));
	Sector* sector = (Sector*) calloc(1, sizeof(Sector));
	char* dadPath;
	char* block;
	char* index;
	char* sons;
	char* token;
	char* next;
	char* name;
	int i, size = MAX_BLOCK_FILE*(MAX_FILENAME_LEN+INDEX_SIZE), indexBlock, indexSector, indexInode, posCharStart;
	
	if (strlen(path)>MAX_PATHNAME_LEN) // control the pathname length
		return -1;

	token = strtok(path, "/");
	if (token != NULL)
	{
		next = strtok(NULL, "/");

		if (next != NULL)
		{
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
			snprintf(index, INDEX_SIZE, sons+MAX_FILENAME_LEN+i);
			i+=MAX_FILENAME_LEN+INDEX_SIZE;
		}

		if (strcmp(name, token)!=0) // not found the son
			return -1;

		indexInode = atoi(index);
	}
	else
		indexInode = 0;

	indexBlock = (int)indexInode*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
	indexSector = (int)indexInode*sizeof(Inode)/SECTOR_SIZE+indexBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
	indexInode = (int)(indexInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

	if (Disk_Read(indexSector, sector->data)!=0)
		return -1;
	strcpy(block, sector->data);
	
	if (indexInode+sizeof(Inode)>SECTOR_SIZE)
	{
		if(Disk_Read(indexSector+1, sector->data)!=0)
			return -1;
		strcat(block, sector->data);
	}	

	// read the inode
	posCharStart=indexInode;
	inode->type = block[posCharStart]; // read the type of inode
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
	char* token;
	char* buff;
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

		if (Disk_Read(indexSector, sector->data)!=0)
			return -1;
		strcpy(block, sector->data);
	
		if (indexInode+sizeof(Inode)>SECTOR_SIZE)
		{
			if (Disk_Read(indexSector+1, sector->data)!=0)
				return -1;
			strcat(block, sector->data);
		}	

		// read the inode
		posCharStart=indexInode;
		dadInode->type = block[posCharStart]; // read the type of inode
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

		if (Disk_Read(indexSector, sector->data)!=0)
			return -1;
		strcpy(block, sector->data);
		
		if (indexInode+sizeof(Inode)>SECTOR_SIZE)
		{
			if (Disk_Read(indexSector+1, sector->data)!=0)
				return -1;
			strcat(block, sector->data);
		}	

		// read the inode
		posCharStart=indexInode;
		dadInode->type = block[posCharStart]; // read the type of inode
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

			if (Disk_Read(indexSector, sector->data)!=0)
				return -1;
			strcpy(block, sector->data);
		
			if (indexInode+sizeof(Inode)>SECTOR_SIZE)
			{
				if (Disk_Read(indexSector+1, sector->data)!=0)
					return -1;
				strcat(block, sector->data);
			}

			// read the son inode
			posCharStart=indexInode;
			sonInode->type = block[posCharStart]; // read the type of inode
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

		if (Disk_Read(indexSector, sector->data)!=0)
			return -1;
		strcpy(block, sector->data);
	
		if (indexInode+sizeof(Inode)>SECTOR_SIZE)
		{
			if (Disk_Read(indexSector+1, sector->data)!=0)
				return -1;
			strcat(block, sector->data);
		}

		// read the son inode
		posCharStart=(int)(indexInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table
		sonInode->type = block[posCharStart]; // read the type of inode
		posCharStart+=1; // add the char readden
		snprintf(sonInode->name, MAX_FILENAME_LEN, block+posCharStart); // read the name of file (directory)
		posCharStart+=MAX_FILENAME_LEN;
		snprintf(sonInode->size, MAX_FILE_SIZE_LEN, block+posCharStart); // read the size of file (directory)
		posCharStart+=MAX_FILE_SIZE_LEN;
		snprintf(sonInode->blocks, MAX_BLOCK_FILE*INDEX_SIZE, block+posCharStart); // read all block of inode

		for (noChar=0; noChar<MAX_FILENAME_LEN+INDEX_SIZE; noChar++) // write all bytes
			if(noChar<strlen(sonInode->name))
				son[noChar] = sonInode->name[noChar]; // write the name of the son
			else if(noChar>MAX_FILENAME_LEN)
				son[noChar] = buff[noChar-MAX_FILENAME_LEN]; // write the index inode
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
	char fatherData[MAX_BLOCK_FILE*(MAX_FILENAME_LEN+INDEX_SIZE)], sonIndex[5], fatherIndex[5];
	char* token, tmp, sonPos, sonName, block,fatherpath, grandfatherpath,fathername, posFatherInode;
	int indexSonInode, indexSonSector, indexSonBlock, i, indexFatherInode, indexFatherBlock, indexFatherSector, noChar, posIndextodelete, intIndex;
	Sector* inodeBitmap = (Sector*) calloc(1, sizeof(Sector));

	if (sizeof(path)>MAX_PATHNAME_LEN) // control the path length
		return -1;
	
	sonName = strtok(path, "/");	
	while(sonName!=NULL){				// search for the dir name					 
		tmp = sonName;
		strcat(fatherpath, "/");
		strcat(fatherpath, tmp);
		sonName = strtok(NULL,"/");
	}

	sonName = tmp;						//save the son name
	if (sonName==NULL) {
		osErrno = osErrno = E_ROOT_DIR;
		return -1;
	}

	fatherpath[strstr(fatherData, sonName)-1] = '\0';    ///find the father path


	if(Dir_Read(fatherpath,(void *)fatherData, MAX_BLOCK_FILE*(MAX_FILENAME_LEN+INDEX_SIZE))!=0)
		return -1;
	sonPos =strstr(fatherData, sonName);	// calculate the son's position in fatherdata to find index

	if(sonPos ==  NULL) {			// the directory not exist
		return -1;				
	}
	else {							// copy the son's inode index
		i=0;
		while(i<INDEX_SIZE){
			sonIndex[i] = fatherData[sonPos+strlen(sonName)+i];
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

	fathername = strtok(fatherpath, "/");
	while (fathername!=NULL){
		tmp = fathername;
		strcat(grandfatherpath, "/");
		strcat(grandfatherpath, fathername);
		fathername = strtok(NULL, "/");
	}
	fathername = tmp;

	if(Dir_Read(grandfatherpath, (void*) fatherData, MAX_BLOCK_FILE*(MAX_FILENAME_LEN+INDEX_SIZE))!=0)
		return -1;
	posFatherInode =strstr(fatherData, fathername);	// calculate the son's position in fatherdata to find index

	if(posFatherInode ==  NULL) {			// the directory not exist
		return -1;				
	}
	else {							// copy the son's inode index
		i=0;
		while(i<INDEX_SIZE){
			fatherIndex[i] = fatherData[posFatherInode+strlen(fathername)+i];
		}
	}
	// find father position
	indexFatherInode = atoi(fatherIndex);
	indexFatherBlock = (int)indexFatherInode*sizeof(Inode)/BLOCK_SIZE+3; // calculate the index of block
	indexFatherSector = (int)indexFatherInode*sizeof(Inode)/SECTOR_SIZE+indexFatherBlock*BLOCK_SIZE/SECTOR_SIZE; // index of sector into the inode table
	indexFatherInode = (int)(indexFatherInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table

	block = (char*) calloc( 1, sizeof(char)*BLOCK_SIZE);
	if(Disk_Read(indexFatherSector, sector->data)!=0)
			return -1;		//read son's inode info
	strcpy(block, sector->data);
	blockread=0;

		if (indexFatherInode+sizeof(Inode)>SECTOR_SIZE)   // if inode is splitted between two sectors
			{
				if(Disk_Read(indexFatherSector+1, sector->data))
					return -1;
				strcat(block, sector->data);
				blockread = 1;
			}

	posCharStart=(int)(indexFatherInode*sizeof(Inode))%SECTOR_SIZE; // recalculate the index of inode into the inode table
	snprintf(dadInode->type, 1, block+posCharStart); // read the type of inode
	posCharStart+=1; // add the char readden
	snprintf(dadInode->name, MAX_FILENAME_LEN, block+posCharStart); // read the name of file (directory)
	posCharStart+=MAX_FILENAME_LEN;
	snprintf(dadInode->size, MAX_FILE_SIZE_LEN, block+posCharStart);
	posCharStart+=MAX_FILE_SIZE_LEN;
	snprintf(sonInode->blocks, MAX_BLOCK_FILE*INDEX_SIZE, block+posCharStart);

	// delete inode of the son
	posIndextodelete = strstr(dadInode->blocks, sonIndex);
	for(i=0; i<INDEX_SIZE; i++)											// delete father "link" to the son
		dadInode->blocks[i+posIndextodelete] = dadInode->blocks[atoi(dadInode->size) + 1 - INDEX_SIZE + i];

	
	intIndex = atoi(dadInode->size)-INDEX_SIZE;
	snprintf(dadInode->size, MAX_FILE_SIZE_LEN, "%05d" ,intIndex) ;


	//TODO save updated father's inode

	/*char* inodeInfo = (char*) calloc(1, sizeof(Inode));
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
	*/
													//save the father updated

	if(blockread=0){								//if i read only a sector 
		snprintf(sector->data, SECTOR_SIZE, block);
		if(Disk_Write(indexFatherSector, sector->data)!=0)
			return -1;
	}
	else{
		snprintf(sector->data, SECTOR_SIZE, block);
		if(Disk_Write(indexFatherSector, sector->data)!=0)
			return -1;
		snprintf(sector->data, SECTOR_SIZE, block+SECTOR_SIZE);
		if(Disk_Write(indexFatherSector+1, sector->data)!=0)
			return -1;
	}

	///////////////////////////////////////////////////////////////////////////////////
	indexSonInode = atoi(sonIndex);
	if(indexSonInode/sizeof(Inode)<512){						//free the inode from the inode bitmap

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
