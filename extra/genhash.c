// This was used to make the hash values for the color palette names that 
// are in the ugly switch statement in ipv4-heatmap.c
// I'll prbly move this to colorgen.r at some point 

#include <stdio.h>

const unsigned long hash(const char *str) {

    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;

}

int main() {

   printf("#define hash_%s %lu\n", "brbg",  hash("brbg"));
   printf("#define hash_%s %lu\n", "piyg",  hash("piyg"));
   printf("#define hash_%s %lu\n", "prgn",  hash("prgn"));
   printf("#define hash_%s %lu\n", "puor",  hash("puor"));
   printf("#define hash_%s %lu\n", "rdbu",  hash("rdbu"));
   printf("#define hash_%s %lu\n", "rdgy",  hash("rdgy"));
   printf("#define hash_%s %lu\n", "rdylbu",  hash("rdylbu"));
   printf("#define hash_%s %lu\n", "rdylgn",  hash("rdylgn"));
   printf("#define hash_%s %lu\n", "spectral",  hash("spectral"));
   printf("#define hash_%s %lu\n", "bupu",  hash("bupu"));
   printf("#define hash_%s %lu\n", "reds",  hash("reds"));
   printf("#define hash_%s %lu\n", "ylgnbu",  hash("ylgnbu"));
   printf("#define hash_%s %lu\n", "ylorbr",  hash("ylorbr"));
   printf("#define hash_%s %lu\n", "ylorrd",  hash("ylorrd"));
   printf("#define hash_%s %lu\n", "viridis",  hash("viridis"));

}