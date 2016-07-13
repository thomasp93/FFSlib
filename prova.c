#include "ffslib.c"

int main(void)
{

	 // Sector* s = (Sector*) calloc(1, sizeof(Sector));
	 // OpenFileTable* openFiles = (OpenFileTable*) calloc(1,sizeof(OpenFileTable));

	 // strcpy(s->data, "0x1717");
	 // Disk_Create();
	 // Disk_Write(0, s->data);
	 // FS_Sync("pips.img"); // se non va sono cazzi suoi (di pips)

	Disk_Create();
	FS_Init("pips.img");
	printf("%d", diskErrno);

}
