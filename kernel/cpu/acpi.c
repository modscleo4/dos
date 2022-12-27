#include "acpi.h"

#include <stdbool.h>
#include <string.h>
#include "../debug.h"

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

static void acpi_read_fadt(acpi_fadt *fadt) {
    //dbgprint("ACPI FADT Signature: %.4s\n", fadt->header.signature);
    //dbgprint("ACPI FADT Firmware Control: %x\n", fadt->firmware_ctrl);
    //dbgprint("ACPI FADT DSDT: %x\n", fadt->dsdt);
    //dbgprint("ACPI FADT Preferred PM Profile: %d\n", fadt->preferred_pm_profile);
    //dbgprint("ACPI FADT SCI Interrupt: %d\n", fadt->sci_int);
    //dbgprint("ACPI FADT SMI Command: %x\n", fadt->smi_cmd);
    //dbgprint("ACPI FADT ACPI Enable: %x\n", fadt->acpi_enable);
    //dbgprint("ACPI FADT ACPI Disable: %x\n", fadt->acpi_disable);
    //dbgprint("ACPI FADT S4BIOS Request: %x\n", fadt->s4bios_req);
    //dbgprint("ACPI FADT P-State Control: %x\n", fadt->pstate_cnt);
    //dbgprint("ACPI FADT PM1A Event Block: %x\n", fadt->pm1a_event_block);
    //dbgprint("ACPI FADT PM1B Event Block: %x\n", fadt->pm1b_event_block);
    //dbgprint("ACPI FADT PM1A Control Block: %x\n", fadt->pm1a_control_block);
    //dbgprint("ACPI FADT PM1B Control Block: %x\n", fadt->pm1b_control_block);
    //dbgprint("ACPI FADT PM2 Control Block: %x\n", fadt->pm2_control_block);
    //dbgprint("ACPI FADT PM Timer Block: %x\n", fadt->pm_timer_block);
    //dbgprint("ACPI FADT GPE0 Block: %x\n", fadt->gpe0_block);
    //dbgprint("ACPI FADT GPE1 Block: %x\n", fadt->gpe1_block);
    //dbgprint("ACPI FADT PM1 Event Length: %d\n", fadt->pm1_event_length);
    //dbgprint("ACPI FADT PM1 Control Length: %d\n", fadt->pm1_control_length);
    //dbgprint("ACPI FADT PM2 Control Length: %d\n", fadt->pm2_control_length);
    //dbgprint("ACPI FADT PM Timer Length: %d\n", fadt->pm_timer_length);
    //dbgprint("ACPI FADT GPE0 Block Length: %d\n", fadt->gpe0_length);
    //dbgprint("ACPI FADT GPE1 Block Length: %d\n", fadt->gpe1_length);
    //dbgprint("ACPI FADT GPE1 Base: %d\n", fadt->gpe1_base);
    //dbgprint("ACPI FADT C-State Control: %x\n", fadt->cstate_control);
    //dbgprint("ACPI FADT Worst C2 Latency: %d\n", fadt->worst_c2_latency);
    //dbgprint("ACPI FADT Worst C3 Latency: %d\n", fadt->worst_c3_latency);
    //dbgprint("ACPI FADT Flush Size: %d\n", fadt->flush_size);
    //dbgprint("ACPI FADT Flush Stride: %d\n", fadt->flush_stride);
    //dbgprint("ACPI FADT Duty Offset: %d\n", fadt->duty_offset);
    //dbgprint("ACPI FADT Duty Width: %d\n", fadt->duty_width);
    //dbgprint("ACPI FADT Day Alarm: %d\n", fadt->day_alarm);
    //dbgprint("ACPI FADT Month Alarm: %d\n", fadt->month_alarm);
    //dbgprint("ACPI FADT Century: %d\n", fadt->century);
}

static void acpi_read_rsdt(acpi_rsdt *rsdt) {
    if (!acpi_rsdt_checksum(rsdt)) {
        dbgprint("ACPI RSDT checksum failed!\n");
        return;
    }

    dbgprint("ACPI RSDT Signature: %.4s\n", rsdt->signature);
    //dbgprint("ACPI RSDT Length: %d\n", rsdt->length);
    //dbgprint("ACPI RSDT Revision: %d\n", rsdt->revision);
    //dbgprint("ACPI RSDT OEM ID: %.6s\n", rsdt->oem_id);
    //dbgprint("ACPI RSDT OEM Table ID: %.8s\n", rsdt->oem_table_id);
    //dbgprint("ACPI RSDT OEM Revision: %d\n", rsdt->oem_revision);
    //dbgprint("ACPI RSDT Creator ID: %d\n", rsdt->creator_id);
    //dbgprint("ACPI RSDT Creator Revision: %d\n", rsdt->creator_revision);
    if (!memcmp(rsdt->signature, "FACP", 4)) {
        acpi_read_fadt((acpi_fadt *)rsdt);
    } else if (!memcmp(rsdt->signature, "RSDT", 4)) {
        size_t fields = (rsdt->length - (sizeof(acpi_rsdt))) / sizeof(uint32_t);
        //dbgprint("ACPI RSDT Entries: %d\n", fields);
        uint32_t *entries = (uint32_t *)((uint32_t)rsdt + sizeof(acpi_rsdt));
        for (int i = 0; i < fields; i++) {
            //dbgprint("ACPI RSDT Entry %d: 0x%x\n", i, entries[i]);
            //dbgwait();
            acpi_read_rsdt((acpi_rsdt *) entries[i]);
        }
    }
}

void acpi_init(void *rdsp_addr) {
    acpi_rsdp *rsdp = (acpi_rsdp *)rdsp_addr;
    if (acpi_rsdp_checksum(rsdp)) {
        dbgprint("ACPI RSDP found at &0x%x\n", rsdp);

        dbgprint("ACPI RSDP Signature: %.8s\n", rsdp->signature);
        dbgprint("ACPI RSDP OEM ID: %.6s\n", rsdp->oem_id);
        dbgprint("ACPI RSDP Revision: %d\n", rsdp->revision);
        dbgprint("ACPI RSDP RSDT Address: 0x%x\n", rsdp->rsdt_address);

        acpi_rsdt *rsdt = (acpi_rsdt *)rsdp->rsdt_address;
        acpi_read_rsdt(rsdt);
        dbgwait();
    }
}
