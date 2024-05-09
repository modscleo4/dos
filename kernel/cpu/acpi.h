#ifndef KERNEL_ACPI_H
#define KERNEL_ACPI_H

#include <stdint.h>

// Root System Description Pointer
#pragma pack(push, 1)
typedef struct acpi_rsdp {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} acpi_rsdp;
#pragma pack(pop)

// Root System Description Table
#pragma pack(push, 1)
typedef struct acpi_rsdt {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
    //uint32_t *entries;
} acpi_rsdt;
#pragma pack(pop)

// Generic Address Structure
#pragma pack(push, 1)
typedef struct acpi_generic_address_structure {
    uint8_t address_space_id;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
} acpi_generic_address_structure;
#pragma pack(pop)

enum ACPIAddressSpaceID {
    ACPI_ADDRESS_SPACE_SYSTEM_MEMORY = 0,
    ACPI_ADDRESS_SPACE_SYSTEM_IO,
    ACPI_ADDRESS_SPACE_PCI_CONFIGURATION_SPACE,
    ACPI_ADDRESS_SPACE_EMBEDDED_CONTROLLER,
    ACPI_ADDRESS_SPACE_SMBUS,
    ACPI_ADDRESS_SPACE_CMOS,
    ACPI_ADDRESS_SPACE_PCI_BAR_TARGET,
    ACPI_ADDRESS_SPACE_IPMI,
    ACPI_ADDRESS_SPACE_GPIO,
    ACPI_ADDRESS_SPACE_GENERIC_SERIAL_BUS,
    ACPI_ADDRESS_SPACE_PLATFORM_COMMUNICATION_CHANNEL
};

// Fixed ACPI Description Table
#pragma pack(push, 1)
typedef struct acpi_fadt {
    acpi_rsdt header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;

    uint8_t reserved;

    uint8_t preferred_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_cnt;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t pm1_event_length;
    uint8_t pm1_control_length;
    uint8_t pm2_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe0_length;
    uint8_t gpe1_length;
    uint8_t gpe1_base;
    uint8_t cstate_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;

    uint16_t boot_architecture_flags;

    uint8_t reserved2;
    uint32_t flags;

    acpi_generic_address_structure reset_reg;

    uint8_t reset_value;
    uint8_t reserved3[3];
} acpi_fadt;
#pragma pack(pop)

// Differentiated System Description Table
#pragma pack(push, 1)
typedef struct acpi_dsdt {
    acpi_rsdt header;
    uint8_t definition_block[0];
} acpi_dsdt;
#pragma pack(pop)

enum ACPIPM1AControlRegisters {
    ACPI_PM1A_CONTROL_SCI_EN = 1 << 0,
    ACPI_PM1A_CONTROL_BM_RLD = 1 << 1,
    ACPI_PM1A_CONTROL_GBL_RLS = 1 << 2,
    ACPI_PM1A_CONTROL_SLP_EN = 1 << 13
};

void acpi_init(acpi_rsdp *rsdp);

void acpi_shutdown(void);

void acpi_reboot(void);

#endif // KERNEL_ACPI_H
