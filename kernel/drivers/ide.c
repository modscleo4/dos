#include "ide.h"

#define DEBUG 1
#define DEBUG_SERIAL 1

#include <stdio.h>
#include <stdlib.h>
#include "../bits.h"
#include "../debug.h"
#include "../cpu/mmu.h"
#include "../modules/bitmap.h"
#include "../modules/timer.h"

ide_channel_registers ide_channels[2];
ide_device ide_devices[4];

bool support_dma;

iodriver *ide_init(pci_device *device) {
    dbgprint("IDE: Initializing IDE controller...\n");
    support_dma = false;

    uint8_t *ide_buf = (uint8_t *)malloc_align(2048, BITMAP_PAGE_SIZE);

    int count = 0;

    if (device->base_address[4]) {
        support_dma = true;
    }

    ide_channels[ATA_PRIMARY].base = pci_get_bar_address(device->base_address, 0) + 0x1F0 * (!device->base_address[0]);
    ide_channels[ATA_PRIMARY].ctrl = pci_get_bar_address(device->base_address, 1) + 0x3F6 * (!device->base_address[1]);
    ide_channels[ATA_SECONDARY].base = pci_get_bar_address(device->base_address, 2) + 0x170 * (!device->base_address[2]);
    ide_channels[ATA_SECONDARY].ctrl = pci_get_bar_address(device->base_address, 3) + 0x376 * (!device->base_address[3]);
    ide_channels[ATA_PRIMARY].bmide = pci_get_bar_address(device->base_address, 4) + 0;
    ide_channels[ATA_SECONDARY].bmide = pci_get_bar_address(device->base_address, 4) + 8;

    // Disable interrupts
    ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
    ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            unsigned char err = 0;
            unsigned char type = IDE_ATA;
            unsigned char status;
            ide_devices[count].reserved = 0;

            ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
            timer_wait(1);

            ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            timer_wait(1);

            if (ide_read(i, ATA_REG_STATUS) == 0) {
                continue;
            }

            while (true) {
                status = ide_read(i, ATA_REG_STATUS);
                if (ISSET_BIT_INT(status, ATA_SR_ERR)) {
                    err = 1;
                    break;
                }

                if (!ISSET_BIT_INT(status, ATA_SR_BSY) && ISSET_BIT_INT(status, ATA_SR_DRQ)) {
                    break;
                }
            }

            if (err) {
                unsigned char cl = ide_read(i, ATA_REG_LBA1);
                unsigned char ch = ide_read(i, ATA_REG_LBA2);

                if (cl == 0x14 && ch == 0xEB) {
                    type = IDE_ATAPI;
                } else if (cl == 0x69 && ch == 0x96) {
                    type = IDE_ATAPI;
                } else {
                    continue;
                }

                ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                timer_wait(1);
            }

            ide_read_buffer(i, ATA_REG_DATA, (unsigned int *)ide_buf, 128);

            ide_devices[count].reserved = 1;
            ide_devices[count].type = type;
            ide_devices[count].channel = i;
            ide_devices[count].drive = j;
            ide_devices[count].signature = *((uint16_t *) (ide_buf + ATA_ID_DEVICETYPE));
            ide_devices[count].capabilities = *((uint16_t *) (ide_buf + ATA_ID_CAPABILITIES));
            ide_devices[count].command_sets = *((uint32_t *) (ide_buf + ATA_ID_COMMANDSETS));

            if (ISSET_BIT_INT(ide_devices[count].command_sets, (1UL << 26UL))) {
                ide_devices[count].size = *((uint32_t *) (ide_buf + ATA_ID_MAX_LBA_EXT));
            } else {
                ide_devices[count].size = *((uint32_t *) (ide_buf + ATA_ID_MAX_LBA));
            }

            for (int k = 0; k < 40; k += 2) {
                ide_devices[count].model[k] = ide_buf[ATA_ID_MODEL + k + 1];
                ide_devices[count].model[k + 1] = ide_buf[ATA_ID_MODEL + k];
            }

            ide_devices[count].model[40] = 0;

            count++;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (ide_devices[i].reserved != 1) {
            continue;
        }

        dbgprint("%s drive %d: %d kB: %s\n",
            (const char *[]){"ATA", "ATAPI"}[ide_devices[i].type],
            i,
            ide_devices[i].size / 2,
            ide_devices[i].model
        );
    }

    iodriver *driver = malloc(sizeof(iodriver));
    driver->device = -1;
    driver->io_buffer = ide_buf;
    driver->sector_size = 512;
    driver->reset = NULL;
    driver->start = &ide_motor_on;
    driver->stop = &ide_motor_off;
    driver->read_sector = &ide_sector_read;
    driver->write_sector = &ide_sector_write;
    return driver;
}

unsigned char ide_read(unsigned char channel, unsigned char reg) {
    unsigned char result;

    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channels[channel].n_ien);
    }

    if (reg < 0x08) {
        result = inb(ide_channels[channel].base + reg - 0x00);
    } else if (reg < 0x0C) {
        result = inb(ide_channels[channel].base + reg - 0x06);
    } else if (reg < 0x0E) {
        result = inb(ide_channels[channel].ctrl + reg - 0x0A);
    } else if (reg < 0x16) {
        result = inb(ide_channels[channel].bmide + reg - 0x0E);
    }

    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].n_ien);
    }

    return result;
}

void ide_write(unsigned char channel, unsigned char reg, unsigned char data) {
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channels[channel].n_ien);
    }

    if (reg < 0x08) {
        outb(ide_channels[channel].base + reg - 0x00, data);
    } else if (reg < 0x0C) {
        outb(ide_channels[channel].base + reg - 0x06, data);
    } else if (reg < 0x0E) {
        outb(ide_channels[channel].ctrl + reg - 0x0A, data);
    } else if (reg < 0x16) {
        outb(ide_channels[channel].bmide + reg - 0x0E, data);
    }

    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].n_ien);
    }
}

void ide_read_buffer(unsigned char channel, unsigned char reg, unsigned int *buffer, unsigned int quads) {
    unsigned int i;

    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channels[channel].n_ien);
    }

    if (reg < 0x08) {
        insl(ide_channels[channel].base + reg - 0x00, buffer, quads);
    } else if (reg < 0x0C) {
        insl(ide_channels[channel].base + reg - 0x06, buffer, quads);
    } else if (reg < 0x0E) {
        insl(ide_channels[channel].ctrl + reg - 0x0A, buffer, quads);
    } else if (reg < 0x16) {
        insl(ide_channels[channel].bmide + reg - 0x0E, buffer, quads);
    }

    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].n_ien);
    }
}

unsigned char ide_polling(unsigned char channel, unsigned int advanced_check) {
    ata_400ns_delay(channel);

    while (ISSET_BIT_INT(ide_read(channel, ATA_REG_STATUS), ATA_SR_BSY)) {}

    if (advanced_check) {
        unsigned char state = ide_read(channel, ATA_REG_STATUS);

        if (ISSET_BIT_INT(state, ATA_SR_ERR)) {
            return 2;
        }

        if (ISSET_BIT_INT(state, ATA_SR_DF)) {
            return 1;
        }

        if (!ISSET_BIT_INT(state, ATA_SR_DRQ)) {
            return 3;
        }
    }

    return 0;
}

unsigned char ide_print_error(iodriver *driver, unsigned char err) {
    if (!err) {
        return 0;
    }

    if (err == 1) {
        printf("%s: Device Fault\n", __func__);
        err = 19;
    } else if (err == 2) {
        unsigned char st = ide_read(ide_devices[driver->device].channel, ATA_REG_ERROR);

        if (ISSET_BIT_INT(st, ATA_ERR_AMNT)) {
            printf("%s: No Address Mark Found\n", __func__);
            err = 7;
        } else if (ISSET_BIT_INT(st, ATA_ERR_TKZNF)) {
            printf("%s: No Media or Media Error\n", __func__);
            err = 3;
        } else if (ISSET_BIT_INT(st, ATA_ERR_ABRT)) {
            printf("%s: Command Aborted\n", __func__);
            err = 20;
        } else if (ISSET_BIT_INT(st, ATA_ERR_MCR)) {
            printf("%s: No Media or Media Error\n", __func__);
            err = 3;
        } else if (ISSET_BIT_INT(st, ATA_ERR_IDNF)) {
            printf("%s: ID mark Not Found\n", __func__);
            err = 21;
        } else if (ISSET_BIT_INT(st, ATA_ERR_MC)) {
            printf("%s: No Media or Media Error\n", __func__);
            err = 3;
        } else if (ISSET_BIT_INT(st, ATA_ERR_UNC)) {
            printf("%s: Uncorrectable Data Error\n", __func__);
            err = 22;
        } else if (ISSET_BIT_INT(st, ATA_ERR_BBK)) {
            printf("%s: Bad Block\n", __func__);
            err = 13;
        }
    } else if (err == 3) {
        printf("%s: Reads Nothing\n", __func__);
        err = 23;
    } else if (err == 4) {
        printf("%s: Write Protected\n", __func__);
        err = 8;
    }

    printf("%s: [%s %s] %s\n",
        __func__,
        (const char *[]){"Primary", "Secondary"}[ide_devices[driver->device].channel],
        (const char *[]){"Master", "Slave"}[ide_devices[driver->device].drive],
        ide_devices[driver->device].model
    );

    return err;
}

void ide_motor_on(iodriver *driver) {
    unsigned int ide_channel = ide_devices[driver->device].channel;

    if (support_dma) {
        char command_register = inb(ide_channels[ide_channel].bmide + ATA_BMR_COMMAND);
        command_register = ENABLE_BIT(command_register, 0);
        outb(ide_channels[ide_channel].bmide + ATA_BMR_COMMAND, command_register);
    }
}

void ide_motor_off(iodriver *driver) {
    unsigned int ide_channel = ide_devices[driver->device].channel;

    if (support_dma) {
        char command_register = inb(ide_channels[ide_channel].bmide + ATA_BMR_COMMAND);
        command_register = DISABLE_BIT(command_register, 0);
        outb(ide_channels[ide_channel].bmide + ATA_BMR_COMMAND, command_register);
    }
}

int ide_do_sector(IOOperation direction, iodriver *driver, unsigned long int lba, unsigned int number_of_sectors, uint8_t *buffer, bool keepOn) {
    unsigned int ide_channel = ide_devices[driver->device].channel;
    unsigned int ide_drive = ide_devices[driver->device].drive;

    unsigned int lba_mode = 0;
    uint8_t _lba[6];

    unsigned int head;
    unsigned int cylinder;
    unsigned int sector;

    uint8_t bmr_command_reg = 0;

    if (lba >= 0x10000000) {
        lba_mode = 2;
        _lba[0] = (lba & 0xFF) >> 0;
        _lba[1] = (lba & 0xFF00) >> 8;
        _lba[2] = (lba & 0xFF0000) >> 16;
        _lba[3] = (lba & 0xFF000000) >> 24;
        _lba[4] = 0;
        _lba[5] = 0;
        head = 0;
    } else if (ISSET_BIT_INT(ide_devices[driver->device].capabilities, 0x200)) {
        lba_mode = 1;
        _lba[0] = (lba & 0xFF) >> 0;
        _lba[1] = (lba & 0xFF00) >> 8;
        _lba[2] = (lba & 0xFF0000) >> 16;
        _lba[3] = 0;
        _lba[4] = 0;
        _lba[5] = 0;
        head = (lba & 0xF000000) >> 24;
    } else {
        lba_mode = 0;
        sector = (lba % 63) + 1;
        cylinder = (lba + 1 - sector) / (16 * 63);
        _lba[0] = sector;
        _lba[1] = (cylinder >> 0) & 0xFF;
        _lba[2] = (cylinder >> 8) & 0xFF;
        _lba[3] = 0;
        _lba[4] = 0;
        _lba[5] = 0;
        head = (lba + 1 - sector) % (16 * 63) / 63;
    }

    int cmd = 0;
    if (lba_mode == 0 && !support_dma && direction == IO_READ) cmd = ATA_CMD_READ_PIO;
    if (lba_mode == 1 && !support_dma && direction == IO_READ) cmd = ATA_CMD_READ_PIO;
    if (lba_mode == 2 && !support_dma && direction == IO_READ) cmd = ATA_CMD_READ_PIO_EXT;
    if (lba_mode == 0 && support_dma && direction == IO_READ) cmd = ATA_CMD_READ_DMA;
    if (lba_mode == 1 && support_dma && direction == IO_READ) cmd = ATA_CMD_READ_DMA;
    if (lba_mode == 2 && support_dma && direction == IO_READ) cmd = ATA_CMD_READ_DMA_EXT;
    if (lba_mode == 0 && !support_dma && direction == IO_WRITE) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 1 && !support_dma && direction == IO_WRITE) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 2 && !support_dma && direction == IO_WRITE) cmd = ATA_CMD_WRITE_PIO_EXT;
    if (lba_mode == 0 && support_dma && direction == IO_WRITE) cmd = ATA_CMD_WRITE_DMA;
    if (lba_mode == 1 && support_dma && direction == IO_WRITE) cmd = ATA_CMD_WRITE_DMA;
    if (lba_mode == 2 && support_dma && direction == IO_WRITE) cmd = ATA_CMD_WRITE_DMA_EXT;

    if (!support_dma) {
        ide_write(ide_channel, ATA_REG_CONTROL, ide_channels[ide_channel].n_ien = 0 + 0x02);
        ide_polling(ide_channel, 0);
    } else {
        prd_table prdt;
        prdt.addr = (uint32_t)mmu_get_physical_address((uintptr_t)buffer);
        prdt.size = number_of_sectors * driver->sector_size;
        prdt.reserved = 0;
        prdt.last = true;

        outb(ide_channels[ide_channel].bmide + ATA_BMR_COMMAND, 0);

        outl(ide_channels[ide_channel].bmide + ATA_BMR_PRDT, (uint32_t)mmu_get_physical_address((uintptr_t)&prdt));

        ide_polling(ide_channel, 0);

        bmr_command_reg = 0;
        if (direction == IO_READ) {
            bmr_command_reg = ENABLE_BIT(bmr_command_reg, 3);
        } else {
            bmr_command_reg = DISABLE_BIT(bmr_command_reg, 3);
        }
        outb(ide_channels[ide_channel].bmide + ATA_BMR_COMMAND, bmr_command_reg);

        ide_polling(ide_channel, 0);

        uint8_t status_register = inb(ide_channels[ide_channel].bmide + ATA_BMR_STATUS);
        status_register = DISABLE_BIT(status_register, 1); // Clear interrupt
        status_register = DISABLE_BIT(status_register, 2); // Clear error
        outb(ide_channels[ide_channel].bmide + ATA_BMR_STATUS, status_register);
    }

    if (lba_mode == 0) {
        ide_write(ide_channel, ATA_REG_HDDEVSEL, 0xA0 | (ide_drive << 4) | head);
    } else {
        ide_write(ide_channel, ATA_REG_HDDEVSEL, 0xE0 | (ide_drive << 4) | head);
    }

    ide_write(ide_channel, ATA_REG_FEATURES, 0);

    if (lba_mode == 2) {
        ide_write(ide_channel, ATA_REG_SECCOUNT1, 0);
        ide_write(ide_channel, ATA_REG_LBA3, _lba[3]);
        ide_write(ide_channel, ATA_REG_LBA4, _lba[4]);
        ide_write(ide_channel, ATA_REG_LBA5, _lba[5]);
    }
    ide_write(ide_channel, ATA_REG_SECCOUNT0, number_of_sectors);
    ide_write(ide_channel, ATA_REG_LBA0, _lba[0]);
    ide_write(ide_channel, ATA_REG_LBA1, _lba[1]);
    ide_write(ide_channel, ATA_REG_LBA2, _lba[2]);
    ide_write(ide_channel, ATA_REG_COMMAND, cmd);

    if (support_dma) {
        bmr_command_reg = ENABLE_BIT(bmr_command_reg, 0); // Start
        outb(ide_channels[ide_channel].bmide + ATA_BMR_COMMAND, bmr_command_reg);

        ide_motor_on(driver);

        if (ide_channel == 0) {
            ata_wait_irq_primary();
        } else {
            ata_wait_irq_secondary();
        }

        bmr_command_reg = DISABLE_BIT(bmr_command_reg, 0); // Stop
        outb(ide_channels[ide_channel].bmide + ATA_BMR_COMMAND, bmr_command_reg);

        if (!keepOn) {
            ide_motor_off(driver);
        }

        ide_polling(ide_channel, false);

        uint8_t status_register = inb(ide_channels[ide_channel].bmide + ATA_BMR_STATUS);
        if (ISSET_BIT(status_register, 1)) { // Error
            outb(ide_channels[ide_channel].bmide + ATA_BMR_STATUS, 2);
            return 1;
        }
    } else {
        if (ide_print_error(driver, ide_polling(ide_channel, true))) {
            return 1;
        }

        insm(ide_channels[ide_channel].base, (uint8_t *)buffer, 256);
        ide_polling(ide_channel, 0);

        if (!keepOn) {
            ide_motor_off(driver);
        }
    }

    return 0;
}

int ide_sector_read(iodriver *driver, unsigned long int lba, uint8_t *buffer, bool keepOn) {
    return ide_do_sector(IO_READ, driver, lba, 1, buffer, keepOn);
}

int ide_sector_write(iodriver *driver, unsigned long int lba, uint8_t *buffer, bool keepOn) {
    return ide_do_sector(IO_WRITE, driver, lba, 1, buffer, keepOn);
}
