#include "ffslib.c"

int main(void)
{
	Disk_Create(); // creazione del disco
	FS_Init("pips.img"); // creazione disco
	if(Dir_Create("/home")!=0) printf("Errore /home"); // creazione cartella home
	if(Dir_Create("/opt")!=0) printf("Errore /opt");
	FS_Sync("pips.img"); // salvataggio disco
}
