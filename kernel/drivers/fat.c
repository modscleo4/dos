#include "fat.h"

int fat_write() {

}

int fat_read() {
    /*unsigned char FAT_table[sector_size];
    unsigned int fat_offset = active_cluster + (active_cluster / 2);// multiply by 1.5
    unsigned int fat_sector = first_fat_sector + (fat_offset / section_size);
    unsigned int ent_offset = fat_offset % section_size;

    //at this point you need to read from sector "fat_sector" on the disk into "FAT_table".

    unsigned short table_value = *(unsigned short *) &FAT_table[ent_offset];

    if (active_cluster & 0x0001) {
        table_value = table_value >> 4;
    } else {
        table_value = table_value & 0x0FFF;
    }
    //the variable "table_value" now has the information you need about the next cluster in the chain.*/
}
