#include "elf.h"

#include <stddef.h>
#include <string.h>

elf32_section_header *elf32_find_section(elf32_header *header, const char *name) {
    elf32_section_header *section_header = (elf32_section_header *)(((void *)header) + header->section_header_offset);
    elf32_section_header *section_name_header = (elf32_section_header *)(((void *)section_header) + (header->section_name_index) * header->section_header_size);

    for (int i = 1; i <= header->section_header_count; i++) {
        unsigned char *section_name = (unsigned char *) ((void *)header) + section_name_header->offset + section_header->name;

        if (strcmp(section_name, name) == 0) {
            return section_header;
        }

        section_header++;
    }

    return NULL;
}

elf64_section_header *elf64_find_section(elf64_header *header, const char *name) {
    elf64_section_header *section_header = (elf64_section_header *)(((void *)header) + header->section_header_offset);
    elf64_section_header *section_name_header = (elf64_section_header *)(((void *)section_header) + (header->section_name_index) * header->section_header_size);

    for (int i = 1; i <= header->section_header_count; i++) {
        unsigned char *section_name = (unsigned char *) ((void *)header) + section_name_header->offset + section_header->name;

        if (strcmp(section_name, name) == 0) {
            return section_header;
        }

        section_header++;
    }

    return NULL;
}
