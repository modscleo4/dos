#include "e1000.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "../bits.h"
#include "../debug.h"

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

static uint16_t e1000_read_eeprom(ethernet_driver *driver, unsigned char address) {
    e1000_mmio_write(driver, E1000_REG_EERD, E1000_REGBIT_EERD_START | (address << 8));
    while (!(e1000_mmio_read(driver, E1000_REG_EERD) & E1000_REGBIT_EERD_DONE));
    return (e1000_mmio_read(driver, E1000_REG_EERD) >> 16);
}

static void e1000_lock_eeprom(ethernet_driver *driver) {
    if (e1000_mmio_read(driver, E1000_REG_EECD) & E1000_REGBIT_EECD_REQ) {
        return;
    }

    e1000_mmio_write(driver, E1000_REG_EECD, e1000_mmio_read(driver, E1000_REG_EECD) | E1000_REGBIT_EECD_REQ);
}

static void e1000_unlock_eeprom(ethernet_driver *driver) {
    e1000_mmio_write(driver, E1000_REG_EECD, e1000_mmio_read(driver, E1000_REG_EECD) & ~E1000_REGBIT_EECD_REQ);
}

static void e1000_read_mac(ethernet_driver *driver) {
    e1000_lock_eeprom(driver);
    *(uint16_t *)&driver->mac[0] = e1000_read_eeprom(driver, 0);
    *(uint16_t *)&driver->mac[2] = e1000_read_eeprom(driver, 1);
    *(uint16_t *)&driver->mac[4] = e1000_read_eeprom(driver, 2);
    e1000_unlock_eeprom(driver);
}

static void e1000_receive_init(ethernet_driver *driver) {
    e1000_mmio_write(driver, E1000_REG_RAL, *(uint32_t *)&driver->mac[0]);
    e1000_mmio_write(driver, E1000_REG_RAH, *(uint16_t *)&driver->mac[4] | E1000_REGBIT_RAH_AV);

    for (int i = 0; i < 128; i++) {
        e1000_mmio_write(driver, E1000_REG_MTA + (i * 4), 0);
    }

    e1000_mmio_write(driver, E1000_REG_RDTR, 0);

    static e1000_receive_descriptor buffer[256] __attribute__((aligned(16)));
    for (int i = 0; i < sizeof(buffer) / sizeof(e1000_receive_descriptor); i++) {
        buffer[i].buffer_address = (uint64_t)malloc(8192+16);
        buffer[i].status = 0;
    }
    driver->rx_buffer = (uint8_t *)buffer;
    driver->rx_buffer_size = sizeof(buffer);
    e1000_mmio_write(driver, E1000_REG_RDBAL, (uint32_t)buffer);
    e1000_mmio_write(driver, E1000_REG_RDBAH, (uint32_t)((uint64_t)buffer >> 32));

    e1000_mmio_write(driver, E1000_REG_RDLEN, driver->rx_buffer_size);

    e1000_mmio_write(driver, E1000_REG_RDH, 0);
    e1000_mmio_write(driver, E1000_REG_RDT, driver->rx_buffer_size / sizeof(e1000_receive_descriptor) - 1);
    driver->rx_tail = 0;

    e1000_mmio_write(driver, E1000_REG_RCTL, E1000_REGBIT_RCTL_EN | E1000_REGBIT_RCTL_SBP | E1000_REGBIT_RCTL_UPE | E1000_REGBIT_RCTL_MPE | E1000_REGBIT_RCTL_LBM_NO | E1000_REGBIT_RCTL_RDMTS_1_2 | E1000_REGBIT_RCTL_MO_36 | E1000_REGBIT_RCTL_BAM | E1000_REGBIT_RCTL_BSEX | E1000_REGBIT_RCTL_BSIZE_8192 | E1000_REGBIT_RCTL_SECRC);
}

static void e1000_transmit_init(ethernet_driver *driver) {
    static e1000_transmit_descriptor buffer[256] __attribute__((aligned(16)));
    driver->tx_buffer = (uint8_t *)buffer;
    driver->tx_buffer_size = sizeof(buffer);
    e1000_mmio_write(driver, E1000_REG_TDBAL, (uint32_t)buffer);
    e1000_mmio_write(driver, E1000_REG_TDBAH, (uint32_t)((uint64_t)buffer >> 32));

    e1000_mmio_write(driver, E1000_REG_TDLEN, driver->tx_buffer_size);

    e1000_mmio_write(driver, E1000_REG_TDH, 0);
    e1000_mmio_write(driver, E1000_REG_TDT, 0);
    driver->tx_tail = 0;

    e1000_mmio_write(driver, E1000_REG_TCTL, E1000_REGBIT_TCTL_EN | E1000_REGBIT_TCTL_PSP | E1000_REGBIT_TCTL_CT_10 | E1000_REGBIT_TCTL_COLD_FULL | E1000_REGBIT_TCTL_RTLC);

    e1000_mmio_write(driver, E1000_REG_TIPG, 10 << E1000_REGBITADDR_TIPG_IPGT | 10 << E1000_REGBITADDR_TIPG_IPGR1 | 10 << E1000_REGBITADDR_TIPG_IPGR2);
}

void e1000_int_enable(ethernet_driver *driver) {
    e1000_mmio_write(driver, E1000_REG_IMS, E1000_REGBIT_IMS_RXT0 | E1000_REGBIT_IMS_RXO | E1000_REGBIT_IMS_RXDMT0 | E1000_REGBIT_IMS_RXSEQ | E1000_REGBIT_IMS_LSC | E1000_REGBIT_IMS_TXDW | E1000_REGBIT_IMS_TXQE);
}

ethernet_driver *e1000_init(pci_device *device) {
    dbgprint("Initializing e1000 Ethernet controller\n");

    unsigned int iobase = 0;
    for (int i = 0; i < 6; i++) {
        if (ISSET_BIT(device->base_address[i], 1)) {
            dbgprint("e1000: I/O base address found at 0x%x (BAR %d)\n", device->base_address[i], i);
            iobase = device->base_address[i];
            break;
        }
    }

    if (iobase == 0) {
        dbgprint("e1000: No I/O base address found\n");
    }

    ethernet_driver *driver = malloc(sizeof(ethernet_driver));
    driver->mmiobase = device->base_address[0];
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

    e1000_mmio_write(driver, E1000_REG_EECD, E1000_REGBIT_EECD_SK | E1000_REGBIT_EECD_CS | E1000_REGBIT_EECD_DI);

    e1000_read_mac(driver);

    // Auto-Speed Detection
    e1000_mmio_write(driver, E1000_REG_CTRL, E1000_REGBIT_CTRL_ASDE | E1000_REGBIT_CTRL_SLU);

    e1000_receive_init(driver);
    e1000_transmit_init(driver);

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
    //uint32_t tdt = e1000_mmio_read(driver, E1000_REG_TDT);
    //uint32_t tdh = e1000_mmio_read(driver, E1000_REG_TDH);
    //dbgprint("e1000: TDT: %d, TDH: %d\n", tdt, tdh);

    static uint8_t packet_data[48];
    if (sizeof(ethernet_header) + data_size < 48) {
        dbgprint("e1000: packet too small.\n");
        memcpy(packet_data, packet->data, data_size);
        memset(packet_data + data_size, 0, 48 - data_size);
    }

    e1000_write_transmit_descriptor(driver, driver->tx_tail, packet, sizeof(ethernet_header), 0, E1000_REGBIT_TXD_CMD_RS, 0, 0);
    driver->tx_tail = (driver->tx_tail + 1) % (driver->tx_buffer_size / sizeof(e1000_transmit_descriptor));
    e1000_transmit_descriptor *_d = e1000_write_transmit_descriptor(driver, driver->tx_tail, sizeof(ethernet_header) + data_size < 48 ? packet_data : packet->data, data_size, 0, E1000_REGBIT_TXD_CMD_EOP | E1000_REGBIT_TXD_CMD_IFCS | E1000_REGBIT_TXD_CMD_RS, 0, 0);
    driver->tx_tail = (driver->tx_tail + 1) % (driver->tx_buffer_size / sizeof(e1000_transmit_descriptor));

    e1000_mmio_write(driver, E1000_REG_TDT, driver->tx_tail);

    // Wait for the command to be executed
    while (!ISSET_BIT_INT(_d->status, E1000_REGBIT_TXD_STAT_DD)) {}

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

        //dbgprint("e1000: Packet received\n");
        ethernet_header *header = (ethernet_header *)descriptor->buffer_address;
        //dbgprint("\tDestination: %02x:%02x:%02x:%02x:%02x:%02x\n", header->destination_mac[0], header->destination_mac[1], header->destination_mac[2], header->destination_mac[3], header->destination_mac[4], header->destination_mac[5]);
        //dbgprint("\tSource: %02x:%02x:%02x:%02x:%02x:%02x\n", header->source_mac[0], header->source_mac[1], header->source_mac[2], header->source_mac[3], header->source_mac[4], header->source_mac[5]);
        //dbgprint("\tType: %x\n", header->ethertype);

        descriptor->status = 0;

        static ethernet_packet packet;
        packet.header = *header;
        packet.data = (void *)(descriptor->buffer_address + sizeof(ethernet_header));

        ethernet_process_packet(driver, &packet, descriptor->length - sizeof(ethernet_header));

        return true;
    }

    return false;
}

static void e1000_read_packet(ethernet_driver *driver) {
    unsigned int rdt = e1000_mmio_read(driver, E1000_REG_RDT);
    unsigned int rdh = e1000_mmio_read(driver, E1000_REG_RDH);
    //dbgprint("e1000: RDT: %d, RDH: %d\n", rdt, rdh);

    while (e1000_read_receive_descriptor(driver, driver->rx_tail)) {
        driver->rx_tail = (driver->rx_tail + 1) % (driver->rx_buffer_size / sizeof(e1000_receive_descriptor));
        e1000_mmio_write(driver, E1000_REG_RDT, driver->rx_tail);
    }
}

void e1000_int_handler(ethernet_driver *driver) {
    uint32_t icr = e1000_mmio_read(driver, E1000_REG_ICR);
    //dbgprint("e1000 int_handler: %x\n", icr);

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_TXDW)) {
        //dbgprint("e1000: Transmit done\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_TXQE)) {
        //dbgprint("e1000: Transmit queue empty\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_LSC)) {
        //dbgprint("e1000: Link status changed\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_RXSEQ)) {
        //dbgprint("e1000: Receive sequence error\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_RXDMT0)) {
        //dbgprint("e1000: Receive descriptor minimum threshold\n");
        e1000_read_packet(driver);
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_RXO)) {
        //dbgprint("e1000: Receive overrun\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_RXT0)) {
        //dbgprint("e1000: Receive done\n");
        e1000_read_packet(driver);
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_MDAC)) {
        //dbgprint("e1000: MDIO access complete\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_RXCFG)) {
        //dbgprint("e1000: Receive config\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_PHYINT)) {
        //dbgprint("e1000: PHY interrupt\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_GPI)) {
        //dbgprint("e1000: GPI\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_ECCER)) {
        //dbgprint("e1000: ECC error\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_TS)) {
        //dbgprint("e1000: Timestamp\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_MNG)) {
        //dbgprint("e1000: Manageability\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_DOCK)) {
        //dbgprint("e1000: Dock\n");
    }

    if (ISSET_BIT_INT(icr, E1000_REGBIT_ICR_INT_ASSERTED)) {
        //dbgprint("e1000: Interrupt asserted\n");
    }

    e1000_mmio_read(driver, E1000_REG_ICR);
}
