#ifndef ARTICIAS_H
#define ARTICIAS_H

#include "short_types.h"
#include <common.h>

#define REG_GROUP       0xF0

/* ArticiaS registers */
#define GLOBALINFO0     0x50
#define GLOBALINFO1     0x51
#define GLOBALINFO2     0x52
#define GLOBALINFO3     0x53
#define GLOBALCTL0      0x54
#define GLOBALCTL1      0x55
#define NVRAMCTL        0x56
#define PCI1ACR0        0x58
#define PCI1ACR1        0x59
#define PCI1ACR2        0x5a
#define PCI1ACR3        0x5b
#define HBUSACR0        0x5c
#define HBUSACR1        0x5d
#define HBUSACR2        0x5e
#define HBUSACR3        0x5f
#define HOSTINT0        0x68
#define HOSTINT1        0x69
#define HOSTINT2        0x6a
#define HOSTINT3        0x6b
#define HOSTRBCR        0x70
#define XDBCR           0x74

#define LBSBCR2         0xd2


/* Memory controller */

#define DIMM0_B0_SCR0   0x90
#define DIMM0_B1_SCR0   0x94
#define DIMM1_B2_SCR0   0x98
#define DIMM1_B3_SCR0   0x9c
#define DIMM2_B4_SCR0   0xa0
#define DIMM2_B5_SCR0   0xa4
#define DIMM3_B6_SCR0   0xa8
#define DIMM3_B7_SCR0   0xac

#define DIMM0_TCR0      0xb0
#define DIMM1_TCR0      0xb2
#define DIMM2_TCR0      0xb4
#define DIMM3_TCR0      0xb6

#define DRAM_REFRESH0   0xb8
#define DRAM_GCR0       0xc0
#define DRAM_PCR0       0xc6
#define DRAM_ECC0       0xc4
#define SRAM_CR         0xc8
#define DRAM_RAS_CTL0   0xcc
#define DRAM_RAS_CTL1   0xcd

/* Bits for REG_GROUP */
#define REG_GROUP_MULTI       (1<<1)
#define REG_GROUP_SPECIAL     (1<<3)
#define REG_GROUP_DIAG        (0x1<<4)
#define REG_GROUP_POWER       (0x2<<4)


#define GLOBALINFO0_BO        (1<<7)


#define GLOBALINFO2_B1ARBITER (1<<6)


#define HBUSACR0_CPUAPC       (1<<0)
#define HBUSACR0_NUMREQ_2     (0<<1)
#define HBUSACR0_NUMREQ_3     (1<<1)
#define HBUSACR0_NUMREQ_4     (2<<1)
#define HBUSACR0_NUMREQ_MASK  (7<<1)
#define HBUSACR0_RAW          (1<<6)
#define HBUSACR0_WAIT         (1<<7)
#define HBUSACR0_RESERVED     (0x30)


#define HBUSACR2_BURST        (1<<0)
#define HBUSACR2_LAT          (1<<1)


#define HBUSACR3_LMWC_SM      (1<<0)
#define HBUSACR3_LMWC_PCI1    (1<<1)
#define HBUSACR3_LMWC_PCI0    (1<<2)
#define HBUSACR3_PMWC_PCI1    (1<<3)
#define HBUSACR3_PMWC_PCI0    (1<<4)
#define HBUSACR3_FKH          (1<<5)
#define HBUSACR3_92H_EN       (1<<6)
#define HBUSACR3_60H_64H_EN   (1<<7)


#define HOSTRBCR_PREFETCH     (1<<4)


#define XDBCR_HWTOXD          (1<<0)
#define XDBCR_KBTOXD          (1<<1)
#define XDBCR_RTCTOXD         (1<<2)
#define XDBCR_SCALE_1_1       (0x0<<3)
#define XDBCR_SCALE_2_2       (0x1<<3)
#define XDBCR_SCALE_3_2       (0x2<<3)
#define XDBCR_SCALE_4_4       (0x3<<3)
#define XDBCR_SCALE_5_8       (0x4<<3)
#define XDBCR_SCALE_6_8       (0x5<<3)
#define XDBCR_SCALE_8_8       (0x6<<3)
#define XDBCR_SCALE_0_16      (0x7<<3)
#define XDBCR_XDPROM          (1<<7)


#define LBSBCR2_1_RWAC        (1<<2)


/* PCI controller */
#define ARTICIAS_PCI_CFGADDR  0xfec00cf8
#define ARTICIAS_PCI_CFGDATA  0xfee00cfc

#define ARTICIAS_PCI_BUS       0x80000000
#define ARTICIAS_PCI_MAXSIZE   0x7cffffff
#define ARTICIAS_PCI_PHYS      0x80000000

#define ARTICIAS_SYS_BUS       0x00000000
#define ARTICIAS_SYS_MAXSIZE   0x7fffffff
#define ARTICIAS_SYS_PHYS      0x00000000

#define ARTICIAS_PCIIO_BUS     0x00800000
#define ARTICIAS_PCIIO_MAXSIZE 0x003fffff
#define ARTICIAS_PCIIO_PHYS    0xfe800000

#define ARTICIAS_ISAIO_BUS     0x00002000
#define ARTICIAS_ISAIO_MAXSIZE 0x0000d000
#define ARTICIAS_ISAIO_PHYS    0xfe002000


/* Prototypes */
long articiaS_ram_init(void);
void articiaS_pci_init(void);


#endif
