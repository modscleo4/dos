#ifndef E1000_H
#define E1000_H

#include <stdint.h>
#include "pci.h"
#include "ethernet.h"

typedef struct e1000_receive_descriptor {
    uint64_t buffer_address;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} __attribute__((packed)) e1000_receive_descriptor;

typedef struct e1000_transmit_descriptor {
    uint64_t buffer_address;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} __attribute__((packed)) e1000_transmit_descriptor;

enum E1000GeneralRegisters {
    /** Device Control */
    E1000_REG_CTRL = 0x00000,
    /** Device Status */
    E1000_REG_STATUS = 0x00008,
    /** EEPROM/Flash Control/Data */
    E1000_REG_EECD = 0x00010,
    /** EEPROM Read */
    E1000_REG_EERD = 0x00014,
    /** Extended Device Control */
    E1000_REG_CTRL_EXT = 0x00018,
    /** MDI Control */
    E1000_REG_MDIC = 0x00020,
    /** Flow Control Address Low */
    E1000_REG_FCAL = 0x00028,
    /** Flow Control Address High */
    E1000_REG_FCAH = 0x0002C,
    /** Flow Control Type */
    E1000_REG_FCT = 0x00030,
    /** VLAN EtherType */
    E1000_REG_VET = 0x00038,
    /** Flow Control Transmit Timer Value */
    E1000_REG_FCTTV = 0x00170,
    /** Transmit Configuration Word */
    E1000_REG_TXCW = 0x00178,
    /** Receive Configuration Word */
    E1000_REG_RXCW = 0x00180,
    /** LED Control */
    E1000_REG_LEDCTL = 0x00E00,
};

enum E1000DMARegisters {
    /** Packed Buffer Allocation */
    E1000_REG_PBA = 0x01000,
};

enum E1000InterruptRegisters {
    /** Interrupt Cause Read */
    E1000_REG_ICR = 0x000C0,
    /** Interrupt Throttling */
    E1000_REG_ITR = 0x000C4,
    /** Interrupt Cause Set */
    E1000_REG_ICS = 0x000C8,
    /** Interrupt Mask Set/Read */
    E1000_REG_IMS = 0x000D0,
    /** Interrupt Mask Clear */
    E1000_REG_IMC = 0x000D8,
};

enum E1000ReceiveRegisters {
    /** Receive Control */
    E1000_REG_RCTL = 0x00100,
    /** Flow Control Receive Threshold Low */
    E1000_REG_FCRTL = 0x02160,
    /** Flow Control Receive Threshold High */
    E1000_REG_FCRTH = 0x02168,
    /** Receive Descriptor Base Address Low */
    E1000_REG_RDBAL = 0x02800,
    /** Receive Descriptor Base Address High */
    E1000_REG_RDBAH = 0x02804,
    /** Receive Descriptor Length */
    E1000_REG_RDLEN = 0x02808,
    /** Receive Descriptor Head */
    E1000_REG_RDH = 0x02810,
    /** Receive Descriptor Tail */
    E1000_REG_RDT = 0x02818,
    /** Receive Delay Timer */
    E1000_REG_RDTR = 0x02820,
    /** Receive Interrupt Absolute Delay Timer */
    E1000_REG_RADV = 0x0282C,
    /** Receive Small Packet Detect Interrupt  */
    E1000_REG_RSRPD = 0x02C00,

    /** Multicast Table Array */
    E1000_REG_MTA = 0x05200,
    /** Receive Address Low */
    E1000_REG_RAL = 0x05400,
    /** Receive Address High */
    E1000_REG_RAH = 0x05404,
    /** VLAN Filter Table Array */
    E1000_REG_VFTA = 0x05600,
};

enum E1000TransmitRegisters {
    /** Transmit Control */
    E1000_REG_TCTL = 0x00400,
    /** Transmit IPG */
    E1000_REG_TIPG = 0x00410,
    /** Adaptive IFS Throttle - AIT */
    E1000_REG_AIFS = 0x00458,
    /** Transmit Descriptor Base Low */
    E1000_REG_TDBAL = 0x03800,
    /** Transmit Descriptor Base High */
    E1000_REG_TDBAH = 0x03804,
    /** Transmit Descriptor Length */
    E1000_REG_TDLEN = 0x03808,
    /** Transmit Descriptor Head */
    E1000_REG_TDH = 0x03810,
    /** Transmit Descriptor Tail */
    E1000_REG_TDT = 0x03818,
    /** Transmit Interrupt Delay Value */
    E1000_REG_TIDV = 0x03820,
};

enum E1000TXDMARegisters {
    /** TX DMA Control */
    E1000_REG_TXDMAC = 0x03000,
    /** Transmit Descriptor Control */
    E1000_REG_TXDCTL = 0x03828,
    /** Transmit Absolute Interrupt Delay Timer */
    E1000_REG_TADV = 0x0282C,
    /** TCP Segmentation Pad and Threshold */
    E1000_REG_TSPMT = 0x03830,
};

enum E1000RXDMARegisters {
    /** Receive Descriptor Control */
    E1000_REG_RXDCTL = 0x02828,
    /** Receive Checksum Control */
    E1000_REG_RXCSUM = 0x05000,
};

enum E1000RegisterBitCTRL {
    E1000_REGBIT_CTRL_FD = 1UL << 0UL,
    E1000_REGBIT_CTRL_LRST = 1UL << 3UL,
    E1000_REGBIT_CTRL_ASDE = 1UL << 5UL,
    E1000_REGBIT_CTRL_SLU = 1UL << 6UL,
    E1000_REGBIT_CTRL_ILOS = 1UL << 7UL,
    E1000_REGBIT_CTRL_SPEED_10 = 0,
    E1000_REGBIT_CTRL_SPEED_100 = 1UL << 8UL,
    E1000_REGBIT_CTRL_SPEED_1000 = 1UL << 9UL,
    E1000_REGBIT_CTRL_FRCSPD = 1UL << 11UL,
    E1000_REGBIT_CTRL_FRCDPX = 1UL << 12UL,
    E1000_REGBIT_CTRL_SDP0_DATA = 1UL << 18UL,
    E1000_REGBIT_CTRL_SDP1_DATA = 1UL << 19UL,
    E1000_REGBIT_CTRL_ADVD3WUC = 1UL << 20UL,
    E1000_REGBIT_CTRL_EN_PHY_PWR_MGMT = 1UL << 21UL,
    E1000_REGBIT_CTRL_SDP0_IODIR = 1UL << 22UL,
    E1000_REGBIT_CTRL_SDP1_IODIR = 1UL << 23UL,
    E1000_REGBIT_CTRL_RST = 1UL << 26UL,
    E1000_REGBIT_CTRL_RFCE = 1UL << 27UL,
    E1000_REGBIT_CTRL_TFCE = 1UL << 28UL,
    E1000_REGBIT_CTRL_VME = 1UL << 30UL,
    E1000_REGBIT_CTRL_PHY_RST = 1UL << 31UL,
};

enum E1000RegisterBitEECD {
    E1000_REGBIT_EECD_SK = 1UL << 0UL,
    E1000_REGBIT_EECD_CS = 1UL << 1UL,
    E1000_REGBIT_EECD_DI = 1UL << 2UL,
    E1000_REGBIT_EECD_DO = 1UL << 3UL,
    E1000_REGBIT_EECD_FWE_D = 1UL << 4UL,
    E1000_REGBIT_EECD_FWE_E = 1UL << 5UL,
    E1000_REGBIT_EECD_REQ = 1UL << 6UL,
    E1000_REGBIT_EECD_GNT = 1UL << 7UL,
    E1000_REGBIT_EECD_PRES = 1UL << 8UL,
    E1000_REGBIT_EECD_SIZE = 1UL << 9UL,

    E1000_REGBIT_EECD_TYPE = 1UL << 13UL,
};

enum E1000RegisterBitEERD {
    E1000_REGBIT_EERD_START = 1UL << 0UL,
    E1000_REGBIT_EERD_DONE = 1UL << 4UL,
};

enum E1000RegisterBitRAH {
    E1000_REGBIT_RAH_AS_SOURCE = 1UL << 16UL,
    E1000_REGBIT_RAH_AV = 1UL << 31UL,
};

enum E1000RegisterBitICR {
    E1000_REGBIT_ICR_TXDW = 1UL << 0UL,
    E1000_REGBIT_ICR_TXQE = 1UL << 1UL,
    E1000_REGBIT_ICR_LSC = 1UL << 2UL,
    E1000_REGBIT_ICR_RXSEQ = 1UL << 3UL,
    E1000_REGBIT_ICR_RXDMT0 = 1UL << 4UL,
    E1000_REGBIT_ICR_RXO = 1UL << 6UL,
    E1000_REGBIT_ICR_RXT0 = 1UL << 7UL,
    E1000_REGBIT_ICR_MDAC = 1UL << 9UL,
    E1000_REGBIT_ICR_RXCFG = 1UL << 10UL,
    E1000_REGBIT_ICR_PHYINT = 1UL << 12UL,
    E1000_REGBIT_ICR_GPI = 1UL << 13UL,
    E1000_REGBIT_ICR_TXD_LOW = 1UL << 15UL,
    E1000_REGBIT_ICR_SRPD = 1UL << 16UL,
};

enum E1000RegisterBitIMS {
    E1000_REGBIT_IMS_TXDW = 1UL << 0UL,
    E1000_REGBIT_IMS_TXQE = 1UL << 1UL,
    E1000_REGBIT_IMS_LSC = 1UL << 2UL,
    E1000_REGBIT_IMS_RXSEQ = 1UL << 3UL,
    E1000_REGBIT_IMS_RXDMT0 = 1UL << 4UL,

    E1000_REGBIT_IMS_RXO = 1UL << 6UL,
    E1000_REGBIT_IMS_RXT0 = 1UL << 7UL,

    E1000_REGBIT_IMS_MDAC = 1UL << 9UL,
    E1000_REGBIT_IMS_RXCFG = 1UL << 10UL,

    E1000_REGBIT_IMS_PHYINT = 1UL << 12UL,
    E1000_REGBIT_IMS_GPI = 1UL << 13UL,
    E1000_REGBIT_IMS_TXD_LOW = 1UL << 15UL,
    E1000_REGBIT_IMS_SRPD = 1UL << 16UL,
};

enum E1000RegisterBitRCTL {

    E1000_REGBIT_RCTL_EN = 1UL << 1UL,
    E1000_REGBIT_RCTL_SBP = 1UL << 2UL,
    E1000_REGBIT_RCTL_UPE = 1UL << 3UL,
    E1000_REGBIT_RCTL_MPE = 1UL << 4UL,
    E1000_REGBIT_RCTL_LPE = 1UL << 5UL,
    E1000_REGBIT_RCTL_LBM_NO = 0UL << 6UL,
    E1000_REGBIT_RCTL_LBM_PHY = 3UL << 6UL,
    E1000_REGBIT_RCTL_RDMTS_1_2 = 0UL << 8UL,
    E1000_REGBIT_RCTL_RDMTS_1_4 = 1UL << 8UL,
    E1000_REGBIT_RCTL_RDMTS_1_8 = 1UL << 9UL,

    E1000_REGBIT_RCTL_MO_36 = 0UL << 12UL,
    E1000_REGBIT_RCTL_MO_35 = 1UL << 12UL,
    E1000_REGBIT_RCTL_MO_34 = 1UL << 13UL,
    E1000_REGBIT_RCTL_MO_32 = 3UL << 12UL,

    E1000_REGBIT_RCTL_BAM = 1UL << 15UL,
    E1000_REGBIT_RCTL_BSIZE_2048 = 0UL << 16UL,
    E1000_REGBIT_RCTL_BSIZE_1024 = 1UL << 16UL,
    E1000_REGBIT_RCTL_BSIZE_512 = 2UL << 16UL,
    E1000_REGBIT_RCTL_BSIZE_256 = 3UL << 16UL,
    E1000_REGBIT_RCTL_BSIZE_16384 = 1UL << 16UL,
    E1000_REGBIT_RCTL_BSIZE_8192 = 2UL << 16UL,
    E1000_REGBIT_RCTL_BSIZE_4096 = 3UL << 16UL,
    E1000_REGBIT_RCTL_VFE = 1UL << 18UL,
    E1000_REGBIT_RCTL_CFIEN = 1UL << 19UL,
    E1000_REGBIT_RCTL_CFI = 1UL << 20UL,

    E1000_REGBIT_RCTL_DPF = 1UL << 22UL,
    E1000_REGBIT_RCTL_PMCF = 1UL << 23UL,

    E1000_REGBIT_RCTL_BSEX = 1UL << 25UL,
    E1000_REGBIT_RCTL_SECRC = 1UL << 26UL,

};

enum E1000RegisterBitTCTL {

    E1000_REGBIT_TCTL_EN = 1UL << 1UL,

    E1000_REGBIT_TCTL_PSP = 1UL << 3UL,
    E1000_REGBIT_TCTL_CT_15 = 0x0F << 4,

    E1000_REGBIT_TCTL_COLD_HALF = 0x200UL << 12UL,
    E1000_REGBIT_TCTL_COLD_FULL = 0x40UL << 12UL,
    E1000_REGBIT_TCTL_SWXOFF = 1UL << 22UL,

    E1000_REGBIT_TCTL_RTLC = 1UL << 24UL,
    E1000_REGBIT_TCTL_NRTU = 1UL << 25UL,

};

enum E1000RegisterBitAddressTIPG {
    E1000_REGBITADDR_TIPG_IPGT = 0,
    E1000_REGBITADDR_TIPG_IPGR1 = 10,
    E1000_REGBITADDR_TIPG_IPGR2 = 20,
};

enum E1000RegisterBitTransmissionDescriptorCommand {
    E1000_REGBIT_TXD_CMD_EOP = 1UL << 0UL,
    E1000_REGBIT_TXD_CMD_IFCS = 1UL << 1UL,
    E1000_REGBIT_TXD_CMD_IC = 1UL << 2UL,
    E1000_REGBIT_TXD_CMD_RS = 1UL << 3UL,
    E1000_REGBIT_TXD_CMD_RPS = 1UL << 4UL,
    E1000_REGBIT_TXD_CMD_DEXT = 1UL << 5UL,
    E1000_REGBIT_TXD_CMD_VLE = 1UL << 6UL,
    E1000_REGBIT_TXD_CMD_IDE = 1UL << 7UL,
};

enum E1000RegisterBitTransmissionDescriptorStatus {
    E1000_REGBIT_TXD_STAT_DD = 1UL << 0UL,
    E1000_REGBIT_TXD_STAT_EC = 1UL << 1UL,
    E1000_REGBIT_TXD_STAT_LC = 1UL << 2UL,
    E1000_REGBIT_TXD_STAT_TU = 1UL << 3UL,
};

enum E1000RegisterBitReceiveDescriptorStatus {
    E1000_REGBIT_RXD_STAT_DD = 1UL << 0UL,
    E1000_REGBIT_RXD_STAT_EOP = 1UL << 1UL,
    E1000_REGBIT_RXD_STAT_IXSM = 1UL << 2UL,
    E1000_REGBIT_RXD_STAT_VP = 1UL << 3UL,

    E1000_REGBIT_RXD_STAT_TCPCS = 1UL << 5UL,
    E1000_REGBIT_RXD_STAT_IPCS = 1UL << 6UL,
    E1000_REGBIT_RXD_STAT_PIF = 1UL << 7UL,
};

ethernet_driver *e1000_init(pci_device *device, uint8_t bus, uint8_t slot, uint8_t func);

unsigned int e1000_send_packet(ethernet_driver *driver, ethernet_packet *packet, size_t data_size);

void e1000_int_handler(ethernet_driver *driver);

#endif // E1000_H
