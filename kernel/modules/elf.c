#include "elf.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stddef.h>
#include <string.h>
#include "../debug.h"

elf32_section_header *elf32_find_section(elf32_header *header, const char *name) {
    dbgprint("section_header_offset = %d\n", header->section_header_offset);
    dbgprint("section_name_index = %d\n", header->section_name_index);
    dbgprint("section_header_size = %d\n", header->section_header_size);
    dbgprint("section_header_count = %d\n", header->section_header_count);

    elf32_section_header *section_header = (elf32_section_header *)(((void *)header) + header->section_header_offset);
    elf32_section_header *section_name_header = (elf32_section_header *)(((void *)section_header) + (header->section_name_index) * header->section_header_size);
    dbgprint("section_header_addr = %p\n", section_header);
    dbgprint("section_name_header_addr = %p\n", section_name_header);
    dbgprint("section_name_header_offset = %p\n", section_name_header->offset);

    for (int i = 1; i <= header->section_header_count; i++) {
        char *section_name = (char *) ((void *)header) + section_name_header->offset + section_header->name;
        dbgprint("section_name_addr = %p\n", section_name);

        if (strcmp(section_name, name) == 0) {
            dbgprint("Found section %s at %p\n", name, section_header);
            return section_header;
        }

        section_header++;
    }

    return NULL;
}

elf64_section_header *elf64_find_section(elf64_header *header, const char *name) {
    elf64_section_header *section_header = (elf64_section_header *)(((void *)header) + header->section_header_offset);
    elf64_section_header *section_name_header = (elf64_section_header *)(((void *)section_header) + (header->section_name_index) * header->section_header_size);
    dbgprint("section_header_addr = %p\n", section_header);
    dbgprint("section_name_header_addr = %p\n", section_name_header);

    for (int i = 1; i <= header->section_header_count; i++) {
        char *section_name = (char *) ((void *)header) + section_name_header->offset + section_header->name;

        if (strcmp(section_name, name) == 0) {
            dbgprint("Found section %s at %p\n", name, section_header);
            return section_header;
        }

        section_header++;
    }

    return NULL;
}
