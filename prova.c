#include "ffslib.c"

int main(void)
{
	Disk_Create(); // creazione del disco
	FS_Init("pips.img"); // creazione disco
	if(Dir_Create("/home/pips")!=0) printf("Errore creazione opt");
	Dir_Create("/home/thomas/Scrivania");
	Dir_Create("/home/thomas/Documenti");
	Dir_Create("/home/thomas/Scaricati");
	if(Dir_Size("/home")!=0) printf("Errore lettura cartella root");
	Dir_Size("/");
	Dir_Size("/home/thomas");
	Dir_Size("/home/thomas/Documenti");
	Dir_Size("/bin");
	FS_Sync("pips.img"); // salvataggio disco
}
