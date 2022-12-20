#ifndef ELF_H
#define ELF_H

typedef struct elf_header {
    unsigned char magic[4];
    unsigned char class;
    unsigned char endian;
    unsigned char version;
    unsigned char osabi;
    unsigned char unused[8];
} __attribute__((packed)) elf_header;

typedef struct elf32_header {
    elf_header header;
    unsigned short int type;
    unsigned short int machine;
    unsigned int version;
    unsigned int entry;
    unsigned int program_header_offset;
    unsigned int section_header_offset;
    unsigned int flags;
    unsigned short int header_size;
    unsigned short int program_header_size;
    unsigned short int program_header_count;
    unsigned short int section_header_size;
    unsigned short int section_header_count;
    unsigned short int section_name_index;
} __attribute__((packed)) elf32_header;

typedef struct elf64_header {
    elf_header header;
    unsigned short int type;
    unsigned short int machine;
    unsigned int version;
    unsigned long int entry;
    unsigned long int program_header_offset;
    unsigned long int section_header_offset;
    unsigned int flags;
    unsigned short int header_size;
    unsigned short int program_header_size;
    unsigned short int program_header_count;
    unsigned short int section_header_size;
    unsigned short int section_header_count;
    unsigned short int section_name_index;
} __attribute__((packed)) elf64_header;

typedef struct elf32_program_header {
    unsigned int type;
    unsigned int offset;
    unsigned int virtual_address;
    unsigned int physical_address;
    unsigned int file_size;
    unsigned int memory_size;
    unsigned int flags;
    unsigned int alignment;
} __attribute__((packed)) elf32_program_header;

typedef struct elf64_program_header {
    unsigned int type;
    unsigned int flags;
    unsigned long int offset;
    unsigned long int virtual_address;
    unsigned long int physical_address;
    unsigned long int file_size;
    unsigned long int memory_size;
    unsigned long int alignment;
} __attribute__((packed)) elf64_program_header;

typedef struct elf32_section_header {
    unsigned long int name;
    unsigned long int type;
    unsigned long int flags;
    unsigned long int address;
    unsigned long int offset;
    unsigned long int size;
    unsigned long int link;
    unsigned long int info;
    unsigned long int address_align;
    unsigned long int entry_size;
} __attribute__((packed)) elf32_section_header;

typedef struct elf64_section_header {
    unsigned long int name;
    unsigned long int type;
    unsigned long int flags;
    unsigned long long int address;
    unsigned long long int offset;
    unsigned long long int size;
    unsigned long int link;
    unsigned long int info;
    unsigned long long int address_align;
    unsigned long long int entry_size;
} __attribute__((packed)) elf64_section_header;

typedef struct elf32_symbol_table_entry {
    unsigned long int name;
    unsigned long int value;
    unsigned long int size;
    unsigned char info;
    unsigned char other;
    unsigned short int section_index;
} __attribute__((packed)) elf32_symbol_table_entry;

#define ELF32_ST_BIND(i) ((i) >> 4)
#define ELF32_ST_TYPE(i) ((i) & 0xf)
#define ELF32_ST_INFO(b, t) (((b) << 4) + ((t) & 0xf))

static const char elf_magic[] = {0x7f, 'E', 'L', 'F'};

enum ELFArch {
    ELF_ARCH_X86 = 1,
    ELF_ARCH_X86_64 = 2
};

enum ELFEndian {
    ELF_ENDIAN_LITTLE = 1,
    ELF_ENDIAN_BIG = 2
};

enum ELFType {
    ELF_TYPE_RELOCATABLE = 1,
    ELF_TYPE_EXECUTABLE = 2,
    ELF_TYPE_SHARED = 3,
    ELF_TYPE_CORE = 4
};

enum ELFInstructionSet {
    ELF_INSTRSET_NONE = 0,
    ELF_INSTRSET_SPARC = 0x2,
    ELF_INSTRSET_X86 = 0x3,
    ELF_INSTRSET_MIPS = 0x8,
    ELF_INSTRSET_POWERPC = 0x14,
    ELF_INSTRSET_ARM = 0x28,
    ELF_INSTRSET_SUPERH = 0x2A,
    ELF_INSTRSET_IA64 = 0x32,
    ELF_INSTRSET_X86_64 = 0x3E,
    ELF_INSTRSET_AARCH64 = 0xB7,
    ELF_INSTRSET_RISCV = 0xF3
};

enum ELFSymbolTableType {
    ELF_SYMBOLTABLE_TYPE_NOTYPE = 0,
    ELF_SYMBOLTABLE_TYPE_OBJECT = 1,
    ELF_SYMBOLTABLE_TYPE_FUNC = 2,
    ELF_SYMBOLTABLE_TYPE_SECTION = 3,
    ELF_SYMBOLTABLE_TYPE_FILE = 4,
    ELF_SYMBOLTABLE_TYPE_LOPROC = 13,
    ELF_SYMBOLTABLE_TYPE_HIPROC = 15
};

elf32_section_header *elf32_find_section(elf32_header *header, const char *name);

elf64_section_header *elf64_find_section(elf64_header *header, const char *name);

#endif // ELF_H
