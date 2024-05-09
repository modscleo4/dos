#ifndef ELF_H
#define ELF_H

#include <stdint.h>

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Off;
typedef int32_t Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

#pragma pack(push, 1)
typedef struct elf_header_ident {
    uint8_t magic[4];
    uint8_t class;
    uint8_t endian;
    uint8_t version;
    uint8_t osabi;
    uint8_t unused[8];
} elf_header_ident;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct elf32_header {
    elf_header_ident ident;
    Elf32_Half type;
    Elf32_Half machine;
    Elf32_Word version;
    Elf32_Addr entry;
    Elf32_Off program_header_offset;
    Elf32_Off section_header_offset;
    Elf32_Word flags;
    Elf32_Half header_size;
    Elf32_Half program_header_size;
    Elf32_Half program_header_count;
    Elf32_Half section_header_size;
    Elf32_Half section_header_count;
    Elf32_Half section_name_index;
} elf32_header;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct elf64_header {
    elf_header_ident ident;
    Elf64_Half type;
    Elf64_Half machine;
    Elf64_Word version;
    Elf64_Addr entry;
    Elf64_Off program_header_offset;
    Elf64_Off section_header_offset;
    Elf64_Word flags;
    Elf64_Half header_size;
    Elf64_Half program_header_size;
    Elf64_Half program_header_count;
    Elf64_Half section_header_size;
    Elf64_Half section_header_count;
    Elf64_Half section_name_index;
} elf64_header;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct elf32_program_header {
    Elf32_Word type;
    Elf32_Off offset;
    Elf32_Addr virtual_address;
    Elf32_Addr physical_address;
    Elf32_Word file_size;
    Elf32_Word memory_size;
    Elf32_Word flags;
    Elf32_Word alignment;
} elf32_program_header;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct elf64_program_header {
    Elf64_Word type;
    Elf64_Word flags;
    Elf64_Off offset;
    Elf64_Addr virtual_address;
    Elf64_Addr physical_address;
    Elf64_Xword file_size;
    Elf64_Xword memory_size;
    Elf64_Xword alignment;
} elf64_program_header;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct elf32_section_header {
    Elf32_Word name;
    Elf32_Word type;
    Elf32_Word flags;
    Elf32_Addr address;
    Elf32_Off offset;
    Elf32_Word size;
    Elf32_Word link;
    Elf32_Word info;
    Elf32_Word address_align;
    Elf32_Word entry_size;
} elf32_section_header;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct elf64_section_header {
    Elf64_Word name;
    Elf64_Word type;
    Elf64_Xword flags;
    Elf64_Addr address;
    Elf64_Off offset;
    Elf64_Xword size;
    Elf64_Word link;
    Elf64_Word info;
    Elf64_Xword address_align;
    Elf64_Xword entry_size;
} elf64_section_header;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct elf32_symbol_table_entry {
    Elf32_Word name;
    Elf32_Addr value;
    Elf32_Word size;
    uint8_t info;
    uint8_t other;
    Elf32_Half section_index;
} elf32_symbol_table_entry;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct elf64_symbol_table_entry {
    Elf64_Word name;
    uint8_t info;
    uint8_t other;
    Elf32_Half section_index;
    Elf32_Addr value;
    Elf64_Xword size;
} elf64_symbol_table_entry;
#pragma pack(pop)

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

enum ELFProgramHeaderType {
    ELF_PT_NULL = 0,
    ELF_PT_LOAD = 1,
    ELF_PT_DYNAMIC = 2,
    ELF_PT_INTERP = 3,
    ELF_PT_NOTE = 4,
    ELF_PT_SHLIB = 5,
    ELF_PT_PHDR = 6,
    ELF_PT_TLS = 7,
    ELF_PT_LOOS = 0x60000000,
    ELF_PT_HIOS = 0x6FFFFFFF,
    ELF_PT_LOPROC = 0x70000000,
    ELF_PT_HIPROC = 0x7FFFFFFF
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
