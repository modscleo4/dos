#include "ide.h"

#include "../bits.h"
#include "../debug.h"
#include "../modules/timer.h"
#include <stdio.h>

ide_channel_registers ide_channels[2];
ide_device ide_devices[4];

unsigned char ide_buf[2048] = {0};
volatile unsigned static char ide_irq_invoked = 0;
bool support_dma;

iodriver *ide_init(unsigned int BAR0, unsigned int BAR1, unsigned int BAR2, unsigned int BAR3, unsigned int BAR4) {
    dbgprint("IDE: Initializing IDE controller...\n");
    support_dma = false;

    int count = 0;

    if (BAR4) {
        support_dma = false;
    }

    ide_channels[ATA_PRIMARY].base = (BAR0 & 0xFFFFFFFC) + 0x1F0 * (!BAR0);
    ide_channels[ATA_PRIMARY].ctrl = (BAR1 & 0xFFFFFFFC) + 0x3F6 * (!BAR1);
    ide_channels[ATA_SECONDARY].base = (BAR2 & 0xFFFFFFFC) + 0x170 * (!BAR2);
    ide_channels[ATA_SECONDARY].ctrl = (BAR3 & 0xFFFFFFFC) + 0x376 * (!BAR3);
    ide_channels[ATA_PRIMARY].bmide = (BAR4 & 0xFFFFFFFC) + 0;
    ide_channels[ATA_SECONDARY].bmide = (BAR4 & 0xFFFFFFFC) + 8;

    // Disable interrupts
    ide_write(ATA_PRIMARY, ATA_REG_CONTROL, !support_dma + 2);
    ide_write(ATA_SECONDARY, ATA_REG_CONTROL, !support_dma + 2);

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
            ide_devices[count].signature = *((unsigned short int *) (ide_buf + ATA_ID_DEVICETYPE));
            ide_devices[count].capabilities = *((unsigned short int *) (ide_buf + ATA_ID_CAPABILITIES));
            ide_devices[count].command_sets = *((unsigned int *) (ide_buf + ATA_ID_COMMANDSETS));

            if (ISSET_BIT_INT(ide_devices[count].command_sets, (1 << 26))) {
                ide_devices[count].size = *((unsigned int *) (ide_buf + ATA_ID_MAX_LBA_EXT));
            } else {
                ide_devices[count].size = *((unsigned int *) (ide_buf + ATA_ID_MAX_LBA));
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

    static iodriver driver;
    driver.device = -1;
    driver.io_buffer = ide_buf;
    driver.reset = NULL;
    driver.start = &ide_motor_on;
    driver.stop = &ide_motor_off;
    driver.read_sector = &ide_sector_read;
    driver.write_sector = &ide_sector_write;
    return &driver;
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

unsigned char ide_print_error(unsigned int drive, unsigned char err) {
    if (!err) {
        return 0;
    }

    const char *fn = "ide_print_error";

    if (err == 1) {
        printf("%s: Device Fault\n", fn);
        err = 19;
    } else if (err == 2) {
        unsigned char st = ide_read(ide_devices[drive].channel, ATA_REG_ERROR);

        if (ISSET_BIT_INT(st, ATA_ERR_AMNT)) {
            printf("%s: No Address Mark Found\n", fn);
            err = 7;
        } else if (ISSET_BIT_INT(st, ATA_ERR_TKZNF)) {
            printf("%s: No Media or Media Error\n", fn);
            err = 3;
        } else if (ISSET_BIT_INT(st, ATA_ERR_ABRT)) {
            printf("%s: Command Aborted\n", fn);
            err = 20;
        } else if (ISSET_BIT_INT(st, ATA_ERR_MCR)) {
            printf("%s: No Media or Media Error\n", fn);
            err = 3;
        } else if (ISSET_BIT_INT(st, ATA_ERR_IDNF)) {
            printf("%s: ID mark Not Found\n", fn);
            err = 21;
        } else if (ISSET_BIT_INT(st, ATA_ERR_MC)) {
            printf("%s: No Media or Media Error\n", fn);
            err = 3;
        } else if (ISSET_BIT_INT(st, ATA_ERR_UNC)) {
            printf("%s: Uncorrectable Data Error\n", fn);
            err = 22;
        } else if (ISSET_BIT_INT(st, ATA_ERR_BBK)) {
            printf("%s: Bad Block\n", fn);
            err = 13;
        }
    } else if (err == 3) {
        printf("%s: Reads Nothing\n", fn);
        err = 23;
    } else if (err == 4) {
        printf("%s: Write Protected\n", fn);
        err = 8;
    }

    printf("%s: [%s %s] %s\n",
        fn,
        (const char *[]){"Primary", "Secondary"}[ide_devices[drive].channel],
        (const char *[]){"Master", "Slave"}[ide_devices[drive].drive],
        ide_devices[drive].model
    );

    return err;
}

void ide_motor_on(unsigned int drive) {
    unsigned int ide_channel = ide_devices[drive].channel;

    if (support_dma) {
        char command_register = inb(ide_channels[ide_channel].bmide + ATA_BMR_Command);
        command_register = ENABLE_BIT(command_register, 0);
        dbgprint("enabling motor, command_register = %x\n", command_register);
        outb(ide_channels[ide_channel].bmide + ATA_BMR_Command, command_register);
    }
}

void ide_motor_off(unsigned int drive) {
    unsigned int ide_channel = ide_devices[drive].channel;

    if (support_dma) {
        char command_register = inb(ide_channels[ide_channel].bmide + ATA_BMR_Command);
        command_register = DISABLE_BIT(command_register, 0);
        outb(ide_channels[ide_channel].bmide + ATA_BMR_Command, command_register);
    }
}

int ide_do_sector(io_operation direction, unsigned int drive, unsigned long int lba, unsigned int number_of_sectors, unsigned char *buffer, bool keepOn) {
    unsigned int ide_channel = ide_devices[drive].channel;
    unsigned int ide_drive = ide_devices[drive].drive;

    unsigned int lba_mode = 0;
    unsigned char _lba[6];

    unsigned int head;
    unsigned int cylinder;
    unsigned int sector;

    if (lba >= 0x10000000) {
        lba_mode = 2;
        _lba[0] = (lba & 0xFF) >> 0;
        _lba[1] = (lba & 0xFF00) >> 8;
        _lba[2] = (lba & 0xFF0000) >> 16;
        _lba[3] = (lba & 0xFF000000) >> 24;
        _lba[4] = 0;
        _lba[5] = 0;
        head = 0;
    } else if (ISSET_BIT_INT(ide_devices[drive].capabilities, 0x200)) {
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
    if (lba_mode == 0 && !support_dma && direction == io_read) cmd = ATA_CMD_READ_PIO;
    if (lba_mode == 1 && !support_dma && direction == io_read) cmd = ATA_CMD_READ_PIO;
    if (lba_mode == 2 && !support_dma && direction == io_read) cmd = ATA_CMD_READ_PIO_EXT;
    if (lba_mode == 0 && support_dma && direction == io_read) cmd = ATA_CMD_READ_DMA;
    if (lba_mode == 1 && support_dma && direction == io_read) cmd = ATA_CMD_READ_DMA;
    if (lba_mode == 2 && support_dma && direction == io_read) cmd = ATA_CMD_READ_DMA_EXT;
    if (lba_mode == 0 && !support_dma && direction == io_write) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 1 && !support_dma && direction == io_write) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 2 && !support_dma && direction == io_write) cmd = ATA_CMD_WRITE_PIO_EXT;
    if (lba_mode == 0 && support_dma && direction == io_write) cmd = ATA_CMD_WRITE_DMA;
    if (lba_mode == 1 && support_dma && direction == io_write) cmd = ATA_CMD_WRITE_DMA;
    if (lba_mode == 2 && support_dma && direction == io_write) cmd = ATA_CMD_WRITE_DMA_EXT;

    if (!support_dma) {
        ide_write(ide_channel, ATA_REG_CONTROL, ide_channels[ide_channel].n_ien = (ide_irq_invoked = 0) + 0x02);
        ide_polling(ide_channel, 0);
    } else {
        prd_table prdt;
        prdt.addr = buffer;
        prdt.size = number_of_sectors;
        prdt.reserved = 0;
        prdt.last = 1;

        outb(ide_channels[ide_channel].bmide + ATA_BMR_Command, 0);

        outl(ide_channels[ide_channel].bmide + ATA_BMR_PRDT, (unsigned int)&prdt);

        ide_polling(ide_channel, 0);

        unsigned char command_register = 0;
        if (direction == io_read) {
            command_register = ENABLE_BIT(command_register, 3);
        } else {
            command_register = DISABLE_BIT(command_register, 3);
        }
        outb(ide_channels[ide_channel].bmide + ATA_BMR_Command, command_register);

        ide_polling(ide_channel, 0);

        unsigned char status_register = inb(ide_channels[ide_channel].bmide + ATA_BMR_Status);
        status_register = DISABLE_BIT(status_register, 1);
        status_register = DISABLE_BIT(status_register, 2);
        outb(ide_channels[ide_channel].bmide + ATA_BMR_Status, status_register);
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
        ide_motor_on(drive);

        if (ide_channel == 0) {
            wait_irq14();
        } else {
            wait_irq15();
        }

        ide_motor_off(drive);

        ide_polling(ide_channel, 0);

        unsigned char status_register = inb(ide_channels[ide_channel].bmide + ATA_BMR_Status);
        if (ISSET_BIT_INT(status_register, 1)) {
            status_register = DISABLE_BIT(status_register, 1);
            outb(ide_channels[ide_channel].bmide + ATA_BMR_Status, status_register);
        }
    } else {
        if (ide_print_error(drive, ide_polling(ide_channel, 1))) {
            return 1;
        }

        insm(ide_channels[ide_channel].base, buffer, 256);
        ide_polling(ide_channel, 0);
    }

    return 0;
}

int ide_sector_read(unsigned int drive, unsigned long int lba, unsigned char *buffer, bool keepOn) {
    return ide_do_sector(io_read, drive, lba, 1, buffer, keepOn);
}

int ide_sector_write(unsigned int drive, unsigned long int lba, unsigned char *buffer, bool keepOn) {
    return ide_do_sector(io_write, drive, lba, 1, buffer, keepOn);
}
