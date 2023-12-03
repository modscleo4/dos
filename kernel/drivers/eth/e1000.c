#include "e1000.h"

#define DEBUG 1
#define DEBUG_SERIAL 1
#define FORCE_MMIO 1
#define RX_LEN 256
#define TX_LEN 256

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "../../bits.h"
#include "../../debug.h"
#include "../../cpu/interrupts.h"
#include "../../cpu/mmu.h"
#include "../../modules/bitmap.h"
#include "../../modules/timer.h"

/**
 * e1000 Ethernet driver
 */

static void e1000_mmio_write(ethernet_driver *driver, uint32_t reg, uint32_t value) {
    *(uint32_t *)(driver->mmiobase + reg) = value;
}

static uint32_t e1000_mmio_read(ethernet_driver *driver, uint32_t reg) {
    return *(uint32_t *)(driver->mmiobase + reg);
}

static void e1000_io_write(ethernet_driver *driver, uint32_t reg, uint32_t value) {
    if (driver->iobase == 0) {
        return;
    }

    outl(driver->iobase + 0x00, reg);
    outl(driver->iobase + 0x04, value);
}

static uint32_t e1000_io_read(ethernet_driver *driver, uint32_t reg) {
    if (driver->iobase == 0) {
        return 0;
    }

    outl(driver->iobase + 0x00, reg);
    return inl(driver->iobase + 0x04);
}

static void e1000_write(ethernet_driver *driver, uint32_t reg, uint32_t value) {
#if FORCE_MMIO
    if (true) {
#else
    if (!driver->iobase) {
#endif
        e1000_mmio_write(driver, reg, value);
    } else {
        e1000_io_write(driver, reg, value);
    }
}

static uint32_t e1000_read(ethernet_driver *driver, uint32_t reg) {
#if FORCE_MMIO
    if (true) {
#else
    if (!driver->iobase) {
#endif
        return e1000_mmio_read(driver, reg);
    } else {
        return e1000_io_read(driver, reg);
    }
}

static uint16_t e1000_read_eeprom(ethernet_driver *driver, unsigned char address) {
    uint32_t eerd = E1000_REGBIT_EERD_START | (address << 8);
    e1000_write(driver, E1000_REG_EERD, eerd);

    while (!(e1000_read(driver, E1000_REG_EERD) & E1000_REGBIT_EERD_DONE));

    uint16_t data = (e1000_read(driver, E1000_REG_EERD) >> 16);

    eerd = e1000_read(driver, E1000_REG_EERD);
    e1000_write(driver, E1000_REG_EERD, DISABLE_BIT_INT(eerd, E1000_REGBIT_EERD_START));

    return data;
}

static void e1000_lock_eeprom(ethernet_driver *driver) {
    if (e1000_read(driver, E1000_REG_EECD) & E1000_REGBIT_EECD_REQ) {
        return;
    }

    e1000_write(driver, E1000_REG_EECD, e1000_read(driver, E1000_REG_EECD) | E1000_REGBIT_EECD_REQ);
}

static void e1000_unlock_eeprom(ethernet_driver *driver) {
    e1000_write(driver, E1000_REG_EECD, e1000_read(driver, E1000_REG_EECD) & ~E1000_REGBIT_EECD_REQ);
}

static void e1000_read_eeprom_mac(ethernet_driver *driver) {
    e1000_lock_eeprom(driver);
    *(uint16_t *)&driver->mac[0] = e1000_read_eeprom(driver, 0);
    *(uint16_t *)&driver->mac[2] = e1000_read_eeprom(driver, 1);
    *(uint16_t *)&driver->mac[4] = e1000_read_eeprom(driver, 2);
    e1000_unlock_eeprom(driver);
}

static void e1000_read_mac(ethernet_driver *driver) {
    uint32_t mac_low = e1000_read(driver, E1000_REG_RAL);
    uint32_t mac_high = e1000_read(driver, E1000_REG_RAH);

    dbgprint("e1000: RAL: %032b\n", mac_low);
    dbgprint("e1000: RAH: %032b\n", mac_high);
}

static void e1000_write_mac(ethernet_driver *driver, uint8_t mac[6]) {
    uint32_t mac_low = 0;
    uint32_t mac_high = E1000_REGBIT_RAH_AV;
    memcpy(&mac_low, &mac[0], 4);
    memcpy(&mac_high, &mac[4], 2);

    e1000_write(driver, E1000_REG_RAL, mac_low);
    e1000_write(driver, E1000_REG_RAH, mac_high);
}

static void e1000_receive_init(ethernet_driver *driver) {
    e1000_write_mac(driver, driver->mac);

    for (int i = 0; i < 128; i++) {
        e1000_write(driver, E1000_REG_MTA + (i * 4), 0);
    }
    dbgprint("e1000: MTA: %x\n", e1000_read(driver, E1000_REG_MTA));

    e1000_receive_descriptor *buffer = (e1000_receive_descriptor *)calloc_align(RX_LEN, sizeof(e1000_receive_descriptor), 16);
    dbgprint("e1000: RX buffer: %x\n", buffer);
    for (int i = 0; i < RX_LEN; i++) {
        buffer[i].buffer_address = (uint64_t)mmu_get_physical_address((uintptr_t)malloc(8192 + 16));
        buffer[i].status = 0;
        dbgprint("e1000: RX buffer %d: %x\n", i, buffer[i].buffer_address);
    }
    driver->rx_buffer = (uint8_t *)buffer;
    driver->rx_buffer_size = RX_LEN * sizeof(e1000_transmit_descriptor);
    e1000_write(driver, E1000_REG_RDBAL, (uint32_t)mmu_get_physical_address((uintptr_t)buffer));
    e1000_write(driver, E1000_REG_RDBAH, (uint32_t)((uint64_t)mmu_get_physical_address((uintptr_t)buffer) >> 32));

    e1000_write(driver, E1000_REG_RDLEN, driver->rx_buffer_size);

    e1000_write(driver, E1000_REG_RDH, 0);
    e1000_write(driver, E1000_REG_RDT, driver->rx_buffer_size / sizeof(e1000_receive_descriptor) - 1);
    driver->rx_tail = 0;

    e1000_write(driver, E1000_REG_RCTL, E1000_REGBIT_RCTL_EN | E1000_REGBIT_RCTL_SBP | E1000_REGBIT_RCTL_UPE | E1000_REGBIT_RCTL_MPE | E1000_REGBIT_RCTL_BAM | E1000_REGBIT_RCTL_BSIZE_8192 | E1000_REGBIT_RCTL_BSEX | E1000_REGBIT_RCTL_SECRC);
}

static void e1000_transmit_init(ethernet_driver *driver) {
    e1000_transmit_descriptor *buffer = (e1000_transmit_descriptor *)calloc_align(TX_LEN, sizeof(e1000_transmit_descriptor), 16);
    //static e1000_transmit_descriptor buffer[256] __attribute__((aligned(16)));
    for (int i = 0; i < TX_LEN; i++) {
        buffer[i].buffer_address = 0;
        buffer[i].cmd = 0;
        buffer[i].status = E1000_REGBIT_TXD_STAT_DD;
    }
    driver->tx_buffer = (uint8_t *)buffer;
    driver->tx_buffer_size = TX_LEN * sizeof(e1000_transmit_descriptor);
    e1000_write(driver, E1000_REG_TDBAL, (uint32_t)mmu_get_physical_address((uintptr_t)buffer));
    e1000_write(driver, E1000_REG_TDBAH, (uint32_t)((uint64_t)mmu_get_physical_address((uintptr_t)buffer) >> 32));

    e1000_write(driver, E1000_REG_TDLEN, driver->tx_buffer_size);

    e1000_write(driver, E1000_REG_TDH, 0);
    e1000_write(driver, E1000_REG_TDT, 0);
    driver->tx_tail = 0;

    e1000_write(driver, E1000_REG_TCTL, E1000_REGBIT_TCTL_EN | E1000_REGBIT_TCTL_PSP | E1000_REGBIT_TCTL_CT_15 | E1000_REGBIT_TCTL_COLD_FULL | E1000_REGBIT_TCTL_RTLC);

    e1000_write(driver, E1000_REG_TIPG, 10 << E1000_REGBITADDR_TIPG_IPGT | 10 << E1000_REGBITADDR_TIPG_IPGR1 | 10 << E1000_REGBITADDR_TIPG_IPGR2);
}

void e1000_int_enable(ethernet_driver *driver) {
    e1000_write(
        driver,
        E1000_REG_IMS,
        E1000_REGBIT_IMS_TXDW | E1000_REGBIT_ICR_TXQE | E1000_REGBIT_ICR_LSC | E1000_REGBIT_ICR_RXSEQ | E1000_REGBIT_ICR_RXDMT0 | E1000_REGBIT_ICR_RXO | E1000_REGBIT_ICR_RXT0 | E1000_REGBIT_IMS_MDAC | E1000_REGBIT_IMS_RXCFG | E1000_REGBIT_IMS_PHYINT | E1000_REGBIT_IMS_GPI | E1000_REGBIT_IMS_TXD_LOW | E1000_REGBIT_IMS_SRPD
    );

    e1000_read(driver, E1000_REG_ICR);
}

void e1000_int_disable(ethernet_driver *driver) {
    e1000_write(driver, E1000_REG_IMC, 0xFFFFFFFF);
    e1000_write(driver, E1000_REG_ICR, 0xFFFFFFFF);
    e1000_read(driver, E1000_REG_STATUS);
}

static void e1000_read_link_status(ethernet_driver *driver) {
    uint32_t status = e1000_read(driver, E1000_REG_STATUS);

    uint8_t duplex = (status >> 0) & 1;
    uint8_t link_up = (status >> 1) & 1;
    uint8_t speed = (status >> 6) & 3;

    driver->up = link_up != 0;
    driver->duplex = duplex != 0;
    driver->speed = speed == 0 ? 10 : speed == 1 ? 100 : speed == 2 || speed == 3 ? 1000 : 0;

    dbgprint(
        "e1000: Link is %s, %s duplex, %d Mbps\n",
        driver->up ? "up" : "down",
        driver->duplex ? "full" : "half",
        driver->speed
    );
}

static void e1000_reset(ethernet_driver *driver) {
    e1000_write(driver, E1000_REG_RCTL, 0);
    e1000_write(driver, E1000_REG_TCTL, E1000_REGBIT_TCTL_PSP);
    e1000_read(driver, E1000_REG_STATUS);

    uint32_t ctrl = e1000_read(driver, E1000_REG_CTRL);
    ctrl |= E1000_REGBIT_CTRL_RST;
    e1000_write(driver, E1000_REG_CTRL, ctrl);

    do {
        timer_wait(1);
    } while (e1000_read(driver, E1000_REG_CTRL) & E1000_REGBIT_CTRL_RST);
}

ethernet_driver *e1000_init(pci_device *device, uint8_t bus, uint8_t slot, uint8_t func) {
    dbgprint("Initializing e1000 Ethernet controller\n");

    uint32_t iobase = 0;
    for (int i = 0; i < 6; i++) {
        if (!ISSET_BIT(device->base_address[i], 0) && ((device->base_address[i] > 1) & 0x2) == 0x02) {
            i++;
            continue;
        }

        if (ISSET_BIT(device->base_address[i], 0)) {
            iobase = pci_get_bar_address(device->base_address, i);
            dbgprint("e1000: I/O base address found at BAR %d: 0x%x\n", i, iobase);
            break;
        }
    }

    if (iobase == 0) {
        dbgprint("e1000: No I/O base address found\n");
    }

    ethernet_driver *driver = malloc(sizeof(ethernet_driver));
    driver->mmiobase = pci_get_bar_address(device->base_address, 0);
    driver->iobase = iobase;
    driver->ipv4.ip[0] = 0;
    driver->ipv4.ip[1] = 0;
    driver->ipv4.ip[2] = 0;
    driver->ipv4.ip[3] = 0;
    driver->ipv4.netmask[0] = 0;
    driver->ipv4.netmask[1] = 0;
    driver->ipv4.netmask[2] = 0;
    driver->ipv4.netmask[3] = 0;
    driver->ipv4.gateway[0] = 0;
    driver->ipv4.gateway[1] = 0;
    driver->ipv4.gateway[2] = 0;
    driver->ipv4.gateway[3] = 0;
    driver->write = &e1000_send_packet;
    driver->int_handler = &e1000_int_handler;
    driver->int_enable = &e1000_int_enable;

    device->header.command.bus_master = 1;
    device->header.command.memory_space = 1;

    pci_write_word(bus, slot, func, 0x04, *(uint16_t *)&device->header.command);

    //e1000_reset(driver);

    e1000_write(driver, E1000_REG_EECD, E1000_REGBIT_EECD_SK | E1000_REGBIT_EECD_CS | E1000_REGBIT_EECD_DI);

    e1000_read_eeprom_mac(driver);
    dbgprint("e1000: MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", driver->mac[0], driver->mac[1], driver->mac[2], driver->mac[3], driver->mac[4], driver->mac[5]);

    e1000_write(driver, E1000_REG_FCAL, 0);
    e1000_write(driver, E1000_REG_FCAH, 0);
    e1000_write(driver, E1000_REG_FCT, 0);
    e1000_write(driver, E1000_REG_FCTTV, 0);

    // Auto-Speed Detection
    e1000_write(driver, E1000_REG_CTRL, E1000_REGBIT_CTRL_ASDE | E1000_REGBIT_CTRL_SLU);

    e1000_receive_init(driver);
    e1000_transmit_init(driver);

    e1000_write(driver, E1000_REG_RDTR, 0);
    e1000_write(driver, E1000_REG_ITR, 500);

    e1000_read_link_status(driver);

    return driver;
}

static e1000_transmit_descriptor *e1000_write_transmit_descriptor(ethernet_driver *driver, unsigned int ptr, void *addr, size_t packet_size, uint8_t cso, uint8_t cmd, uint8_t status, uint8_t css) {
    dbgprint("e1000: Writing transmit descriptor %d &%x (%db)\n", ptr, addr, packet_size);
    e1000_transmit_descriptor *descriptor = &((e1000_transmit_descriptor *)driver->tx_buffer)[ptr];
    dbgprint("\tDescriptor: %x\n", descriptor);
    descriptor->buffer_address = (uint64_t)addr;
    descriptor->length = packet_size;
    descriptor->cso = cso;
    descriptor->cmd = cmd;
    descriptor->status = status;
    descriptor->css = css;

    return descriptor;
}

unsigned int e1000_send_packet(ethernet_driver *driver, ethernet_packet *packet, size_t data_size) {
    dbgprint("e1000: Sending packet len %d\n", data_size);
    uint32_t tdt = e1000_read(driver, E1000_REG_TDT);
    uint32_t tdh = e1000_read(driver, E1000_REG_TDH);
    dbgprint("e1000: TDT: %d, TDH: %d\n", tdt, tdh);

    e1000_write_transmit_descriptor(driver, driver->tx_tail, (void *) mmu_get_physical_address((uintptr_t)packet), sizeof(ethernet_header), 0, E1000_REGBIT_TXD_CMD_RS, 0, 0);
    driver->tx_tail = (driver->tx_tail + 1) % TX_LEN;
    e1000_transmit_descriptor *_d = e1000_write_transmit_descriptor(driver, driver->tx_tail, (void *)mmu_get_physical_address((uintptr_t)packet->data), data_size, 0, E1000_REGBIT_TXD_CMD_EOP | E1000_REGBIT_TXD_CMD_RS, 0, 0);
    driver->tx_tail = (driver->tx_tail + 1) % TX_LEN;

    interrupts_disable();

    e1000_write(driver, E1000_REG_TDT, driver->tx_tail);

    // Wait for the command to be executed
    while (_d->status == 0) {}

    dbgprint("e1000: Packet sent, status: %x\n", _d->status);

    interrupts_reenable();

    return 0;
}

static bool e1000_read_receive_descriptor(ethernet_driver *driver, unsigned int ptr) {
    e1000_receive_descriptor *descriptor = &((e1000_receive_descriptor *)driver->rx_buffer)[ptr];
    //dbgprint("e1000: Reading packet %d\n", ptr);
    //dbgprint("\tDescriptor: %x\n", descriptor);
    //dbgprint("\tStatus: %x\n", descriptor->status);
    //dbgprint("\tErrors: %x\n", descriptor->errors);
    //dbgprint("\tLength: %d\n", descriptor->length);
    //dbgprint("\tChecksum: %x\n", descriptor->checksum);
    //dbgprint("\tSpecial: %x\n", descriptor->special);
    //dbgprint("\tBuffer: %x\n", descriptor->buffer_address);

    if (descriptor->errors) {
        dbgprint("e1000: Packet has errors (%x)\n", descriptor->errors);
        return true;
    }

    if (ISSET_BIT_INT(descriptor->status, E1000_REGBIT_RXD_STAT_DD)) {
        if (!ISSET_BIT_INT(descriptor->status, E1000_REGBIT_RXD_STAT_EOP)) {
            dbgprint("e1000: Packet not supported\n");
            return true;
        }

        dbgprint("e1000: Packet received\n");
        ethernet_header *header = (ethernet_header *)descriptor->buffer_address;
        dbgprint("\tDestination: %02x:%02x:%02x:%02x:%02x:%02x\n", header->destination_mac[0], header->destination_mac[1], header->destination_mac[2], header->destination_mac[3], header->destination_mac[4], header->destination_mac[5]);
        dbgprint("\tSource: %02x:%02x:%02x:%02x:%02x:%02x\n", header->source_mac[0], header->source_mac[1], header->source_mac[2], header->source_mac[3], header->source_mac[4], header->source_mac[5]);
        dbgprint("\tType: %x\n", header->ethertype);

        descriptor->status = 0;

        ethernet_packet packet;
        packet.header = *header;
        packet.data = (void *)(descriptor->buffer_address + sizeof(ethernet_header));

        ethernet_process_packet(driver, &packet, descriptor->length - sizeof(ethernet_header));

        return true;
    }

    return false;
}

static void e1000_read_packet(ethernet_driver *driver) {
    while (e1000_read_receive_descriptor(driver, driver->rx_tail)) {
        driver->rx_tail = (driver->rx_tail + 1) % (driver->rx_buffer_size / sizeof(e1000_receive_descriptor));
        e1000_write(driver, E1000_REG_RDT, driver->rx_tail);
    }
}

void e1000_int_handler(ethernet_driver *driver) {
    uint32_t icr = e1000_read(driver, E1000_REG_ICR);
    dbgprint("e1000 int_handler: %x\n", icr);

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_TXDW)) {
        dbgprint("e1000: Transmit done\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_TXQE)) {
        dbgprint("e1000: Transmit queue empty\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_LSC)) {
        dbgprint("e1000: Link status changed\n");
        e1000_read_link_status(driver);
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_RXSEQ)) {
        dbgprint("e1000: Receive sequence error\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_RXDMT0)) {
        dbgprint("e1000: Receive descriptor minimum threshold\n");
        e1000_read_packet(driver);
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_RXO)) {
        dbgprint("e1000: Receive overrun\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_RXT0)) {
        dbgprint("e1000: Receive done\n");
        e1000_read_packet(driver);
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_MDAC)) {
        dbgprint("e1000: MDIO access complete\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_RXCFG)) {
        dbgprint("e1000: Receive config\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_PHYINT)) {
        dbgprint("e1000: PHY interrupt\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_GPI)) {
        dbgprint("e1000: GPI\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_TXD_LOW)) {
        dbgprint("e1000: Transmit descriptor low\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_SRPD)) {
        dbgprint("e1000: Small receive packet detected\n");
    }

    e1000_read(driver, E1000_REG_ICR);
}
