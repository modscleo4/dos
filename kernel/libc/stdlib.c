#include <stdlib.h>

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../bits.h"
#include "../debug.h"
#include "../kernel.h"
#include "../ring3.h"
#include "../rootfs.h"
#include "../cpu/cpuid.h"
#include "../cpu/gdt.h"
#include "../cpu/panic.h"
#include "../cpu/mmu.h"
#include "../modules/bitmap.h"
#include "../modules/elf.h"
#include "../modules/heap.h"
#include "../modules/process.h"
#include "../modules/spinlock.h"

static int min(int a, int b) {
    return a < b ? a : b;
}

static int max(int a, int b) {
    return a > b ? a : b;
}

static const char numbase[] = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";

float atof(const char *str) {
    return (float)strtod(str, NULL);
}

int atoi(const char *str) {
    return (int)atol(str);
}

char *htoa(short int value, char *str, int base) {
    return ltoa((long int)value, str, base);
}

char *itoa(int value, char *str, int base) {
    return ltoa((long int)value, str, base);
}

char *ltoa(long int value, char *str, int base) {
    char *rc;
    char *ptr;
    char *low;

    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    rc = ptr = str;

    if (value < 0) {
        if (base == 10) {
            *ptr++ = '-';
        } else {
            value = -value;
        }
    }

    low = ptr;

    do {
        *ptr++ = numbase[35 + value % base];
        value /= base;
    } while (value);

    *ptr-- = '\0';

    while (low < ptr) {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }

    return rc;
}

char *hutoa(unsigned short int value, char *str, int base) {
    return lutoa((unsigned long int)value, str, base);
}

char *utoa(unsigned int value, char *str, int base) {
    return lutoa((unsigned long int)value, str, base);
}

char *lutoa(unsigned long int value, char *str, int base) {
    char *rc;
    char *ptr;
    char *low;

    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    rc = ptr = str;

    low = ptr;

    do {
        *ptr++ = numbase[35 + value % base];
        value /= base;
    } while (value);

    *ptr-- = '\0';

    while (low < ptr) {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }

    return rc;
}

char *ftoa(float value, char *str, int precision) {
    if (isnanf(value)) {
        strcpy(str, "nan");
        return str;
    } else if (isinff(value)) {
        if (value < 0) {
            strcpy(str, "-inf");
        } else {
            strcpy(str, "inf");
        }

        return str;
    }

    return lftoa((double)value, str, precision);
}

char *lftoa(double value, char *str, int precision) {
    if (isnanl(value)) {
        strcpy(str, "nan");
        return str;
    } else if (isinfl(value)) {
        if (value < 0) {
            strcpy(str, "-inf");
        } else {
            strcpy(str, "inf");
        }

        return str;
    }

    ltoa((long int)value, str, 10);
    int i = strlen(str);
    if (precision > 0) {
        str[i] = '.';

        value -= (long int)value;
        value *= powl(10, precision);
        ltoa((long int)value, str + i + 1, 10);

        int j = strlen(str + i + 1);
        while (j < precision) {
            str[i + j + 1] = '0';
            str[i + j + 2] = 0;
            j++;
        }
    }

    return str;
}

long int atol(const char *str) {
    return strtol(str, NULL, 10);
}

double strtod(const char *str, char **endptr) {
    return 0.0F;
}

long int strtol(const char *str, char **endptr, int base) {
    if (base < 0 || base == 1 || base > 36) {
        if (endptr) {
            *endptr = (char *)str;
        }

        return 0;
    }

    while (isspace(*str)) {
        str++;
    }

    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    if (base == 0) {
        if (*str == '0') {
            if (str[1] == 'x' || str[1] == 'X') {
                base = 16;
                str += 2;
            } else {
                base = 8;
                str++;
            }
        } else {
            base = 10;
        }
    } else if (base == 16) {
        if (*str == '0' && (str[1] == 'x' || str[1] == 'X')) {
            str += 2;
        }
    }

    if (!*str) {
        return 0;
    }

    long int value = 0;
    while (*str) {
        int digit = 0;
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'a' && *str <= 'z') {
            digit = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'Z') {
            digit = *str - 'A' + 10;
        } else {
            return 0;
        }

        if (digit >= base) {
            return 0;
        }

        value = value * base + digit;
        str++;
    }

    if (endptr) {
        *endptr = (char *)str;
    }

    return value * sign;
}

unsigned long int strtoul(const char *str, char **endptr, int base) {
    return 0;
}

static heap kernel_heap;
static spinlock *malloc_lock = NULL;

void kernel_malloc_init(void) {
    if (!malloc_lock) {
        malloc_lock = spinlock_init();
    }

    void *addr = mmu_alloc_pages(256); // 1 MB

    heap_init(&kernel_heap);
    heap_add_block(&kernel_heap, addr, 256 * BITMAP_PAGE_SIZE, 16);

    dbgprint("kernel_heap: &%x\n", &kernel_heap);
    dbgprint("kernel_heap.first_block: &%x\n", kernel_heap.first_block);
    dbgprint("kernel_heap.first_block->size: %d\n", kernel_heap.first_block->size);
    dbgprint("kernel_heap.first_block->next: %d\n", kernel_heap.first_block->next);
    dbgprint("kernel_heap.first_block->used: %d\n", kernel_heap.first_block->used);
}

void *calloc(size_t num, size_t size) {
    void *addr = malloc(num * size);
    if (addr) {
        memset(addr, 0, num * size);
    }

    return addr;
}

void *calloc_align(size_t num, size_t size, size_t align) {
    void *addr = calloc(1, num * size + align);
    if (addr) {
        addr = (void *)(uintptr_t)addr + (align - (uintptr_t)addr % align);
    }

    return addr;
}

void free(void *ptr) {
    if (!ptr) {
        return;
    }

    spinlock_lock(malloc_lock);

    dbgprint("free(&%x)\n", ptr);

    if (!heap_free(&kernel_heap, ptr)) {
        spinlock_unlock(malloc_lock);
        dbgprint("free: could not free &%x\n", ptr);
        return;
    }

    spinlock_unlock(malloc_lock);
}

void *malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    spinlock_lock(malloc_lock);

    if (size < _MIN_MALLOC_SIZE) {
        size = _MIN_MALLOC_SIZE;
    }

    void *addr = heap_alloc(&kernel_heap, size);
    if (!addr) {
        heap_add_block(&kernel_heap, mmu_alloc_pages(256), 256 * BITMAP_PAGE_SIZE, 16);
        addr = heap_alloc(&kernel_heap, size);
    }

    spinlock_unlock(malloc_lock);

    if (!addr) {
        dbgprint("malloc: could not allocate %d bytes\n", size);
        return NULL;
    }

    dbgprint("malloc(%d) -> &%x\n", size, addr);

    return addr;
}

void *malloc_align(size_t size, size_t align) {
    void *addr = malloc(size + align);
    if (addr && (uintptr_t)addr % align != 0) {
        addr = (void *)(uintptr_t)addr + (align - (uintptr_t)addr % align);
    }

    return addr;
}

void *realloc(void *ptr, size_t size) {
    if (size == 0) {
        return NULL;
    }

    void *new_addr = malloc(size);
    if (!new_addr) {
        return NULL;
    }

    memcpy(new_addr, ptr, min(size, heap_allocated_size(&kernel_heap, ptr)));
    free(ptr);

    return new_addr;
}

void abort(void) {
    //
}

int atexit(void (*func)(void)) {
    return 0;
}

void exit(int status) {
    //
}

char *getenv(const char *name) {
    return NULL;
}

int system(const char *command) {
    struct stat st;
    int stat_ret = 0;
    if ((stat_ret = rootfs.stat(&rootfs_io, &rootfs, command, &st))) {
        return stat_ret;
    }

    size_t elf_file_size = st.st_size;
    dbgprint("File size: %d bytes\n", elf_file_size);

    void *addr;
    if (!(addr = rootfs.load_file(&rootfs_io, &rootfs, &st))) {
        dbgprint("Could not allocate or load file.\n");
        return -2;
    }

    dbgprint("%s loaded at address 0x%x\n", command, addr);

    elf_header_ident *header = (elf_header_ident *) addr;
    if (memcmp(header->magic, elf_magic, 4)) {
        dbgprint("Not an ELF file\n");
        return -3;
    }

    if (header->version == ELF_ARCH_X86) {
        elf32_header *exec_header = (elf32_header *) addr;

        dbgprint("x86 ELF file (%d bytes)\n", elf_file_size);
        dbgprint("Entry point: %x\n", exec_header->entry);

        page_directory_table *process_page_directory = mmu_new_page_directory();
        void *process_stack = bitmap_alloc_page();

        elf32_program_header *program_header = (elf32_program_header *)(((void *)exec_header) + exec_header->program_header_offset);
        for (int i = 1; i <= exec_header->program_header_count; i++) {
            dbgprint("Program header %d: type=%d, offset=%x, virtual_address=%x, physical_address=%x, file_size=%d, memory_size=%d\n", i, program_header->type, program_header->offset, program_header->virtual_address, program_header->physical_address, program_header->file_size, program_header->memory_size);
            if (program_header->type == ELF_PT_LOAD) { // map to virtual address
                mmu_map_pages(process_page_directory, mmu_get_physical_address((uintptr_t)addr + program_header->offset), program_header->virtual_address, program_header->memory_size / BITMAP_PAGE_SIZE + 1, true, true, true);
            }

            program_header++;
        }

        // Map the ELF stack to 8 kiB below the kernel
        mmu_map_pages(process_page_directory, (uintptr_t)process_stack, (uintptr_t)0xc0000000 - 0x1000, 1, true, true, true);
        mmu_map_pages(current_pdt, (uintptr_t)process_stack, (uintptr_t)0xc0000000 - 0x1000, 1, true, true, true);

        void *a = bitmap_alloc_page();
        mmu_map_pages(process_page_directory, (uintptr_t)a, (uintptr_t)0xb00000, 1, true, true, true);

        // Map the kernel to 0xc0000000 (higher half)
        mmu_copy_kernel_pages(process_page_directory);

        process_create((uintptr_t)exec_header->entry, (uintptr_t)(0xc0000000 - 0x4), process_page_directory, false);

        dbgprint("Switching to ring 3...\n");

        switch_ring3((page_directory_table *)mmu_get_physical_address((uintptr_t)process_page_directory), (void *)(exec_header->entry), (uintptr_t)(0xc0000000 - 0x4));
    } else if (header->version == ELF_ARCH_X86_64) {
        elf64_header *exec_header = (elf64_header *) addr;

        dbgprint("x86_64 ELF file\n");
        dbgprint("Entry point: %x\n", exec_header->entry);

        page_directory_table *process_page_directory = mmu_new_page_directory();
        void *process_stack = bitmap_alloc_page();

        elf64_program_header *program_header = (elf64_program_header *)(((void *)exec_header) + exec_header->program_header_offset);
        for (int i = 1; i <= exec_header->program_header_count; i++) {
            if (program_header->type == ELF_PT_LOAD) { // map to virtual address
                mmu_map_pages(process_page_directory, mmu_get_physical_address((uintptr_t)addr + program_header->offset), program_header->virtual_address, program_header->memory_size / BITMAP_PAGE_SIZE + 1, true, true, true);
            }

            program_header++;
        }

        // Map the ELF stack to 8 kiB below the kernel
        mmu_map_pages(process_page_directory, (uintptr_t)process_stack, (uintptr_t)0xc0000000 - 0x1000, 1, true, true, true);
        mmu_map_pages(current_pdt, (uintptr_t)process_stack, (uintptr_t)0xc0000000 - 0x1000, 1, true, true, true);

        // Map the kernel to 0xc0000000 (higher half)
        mmu_copy_kernel_pages(process_page_directory);

        process_create((uintptr_t)exec_header->entry, (uintptr_t)(0xc0000000 - 0x8), process_page_directory, false);

        dbgprint("Switching to ring 3...\n");

        switch_ring3((page_directory_table *)mmu_get_physical_address((uintptr_t)process_page_directory), (void *)(exec_header->entry), (uintptr_t)(0xc0000000 - 0x8));
    } else {
        dbgprint("Unsupported architecture: %x\n", header->version);
        return -4;
    }

    return 0;
}

void swap(void *a, void *b, size_t size) {
    char *tmp = malloc(size);
    memcpy(tmp, a, size);

    memcpy(a, b, size);
    memcpy(b, tmp, size);

    free(tmp);
}

/**
 * Binary Search implementation
 *
 * Assume that the array is sorted in ascending order.
 * 1. Compare x with the middle element.
 * 2. If x matches with middle element, we return the mid index.
 * 3. Else if x is greater than the mid element, then x can only lie in right half subarray after the mid element.
 *  So we recur for right half.
 * 4. Else (x is smaller) recur for the left half.
 * Go to step 1 while start <= end.
 * 5. We reach here if element was not present in array.
 */
void *bsearch(const void *key, const void *base, size_t num, size_t size, int (*compar)(const void *, const void *)) {
    size_t start = 0;
    size_t end = num;

    while (start < end) {
        size_t mid = (start + end) / 2;
        int cmp = compar(key, base + mid * size);
        if (cmp == 0) {
            return (void *)base + mid * size;
        } else if (cmp < 0) {
            end = mid;
        } else {
            start = mid + 1;
        }
    }

    return NULL;
}

/**
 * Quick Sort implementation in-place
 *
 * 1. Choose a pivot element from the list. We can choose the first element as the pivot element for simplicity.
 * 2. Reorder the list so that all elements with values less than the pivot element come before the pivot element,
 *   while all elements with values greater than the pivot element come after it (equal values can go either way).
 *  After this partitioning, the pivot element is in its final position. This is called the partition operation.
 * 3. Recursively apply the above steps to the sub-list of elements with smaller values and separately the sub-list
 *  of elements with greater values.
 *
 * Example:
 * | 5 | 3 | 7 | 6 | 2 | 9 | 1 | 4 | 8 |
 *   ^
 *  pivot
 *
 * After partitioning:
 * | 3 | 2 | 1 | 4 | 5 | 7 | 6 | 9 | 8 |
 */
static void qsort_inplace(void *base, size_t num, size_t size, int (*compar)(const void *, const void *), size_t start, size_t end) {
    if (end - start <= 1) {
        return;
    }

    // Partition
    // Move all elements smaller than the pivot to the left, and all greater than the pivot to the right
    size_t pivot_index = end - 1;

    size_t i = start - 1;
    for (size_t j = start; j < end - 1; j++) {
        int cmp = compar(base + j * size, base + pivot_index * size);
        if (cmp < 0) { // Smaller
            i++;
            swap(base + i * size, base + j * size, size);
        }
    }
    swap(base + (i + 1) * size, base + pivot_index * size, size);
    pivot_index = i + 1;

    qsort_inplace(base, num, size, compar, start, pivot_index);
    qsort_inplace(base, num, size, compar, pivot_index + 1, end);
}

void qsort(void *base, size_t num, size_t size, int (*compar)(const void *, const void *)) {
    qsort_inplace(base, num, size, compar, 0, num);
}

div_t div(int numer, int denom) {
    div_t result;
    result.quot = numer / denom;
    result.rem = numer % denom;

    if (numer >= 0 && result.rem < 0) {
        result.quot++;
        result.rem -= denom;
    }

    return result;
}

ldiv_t ldiv(long int numer, long int denom) {
    ldiv_t result;
    result.quot = numer / denom;
    result.rem = numer % denom;

    if (numer >= 0 && result.rem < 0) {
        result.quot++;
        result.rem -= denom;
    }

    return result;
}

int abs(int x) {
    return (int)labs((long int)x);
}

long int labs(long int x) {
    return x < 0 ? -x : x;
}

static uint32_t rand_next = 1;

int rand(void) {
    if (ISSET_BIT_INT(cpuinfo.ecx, CPUID_FEAT_ECX_RDRAND)) {
        bool success = false;
        asm volatile(
            "rdrand %0;"
            "setc %1;"
            : "=r"(rand_next), "=r"(success)
        );

        if (success) {
            return rand_next;
        }
    }

    rand_next = rand_next * 1103515245 + 12345;
    return (rand_next / 65536) % 32768;
}

void srand(unsigned int seed) {
    rand_next = seed;
}

int mblen(const char *pmb, size_t max) {
    return 0;
}

size_t mbstowcs(wchar_t *dest, const char *src, size_t max) {
    return 0;
}

int mbtowc(wchar_t *pwc, const char *pmb, size_t max) {
    return 0;
}

size_t wcstombs(char *dest, const wchar_t *src, size_t max) {
    return 0;
}

int wctomb(char *pmb, wchar_t wc) {
    return 0;
}
