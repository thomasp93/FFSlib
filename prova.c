#include "ffslib.c"

int main(void)
{
	Disk_Create(); // creazione del disco
	FS_Init("pips.img"); // creazione disco
	Dir_Create("/home"); // creazione cartella home
	FS_Sync("pips.img"); // salvataggio disco
}
