#ifndef PCI_H
#define PCI_H

#include <stdbool.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

typedef struct pci_header_type {
    unsigned char type: 7;
    bool multi_func: 1;
} __attribute__((packed)) pci_header_type;

typedef struct pci_bist_reg {
    unsigned char completion_code: 4;
    unsigned char reserved: 2;
    bool start_bist: 1;
    bool bist_capable: 1;
} __attribute__((packed)) pci_bist_reg;

typedef struct pci_command_reg {
    bool io_space: 1;
    bool memory_space: 1;
    bool bus_master: 1;
    bool special_cycles: 1;
    bool memory_write_and_invalidate: 1;
    bool vga_palette_snoop: 1;
    bool parity_error_response: 1;
    bool reserved: 1;
    bool serr_enable: 1;
    bool fast_back_to_back_enable: 1;
    bool interrupt_disable: 1;
    unsigned char reserved2: 5;
} __attribute__((packed)) pci_command_reg;

typedef struct pci_status_reg {
    unsigned char reserved: 3;
    bool interrupt_status: 1;
    bool capabilities_list: 1;
    bool capable_66mhz: 1;
    bool reserved2: 1;
    bool fast_back_to_back: 1;
    bool master_data_parity_error: 1;
    unsigned char devsel_timing: 2;
    bool signaled_target_abort: 1;
    bool received_target_abort: 1;
    bool received_master_abort: 1;
    bool signaled_system_error: 1;
    bool detected_parity_error: 1;
} __attribute__((packed)) pci_status_reg;

typedef struct pci_header {
    unsigned short int vendor;
    unsigned short int device;
    pci_command_reg command;
    pci_status_reg status;
    unsigned char revision;
    unsigned char prog_if;
    unsigned char subclass;
    unsigned char class;
    unsigned char cache_line_size;
    unsigned char latency_timer;
    pci_header_type header_type;
    pci_bist_reg bist;
} __attribute__((packed)) pci_header;

typedef struct pci_device {
    pci_header header;
    unsigned int base_address[6];
    unsigned int cardbus_cis;
    unsigned short int subsystem_vendor;
    unsigned short int subsystem_device;
    unsigned int rom_base_address;
    unsigned char capabilities_pointer;
    unsigned char reserved[3];
    unsigned int reserved2;
    unsigned char interrupt_line;
    unsigned char interrupt_pin;
    unsigned char min_grant;
    unsigned char max_latency;
} __attribute__((packed)) pci_device;

typedef struct pci_pci_bridge {
    pci_header header;
    unsigned int base_address[2];
    unsigned char primary_bus;
    unsigned char secondary_bus;
    unsigned char subordinate_bus;
    unsigned char secondary_latency_timer;
    unsigned char io_base;
    unsigned char io_limit;
    unsigned short int secondary_status;
    unsigned short int memory_base;
    unsigned short int memory_limit;
    unsigned short int prefetch_memory_base;
    unsigned short int prefetch_memory_limit;
    unsigned int prefetch_base_upper32;
    unsigned int prefetch_limit_upper32;
    unsigned short int io_base_upper16;
    unsigned short int io_limit_upper16;
    unsigned char capabilities_pointer;
    unsigned char reserved[3];
    unsigned int expansion_rom_base_address;
    unsigned char interrupt_line;
    unsigned char interrupt_pin;
    unsigned short int bridge_control;
} __attribute__((packed)) pci_pci_bridge;

typedef struct pci_cardbus_bridge {
    pci_header header;
    unsigned int cardbus_socket_exca_base_address;
    unsigned char offset_capabilities_pointer;
    unsigned char reserved;
    unsigned short int secondary_status;
    unsigned char pci_bus_number;
    unsigned char cardbus_bus_number;
    unsigned char subordinate_bus;
    unsigned char cardbus_latency_timer;
    unsigned int memory_base_0;
    unsigned int memory_limit_0;
    unsigned int memory_base_1;
    unsigned int memory_limit_1;
    unsigned int io_base_0;
    unsigned int io_limit_0;
    unsigned int io_base_1;
    unsigned int io_limit_1;
    unsigned char interrupt_line;
    unsigned char interrupt_pin;
    unsigned short int bridge_control;
    unsigned short int subsystem_device_id;
    unsigned short int subsystem_vendor_id;
    unsigned int legacy_base_address;
} __attribute__((packed)) pci_cardbus_bridge;

void pci_init(void);

unsigned short int pci_read_word(unsigned int, unsigned int, unsigned int, unsigned short int);

void pci_write_word(unsigned int, unsigned int, unsigned int, unsigned short int, unsigned int);

void pci_read_header(unsigned int, unsigned int, unsigned int, pci_header *);

void pci_read_device(unsigned int, unsigned int, unsigned int, pci_header *, pci_device *);

void pci_read_pci_bridge(unsigned int, unsigned int, unsigned int, pci_header *, pci_pci_bridge *);

void pci_read_cardbus_bridge(unsigned int, unsigned int, unsigned int, pci_header *, pci_cardbus_bridge *);

void pci_check_device(unsigned char, unsigned char);

void pci_check_bus(unsigned char);

void pci_check_function(unsigned char, unsigned char, unsigned char, pci_header *);

void pci_discover_devices(void);

#endif // PCI_H
