#include <stdio.h>
#include "assembler.h"

int main(int argc, char *argv[]) {

    if(argc < 2)
    {
        printf("input file not specified.");
        return -1;
    }

    for (int i = 1; i < argc; i++) {
        printf("Compiling %s\n", argv[i]);
        compile(argv[i]);
    }

    return 0;
}
