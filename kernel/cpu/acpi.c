#include "acpi.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "irq.h"
#include "mmu.h"
#include "system.h"
#include "../bits.h"
#include "../debug.h"
#include "../drivers/pci.h"
#include "../modules/bitmap.h"
#include "../modules/timer.h"

static acpi_fadt *fadt;
static uint8_t acpi_typ_a;
static uint8_t acpi_typ_b;

static void acpi_sci_handler(registers *r, uint32_t int_no) {
    dbgprint("ACPI SCI triggered!\n");
}

static bool acpi_rsdp_checksum(acpi_rsdp *rsdp) {
    uint8_t checksum = 0;
    uint8_t *ptr = (uint8_t *)rsdp;
    for (int i = 0; i < sizeof(acpi_rsdp); i++) {
        checksum += ptr[i];
    }

    return checksum == 0;
}

static bool acpi_rsdt_checksum(acpi_rsdt *rsdt) {
    uint8_t checksum = 0;
    uint8_t *ptr = (uint8_t *)rsdt;
    for (int i = 0; i < rsdt->length; i++) {
        checksum += ptr[i];
    }

    return checksum == 0;
}

static void acpi_read_dsdt(acpi_dsdt *dsdt) {
    // TODO: Implement ACPICA

    // Map the DSDT into memory
    mmu_map_pages(current_pdt, (uintptr_t)dsdt, (uintptr_t)dsdt, 4, true, false, true);

    if (!acpi_rsdt_checksum((acpi_rsdt *)dsdt)) {
        dbgprint("ACPI DSDT checksum failed!\n");
        return;
    }

    dbgprint("ACPI DSDT Signature: %.4s\n", dsdt->header.signature);
    dbgprint("ACPI DSDT Length: %d\n", dsdt->header.length);
    dbgprint("ACPI DSDT Revision: %d\n", dsdt->header.revision);
    dbgprint("ACPI DSDT OEM ID: %.4s\n", dsdt->header.oem_id);
    dbgprint("ACPI DSDT OEM Table ID: %.6s\n", dsdt->header.oem_table_id);
    dbgprint("ACPI DSDT OEM Revision: %d\n", dsdt->header.oem_revision);
    dbgprint("ACPI DSDT Creator ID: %d\n", dsdt->header.creator_id);
    dbgprint("ACPI DSDT Creator Revision: %d\n", dsdt->header.creator_revision);

    // Find the \_S5 object
    uint8_t *ptr = (uint8_t *)dsdt;

    // Skip the header
    ptr += sizeof(acpi_dsdt);

    // Go through the rest of the DSDT
    while (ptr < (uint8_t *)dsdt + dsdt->header.length) {
        if (!memcmp(ptr, "_S5_", 4)) {
            ptr += 5;
            ptr += ((*ptr & 0xC0) >> 6) + 2; // calculate PkgLength size

            // Found the \_S5 object
            if (*ptr == 0x0A)
                ptr++; // skip byteprefix
            acpi_typ_a = *ptr;
            ptr++;

            // Found the \_S5 object
            if (*ptr == 0x0A)
                ptr++; // skip byteprefix
            acpi_typ_b = *ptr;
            ptr++;

            dbgprint("ACPI TYP A: %d\n", acpi_typ_a);
            dbgprint("ACPI TYP B: %d\n", acpi_typ_b);
            break;
        }

        ptr++;
    }
}

static void acpi_read_fadt(acpi_fadt *fadt) {
    dbgprint("ACPI FADT Signature: %.4s\n", fadt->header.signature);
    dbgprint("ACPI FADT Firmware Control: %x\n", fadt->firmware_ctrl);
    dbgprint("ACPI FADT DSDT: %x\n", fadt->dsdt);
    dbgprint("ACPI FADT Preferred PM Profile: %d\n", fadt->preferred_pm_profile);
    dbgprint("ACPI FADT SCI Interrupt: %d\n", fadt->sci_int);
    dbgprint("ACPI FADT SMI Command: %x\n", fadt->smi_cmd);
    dbgprint("ACPI FADT ACPI Enable: %x\n", fadt->acpi_enable);
    dbgprint("ACPI FADT ACPI Disable: %x\n", fadt->acpi_disable);
    dbgprint("ACPI FADT S4BIOS Request: %x\n", fadt->s4bios_req);
    dbgprint("ACPI FADT P-State Control: %x\n", fadt->pstate_cnt);
    dbgprint("ACPI FADT PM1A Event Block: %x\n", fadt->pm1a_event_block);
    dbgprint("ACPI FADT PM1B Event Block: %x\n", fadt->pm1b_event_block);
    dbgprint("ACPI FADT PM1A Control Block: %x\n", fadt->pm1a_control_block);
    dbgprint("ACPI FADT PM1B Control Block: %x\n", fadt->pm1b_control_block);
    dbgprint("ACPI FADT PM2 Control Block: %x\n", fadt->pm2_control_block);
    dbgprint("ACPI FADT PM Timer Block: %x\n", fadt->pm_timer_block);
    dbgprint("ACPI FADT GPE0 Block: %x\n", fadt->gpe0_block);
    dbgprint("ACPI FADT GPE1 Block: %x\n", fadt->gpe1_block);
    dbgprint("ACPI FADT PM1 Event Length: %d\n", fadt->pm1_event_length);
    dbgprint("ACPI FADT PM1 Control Length: %d\n", fadt->pm1_control_length);
    dbgprint("ACPI FADT PM2 Control Length: %d\n", fadt->pm2_control_length);
    dbgprint("ACPI FADT PM Timer Length: %d\n", fadt->pm_timer_length);
    dbgprint("ACPI FADT GPE0 Block Length: %d\n", fadt->gpe0_length);
    dbgprint("ACPI FADT GPE1 Block Length: %d\n", fadt->gpe1_length);
    dbgprint("ACPI FADT GPE1 Base: %d\n", fadt->gpe1_base);
    dbgprint("ACPI FADT C-State Control: %x\n", fadt->cstate_control);
    dbgprint("ACPI FADT Worst C2 Latency: %d\n", fadt->worst_c2_latency);
    dbgprint("ACPI FADT Worst C3 Latency: %d\n", fadt->worst_c3_latency);
    dbgprint("ACPI FADT Flush Size: %d\n", fadt->flush_size);
    dbgprint("ACPI FADT Flush Stride: %d\n", fadt->flush_stride);
    dbgprint("ACPI FADT Duty Offset: %d\n", fadt->duty_offset);
    dbgprint("ACPI FADT Duty Width: %d\n", fadt->duty_width);
    dbgprint("ACPI FADT Day Alarm: %d\n", fadt->day_alarm);
    dbgprint("ACPI FADT Month Alarm: %d\n", fadt->month_alarm);
    dbgprint("ACPI FADT Century: %d\n", fadt->century);
    dbgprint("ACPI FADT Boot Architecture Flags: %d\n", fadt->boot_architecture_flags);
    dbgprint("ACPI FADT Flags: %d\n", fadt->flags);
    dbgprint("ACPI FADT Reset Register ID: %x\n", fadt->reset_reg.address_space_id);
    dbgprint("ACPI FADT Reset Register: %x\n", fadt->reset_reg.address);
    dbgprint("ACPI FADT Reset Value: %d\n", fadt->reset_value);

    // Enable ACPI
    outb(fadt->smi_cmd, fadt->acpi_enable);

    // Check if ACPI is enabled
    while (!ISSET_BIT(inw(fadt->pm1a_control_block), 0)) {}

    dbgprint("ACPI is enabled!\n");

    // Register the SCI handler
    irq_install_handler(fadt->sci_int, acpi_sci_handler);

    // Enable the SCI
    outw(fadt->pm1a_control_block, ACPI_PM1A_CONTROL_SCI_EN);

    acpi_read_dsdt((acpi_dsdt *)fadt->dsdt);
}

static void acpi_read_rsdt(acpi_rsdt *rsdt) {
    // Map the RSDT into memory
    mmu_map_pages(current_pdt, (uint32_t)rsdt, (uint32_t)rsdt, 1, true, false, true);

    if (!acpi_rsdt_checksum(rsdt)) {
        dbgprint("ACPI RSDT checksum failed!\n");
        return;
    }

    dbgprint("ACPI RSDT Signature: %.4s\n", rsdt->signature);
    dbgprint("ACPI RSDT Length: %d\n", rsdt->length);
    dbgprint("ACPI RSDT Revision: %d\n", rsdt->revision);
    dbgprint("ACPI RSDT OEM ID: %.6s\n", rsdt->oem_id);
    dbgprint("ACPI RSDT OEM Table ID: %.8s\n", rsdt->oem_table_id);
    dbgprint("ACPI RSDT OEM Revision: %d\n", rsdt->oem_revision);
    dbgprint("ACPI RSDT Creator ID: %d\n", rsdt->creator_id);
    dbgprint("ACPI RSDT Creator Revision: %d\n", rsdt->creator_revision);
    if (!memcmp(rsdt->signature, "FACP", 4)) {
        acpi_read_fadt((acpi_fadt *)rsdt);
        fadt = (acpi_fadt *)rsdt;
    } else if (!memcmp(rsdt->signature, "RSDT", 4)) {
        size_t fields = (rsdt->length - (sizeof(acpi_rsdt))) / sizeof(uint32_t);
        dbgprint("ACPI RSDT Entries: %d\n", fields);
        uint32_t *entries = (uint32_t *)((uint32_t)rsdt + sizeof(acpi_rsdt));
        for (int i = 0; i < fields; i++) {
            dbgprint("ACPI RSDT Entry %d: 0x%x\n", i, entries[i]);
            acpi_read_rsdt((acpi_rsdt *) entries[i]);
        }
    }
}

void acpi_init(acpi_rsdp *rsdp) {
    // Map the RSDP into memory
    mmu_map_pages(current_pdt, (uint32_t)rsdp, (uint32_t)rsdp, 1, true, false, true);

    if (acpi_rsdp_checksum(rsdp)) {
        dbgprint("ACPI RSDP found at &0x%x\n", rsdp);

        dbgprint("ACPI RSDP Signature: %.8s\n", rsdp->signature);
        dbgprint("ACPI RSDP OEM ID: %.6s\n", rsdp->oem_id);
        dbgprint("ACPI RSDP Revision: %d\n", rsdp->revision);
        dbgprint("ACPI RSDP RSDT Address: 0x%x\n", rsdp->rsdt_address);

        acpi_rsdt *rsdt = (acpi_rsdt *)rsdp->rsdt_address;
        acpi_read_rsdt(rsdt);
    }
}

void acpi_shutdown(void) {
    if (fadt) {
        outw(fadt->pm1a_control_block, ACPI_PM1A_CONTROL_SLP_EN | (acpi_typ_a << 10));
        if (fadt->pm1b_control_block) {
            outw(fadt->pm1b_control_block, ACPI_PM1A_CONTROL_SLP_EN | (acpi_typ_b << 10));
        }
    }
}

void acpi_reboot(void) {
    if (fadt) {
        switch (fadt->reset_reg.address_space_id) {
            case ACPI_ADDRESS_SPACE_SYSTEM_IO:
                outb(fadt->reset_reg.address, fadt->reset_value);
                break;

            case ACPI_ADDRESS_SPACE_SYSTEM_MEMORY: {
                mmu_map_pages(current_pdt, fadt->reset_reg.address, fadt->reset_reg.address, 1, true, false, true);
                uint8_t *ptr = (uint8_t *)fadt->reset_reg.address;
                *ptr = fadt->reset_value;
                break;
            }

            case ACPI_ADDRESS_SPACE_PCI_CONFIGURATION_SPACE:
                // Segment: 0
                // Bus: 0
                // Slot: (address >> 32) & 0xFFFF
                // Function: (address >> 16) & 0xFFFF
                // Offset: address & 0xFFFF

                pci_write_byte(0, (fadt->reset_reg.address >> 32) & 0xFFFF, (fadt->reset_reg.address >> 16) & 0xFFFF, fadt->reset_reg.address & 0xFFFF, fadt->reset_value);
                break;
        }
    }
}
