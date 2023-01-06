#ifndef PCI_H
#define PCI_H

#include <stdbool.h>
#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

typedef struct pci_header_type {
    uint8_t type: 7;
    bool multi_func: 1;
} __attribute__((packed)) pci_header_type;

typedef struct pci_bist_reg {
    uint8_t completion_code: 4;
    uint8_t reserved: 2;
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
    uint8_t reserved2: 5;
} __attribute__((packed)) pci_command_reg;

typedef struct pci_status_reg {
    uint8_t reserved: 3;
    bool interrupt_status: 1;
    bool capabilities_list: 1;
    bool capable_66mhz: 1;
    bool reserved2: 1;
    bool fast_back_to_back: 1;
    bool master_data_parity_error: 1;
    uint8_t devsel_timing: 2;
    bool signaled_target_abort: 1;
    bool received_target_abort: 1;
    bool received_master_abort: 1;
    bool signaled_system_error: 1;
    bool detected_parity_error: 1;
} __attribute__((packed)) pci_status_reg;

typedef struct pci_header {
    uint16_t vendor;
    uint16_t device;
    pci_command_reg command;
    pci_status_reg status;
    uint8_t revision;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    pci_header_type header_type;
    pci_bist_reg bist;
} __attribute__((packed)) pci_header;

typedef struct pci_device {
    pci_header header;
    uint32_t base_address[6];
    uint32_t cardbus_cis;
    uint16_t subsystem_vendor;
    uint16_t subsystem_device;
    uint32_t rom_base_address;
    uint8_t capabilities_pointer;
    uint8_t reserved[3];
    uint32_t reserved2;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;
} __attribute__((packed)) pci_device;

typedef struct pci_pci_bridge {
    pci_header header;
    uint32_t base_address[2];
    uint8_t primary_bus;
    uint8_t secondary_bus;
    uint8_t subordinate_bus;
    uint8_t secondary_latency_timer;
    uint8_t io_base;
    uint8_t io_limit;
    uint16_t secondary_status;
    uint16_t memory_base;
    uint16_t memory_limit;
    uint16_t prefetch_memory_base;
    uint16_t prefetch_memory_limit;
    uint32_t prefetch_base_upper32;
    uint32_t prefetch_limit_upper32;
    uint16_t io_base_upper16;
    uint16_t io_limit_upper16;
    uint8_t capabilities_pointer;
    uint8_t reserved[3];
    uint32_t expansion_rom_base_address;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint16_t bridge_control;
} __attribute__((packed)) pci_pci_bridge;

typedef struct pci_cardbus_bridge {
    pci_header header;
    uint32_t cardbus_socket_exca_base_address;
    uint8_t offset_capabilities_pointer;
    uint8_t reserved;
    uint16_t secondary_status;
    uint8_t pci_bus_number;
    uint8_t cardbus_bus_number;
    uint8_t subordinate_bus;
    uint8_t cardbus_latency_timer;
    uint32_t memory_base_0;
    uint32_t memory_limit_0;
    uint32_t memory_base_1;
    uint32_t memory_limit_1;
    uint32_t io_base_0;
    uint32_t io_limit_0;
    uint32_t io_base_1;
    uint32_t io_limit_1;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint16_t bridge_control;
    uint16_t subsystem_device_id;
    uint16_t subsystem_vendor_id;
    uint32_t legacy_base_address;
} __attribute__((packed)) pci_cardbus_bridge;

void pci_init(void);

uint64_t pci_get_bar_address(uint32_t bar[], int i);

uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint16_t offset);

void pci_write_word(uint8_t bus, uint8_t slot, uint8_t func, uint16_t offset, uint16_t value);

void pci_read_header(uint8_t bus, uint8_t slot, uint8_t func, pci_header *header);

void pci_read_device(uint8_t bus, uint8_t slot, uint8_t func, pci_header *header, pci_device *device);

void pci_write_device(uint8_t bus, uint8_t slot, uint8_t func, pci_device *device);

void pci_read_pci_bridge(uint8_t bus, uint8_t slot, uint8_t func, pci_header *header, pci_pci_bridge *bridge);

void pci_write_pci_bridge(uint8_t bus, uint8_t slot, uint8_t func, pci_pci_bridge *bridge);

void pci_read_cardbus_bridge(uint8_t bus, uint8_t slot, uint8_t func, pci_header *header, pci_cardbus_bridge *bridge);

void pci_write_cardbus_bridge(uint8_t bus, uint8_t slot, uint8_t func, pci_cardbus_bridge *bridge);

void pci_check_device(uint8_t bus, uint8_t device);

void pci_check_bus(uint8_t bus);

void pci_check_function(uint8_t bus, uint8_t device, uint8_t func, pci_header *h);

void pci_discover_devices(void);

#endif // PCI_H
