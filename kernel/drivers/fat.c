#include "fat.h"
#include <string.h>

void buffer2fatentry(unsigned char *buffer, fat_entry *f) {
    memcpy(f, buffer, sizeof(fat_entry));
}

unsigned short int fat_next_cluster(unsigned int cluster, const unsigned char *buffer, unsigned int ent_offset) {
    unsigned short int table_value = *(unsigned short int *)&buffer[ent_offset];

    if (cluster & 0x0001) {
        table_value = table_value >> 4;
    } else {
        table_value = table_value & 0x0FFF;
    }

    return table_value;
}

int fat_writefile() {
}

int fat_readfile() {
}
