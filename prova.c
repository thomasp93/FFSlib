#include "ffslib.c"

int main(void)
{
	Disk_Create(); // creazione del disco
	FS_Init("pips.img"); // creazione disco
	if(Dir_Size("/opt")!=0) printf("Errore lettura cartella root");
	FS_Sync("pips.img"); // salvataggio disco
}
