#include <stdio.h>

#include "LibFS.h"

void 
usage(char *prog) {
  fprintf(stderr, "usage: %s <disk image file>\n", prog);
  exit(1);
}

int main(int argc, char *argv[]) {

    char *path = argv[1];

    if (argc != 2) {
	    usage(argv[0]);
    }

    // initialize disk; if not exists creare it and format; if exist check signature
    ret = FS_Init(path); 

    // FS ops

    FS_Sync();

    return 0;
}

