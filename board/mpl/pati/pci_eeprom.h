#ifndef __PCI_EEPROM_H_
#define __PCI_EEPROM_H_	1

#include "pati.h"
/******************************************************************************
 * Eeprom Support
 ******************************************************************************/
/**********************************************
*               Definitions
**********************************************/
#define EE46_CMD_LEN    9       /* Bits in instructions */
#define EE56_CMD_LEN    11      /* Bits in instructions */
#define EE66_CMD_LEN    11      /* Bits in instructions */
#define EE_READ         0x0180  /* 01 1000 0000 read instruction */
#define EE_WRITE        0x0140  /* 01 0100 0000 write instruction */
#define EE_WREN         0x0130  /* 01 0011 0000 write enable instruction */
#define EE_WRALL        0x0110  /* 01 0001 0000 write all registers */
#define EE_PRREAD       0x0180  /* 01 1000 0000 read address stored in Protect Register */
#define EE_PRWRITE      0x0140  /* 01 0100 0000 write the address into PR */
#define EE_WDS          0x0100  /* 01 0000 0000 write disable instruction */
#define EE_PREN         0x0130  /* 01 0011 0000 protect enable instruction */
#define EE_PRCLEAR      0x01FF  /* 01 1111 1111 clear protect register instr */
#define EE_PRDS         0x0100  /* 01 0000 0000 ONE TIME ONLY, permenant */

/***************************************************
 * EEPROM
 ***************************************************/
#define LOW_WORD(x)	(((x) & 0xFFFF))
#define HIGH_WORD(x)	(((x) >> 16) & 0xFFFF)

typedef struct pci_eeprom_t {
	unsigned short offset;
	unsigned short value;
} pci_eeprom;

static pci_eeprom pati_eeprom[] = {
	{ 0x00,PCI_DEVICE_ID_PATI },	/* PCI Device ID PCIIDR[31:16] */
	{ 0x02,PCI_VENDOR_ID_MPL },	/* PCI Vendor ID PCIIDR[15:0] */
	{ 0x04,PCI_CLASS_PROCESSOR_POWERPC },	/* PCI Class Code PCICCR[23:8] */
	{ 0x06,0x00BA },	/* PCI Class Code / PCI Revision ID PCICCR[7:0] / PCIREV[7:0] */
	{ 0x08,0x0007 },	/* PCI Maximum Latency / PCI Minimum Grant PCIMLR[7:0] / PCIMGR[7:0] */
	{ 0x0A,0x0100 },	/* PCI Interrupt Pin / PCI Interrupt Line PCIIPR[7:0] / PCIILR[7:0] */
	{ 0x0C,0x0000 },	/* MSW of Mailbox 0 (User Defined) PCI9056_MAILBOX0[31:16] */
	{ 0x0E,0x0000 },	/* LSW of Mailbox 0 (User Defined) PCI9056_MAILBOX0[15:0] */
	{ 0x10,0x0000 },	/* MSW of Mailbox 1 (User Defined) PCI9056_MAILBOX1[31:16] */
	{ 0x12,0x0000 },	/* LSW of Mailbox 1 (User Defined) PCI9056_MAILBOX1[15:0] */
	{ 0x14,HIGH_WORD(PATI_LOC_CFG_MASK) },	/* MSW of Direct Slave Local Address Space 0 Range LAS0RR[31:16] */
	{ 0x16,LOW_WORD(PATI_LOC_CFG_MASK) },	/* LSW of Direct Slave Local Address Space 0 Range LAS0RR[15:0] */
	{ 0x18,HIGH_WORD(PATI_LOC_CFG_ADDR) },	/* MSW of Direct Slave Local Address Space 0 Local Base Address (Remap) LAS0BA[31:16] (CFG) */
	{ 0x1A,LOW_WORD(PATI_LOC_CFG_ADDR)|1 },	/* LSW of Direct Slave Local Address Space 0 Local Base Address (Remap) LAS0BA[15:2, 0], Reserved [1] */
	{ 0x1C,0x0000 },	/* MSW of Mode/DMA Arbitration MARBR[31, 29:16] or DMAARB[31, 29:16], Reserved [30] */
	{ 0x1E,0x0000 },	/* LSW of Mode/DMA Arbitration MARBR[15:0] or DMAARB[15:0] */
	{ 0x20,0x0030 },	/* Local Miscellaneous Control 2 / Serial EEPROM WP Addr Boundary LMISC2[5:0], Res[7:6] / PROT_AREA[6:0], Res[7] */
	{ 0x22,0x0510 },	/* Local Miscellaneous Control 1 / Local Bus Big/Little Endian Descriptor LMISC1[7:0] / BIGEND[7:0] */
	{ 0x24,0x0000 },	/* MSW of Direct Slave Expansion ROM Range EROMRR[31:16] */
	{ 0x26,0x0000 },	/* LSW of Direct Slave Expansion ROM Range EROMRR[15:11, 0], Reserved [10:1]  */
	{ 0x28,0x0000 },	/* MSW of Direct Slave Expansion ROM Local Base Address (Remap) and BREQo Control EROMBA[31:16] */
	{ 0x2A,0x0000 },	/* LSW of Direct Slave Expansion ROM Local Base Address (Remap) and BREQo Control EROMBA[15:11, 5:0], Reserved [10:6] */
	{ 0x2C,(0x4243 | HIGH_WORD((PATI_LOC_CFG_SPACE0_ATTR))) },	/* MSW of Local Address Space 0/Expansion ROM Bus Region Descriptor LBRD0[31:16] */
	{ 0x2E,LOW_WORD(PATI_LOC_CFG_SPACE0_ATTR) },	/* LSW of Local Address Space 0/Expansion ROM Bus Region Descriptor LBRD0[15:0] */
	{ 0x30,HIGH_WORD(PATI_DMASTER_MASK) },	/* MSW of Local Range for Direct Master-to-PCI DMRR[31:16] */
	{ 0x32,LOW_WORD(PATI_DMASTER_MASK) },	/* LSW of Local Range for Direct Master-to-PCI (Reserved) DMRR[15:0] */
	{ 0x34,HIGH_WORD(PATI_DMASTER_ADDR) },	/* MSW of Local Base Address for Direct Master-to-PCI Memory DMLBAM[31:16] */
	{ 0x36,LOW_WORD(PATI_DMASTER_ADDR) },	/* LSW of Local Base Address for Direct Master-to-PCI Memory (Reserved) DMLBAM[15:0] */
	{ 0x38,0x0000 },	/* MSW of Local Bus Address for Direct Master-to-PCI I/O Configuration DMLBAI[31:16] */
	{ 0x3A,0x0000 },	/* LSW of Local Bus Address for Direct Master-to-PCI I/O Configuration (Reserved) DMLBAI[15:0] */
	{ 0x3C,0x0000 },	/* MSW of PCI Base Address (Remap) for Direct Master-to-PCI Memory DMPBAM[31:16] */
	{ 0x3E,0x0000 },	/* LSW of PCI Base Address (Remap) for Direct Master-to-PCI Memory DMPBAM[15:0] */
	{ 0x40,0x0000 },	/* MSW of PCI Configuration Address for Direct Master-to-PCI I/O Configuration DMCFGA[31, 23:16] Reserved [30:24]*/
	{ 0x42,0x0000 },	/* LSW of PCI Configuration Address for Direct Master-to-PCI I/O Configuration DMCFGA[15:0] */
	{ 0x44,0x0000 },	/* PCI Subsystem ID PCISID[15:0] */
	{ 0x46,0x0000 },	/* PCI Subsystem Vendor ID PCISVID[15:0] */
	{ 0x48,HIGH_WORD(PATI_LOC_SDRAM_MASK) },	/* MSW of Direct Slave Local Address Space 1 Range (1 MB) LAS1RR[31:16] */
	{ 0x4A,LOW_WORD(PATI_LOC_SDRAM_MASK) },	/* LSW of Direct Slave Local Address Space 1 Range (1 MB) LAS1RR[15:0] */
	{ 0x4C,HIGH_WORD(PATI_LOC_SDRAM_ADDR) },	/* MSW of Direct Slave Local Address Space 1 Local Base Address (Remap) LAS1BA[31:16] (SDRAM) */
	{ 0x4E,LOW_WORD(PATI_LOC_SDRAM_ADDR) | 0x1 },	/* LSW of Direct Slave Local Address Space 1 Local Base Address (Remap) LAS1BA[15:2, 0], Reserved [1] */
	{ 0x50,HIGH_WORD(PATI_LOC_SDRAM_SPACE1_ATTR) },	/* MSW of Local Address Space 1 Bus Region Descriptor LBRD1[31:16] */
	{ 0x52,LOW_WORD(PATI_LOC_SDRAM_SPACE1_ATTR) },	/* LSW of Local Address Space 1 Bus Region Descriptor (Reserved) LBRD1[15:0] */
	{ 0x54,0x0000 },	/* Hot Swap Control/Status (Reserved) Reserved */
	{ 0x56,0x0000 },	/* Hot Swap Next Capability Pointer / Hot Swap Control HS_NEXT[7:0] / HS_CNTL[7:0] */
	{ 0x58,0x0000 },	/* Reserved Reserved */
	{ 0x5A,0x0000 },	/* PCI Arbiter Control PCIARB[3:0], Reserved [15:4] */
	{ 0x5C,0x0000 },	/* Power Management Capabilities PMC[15:9, 2:0] */
	{ 0x5E,0x0000 },	/* Power Management Next Capability Pointer (Reserved) / Power Management Capability ID (Reserved) Reserved*/
	{ 0x60,0x0000 },	/* Power Management Data / PMCSR Bridge Support Extension (Reserved) PMDATA[7:0] / Reserved */
	{ 0x62,0x0000 },	/* Power Management Control/Status PMCSR[14:8] */
	{ 0xFFFF,0xFFFF}	/* terminaror */
};
#define PATI_EEPROM_LAST_OFFSET	0x64
#endif /* #ifndef __PCI_EEPROM_H_ */
