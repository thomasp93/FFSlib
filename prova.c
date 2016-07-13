#include "ffslib.c"

int main(void)
{
/*
	Sector* s = (Sector*) calloc(1, sizeof(Sector));
	s->data[0]='0';
	s->data[1]='x';
	s->data[2]='1';
	s->data[3]='7';
	s->data[4]='1';
	s->data[5]='7';
	Disk_Create();
	Disk_Write(0, s->data);
	Disk_Save("pips.img"); // se non va sono cazzi suoi (di pips)
*/
	Disk_Create();
	FS_Init("pips.img");
	//printf("%d", diskErrno);

}
