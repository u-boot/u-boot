/*
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001
 * James Dougherty (jfd@cs.stanford.edu)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * PCI Configuration space access support for MPC824x/MPC107 PCI Bridge
 */
#include <common.h>
#include <mpc824x.h>
#include <pci.h>

#include "mousse.h"

/*
 * Promise ATA/66 support.
 */
#define XFER_PIO_4	0x0C	/* 0000|1100 */
#define XFER_PIO_3	0x0B	/* 0000|1011 */
#define XFER_PIO_2	0x0A	/* 0000|1010 */
#define XFER_PIO_1	0x09	/* 0000|1001 */
#define XFER_PIO_0	0x08	/* 0000|1000 */
#define XFER_PIO_SLOW	0x00	/* 0000|0000 */

/* Promise Regs */
#define REG_A		0x01
#define REG_B		0x02
#define REG_C		0x04
#define REG_D		0x08

void
pdc202xx_decode_registers (unsigned char registers, unsigned char value)
{
	unsigned char	bit = 0, bit1 = 0, bit2 = 0;
	switch(registers) {
		case REG_A:
			bit2 = 0;
			printf("  A Register ");
			if (value & 0x80) printf("SYNC_IN ");
			if (value & 0x40) printf("ERRDY_EN ");
			if (value & 0x20) printf("IORDY_EN ");
			if (value & 0x10) printf("PREFETCH_EN ");
			if (value & 0x08) { printf("PA3 ");bit2 |= 0x08; }
			if (value & 0x04) { printf("PA2 ");bit2 |= 0x04; }
			if (value & 0x02) { printf("PA1 ");bit2 |= 0x02; }
			if (value & 0x01) { printf("PA0 ");bit2 |= 0x01; }
			printf("PIO(A) = %d ", bit2);
			break;
		case REG_B:
			bit1 = 0;bit2 = 0;
			printf("  B Register ");
			if (value & 0x80) { printf("MB2 ");bit1 |= 0x80; }
			if (value & 0x40) { printf("MB1 ");bit1 |= 0x40; }
			if (value & 0x20) { printf("MB0 ");bit1 |= 0x20; }
			printf("DMA(B) = %d ", bit1 >> 5);
			if (value & 0x10) printf("PIO_FORCED/PB4 ");
			if (value & 0x08) { printf("PB3 ");bit2 |= 0x08; }
			if (value & 0x04) { printf("PB2 ");bit2 |= 0x04; }
			if (value & 0x02) { printf("PB1 ");bit2 |= 0x02; }
			if (value & 0x01) { printf("PB0 ");bit2 |= 0x01; }
			printf("PIO(B) = %d ", bit2);
			break;
		case REG_C:
			bit2 = 0;
			printf("  C Register ");
			if (value & 0x80) printf("DMARQp ");
			if (value & 0x40) printf("IORDYp ");
			if (value & 0x20) printf("DMAR_EN ");
			if (value & 0x10) printf("DMAW_EN ");

			if (value & 0x08) { printf("MC3 ");bit2 |= 0x08; }
			if (value & 0x04) { printf("MC2 ");bit2 |= 0x04; }
			if (value & 0x02) { printf("MC1 ");bit2 |= 0x02; }
			if (value & 0x01) { printf("MC0 ");bit2 |= 0x01; }
			printf("DMA(C) = %d ", bit2);
			break;
		case REG_D:
			printf("  D Register ");
			break;
		default:
			return;
	}
	printf("\n        %s ", (registers & REG_D) ? "DP" :
				(registers & REG_C) ? "CP" :
				(registers & REG_B) ? "BP" :
				(registers & REG_A) ? "AP" : "ERROR");
	for (bit=128;bit>0;bit/=2)
		printf("%s", (value & bit) ? "1" : "0");
	printf("\n");
}

/*
 * Promise ATA/66 Support: configure Promise ATA66 card in specified mode.
 */
int
pdc202xx_tune_chipset (pci_dev_t dev, int drive, unsigned char speed)
{
	unsigned short		drive_conf;
	int			err = 0;
	unsigned char			drive_pci, AP, BP, CP, DP;
	unsigned char			TA = 0, TB = 0;

	switch (drive) {
		case 0: drive_pci = 0x60; break;
		case 1: drive_pci = 0x64; break;
		case 2: drive_pci = 0x68; break;
		case 3: drive_pci = 0x6c; break;
		default: return -1;
	}

	pci_read_config_word(dev, drive_pci, &drive_conf);
	pci_read_config_byte(dev, (drive_pci), &AP);
	pci_read_config_byte(dev, (drive_pci)|0x01, &BP);
	pci_read_config_byte(dev, (drive_pci)|0x02, &CP);
	pci_read_config_byte(dev, (drive_pci)|0x03, &DP);

	if ((AP & 0x0F) || (BP & 0x07)) {
	  /* clear PIO modes of lower 8421 bits of A Register */
	  pci_write_config_byte(dev, (drive_pci), AP & ~0x0F);
	  pci_read_config_byte(dev, (drive_pci), &AP);

	  /* clear PIO modes of lower 421 bits of B Register */
	  pci_write_config_byte(dev, (drive_pci)|0x01, BP & ~0x07);
	  pci_read_config_byte(dev, (drive_pci)|0x01, &BP);

	  pci_read_config_byte(dev, (drive_pci), &AP);
	  pci_read_config_byte(dev, (drive_pci)|0x01, &BP);
	}

	pci_read_config_byte(dev, (drive_pci), &AP);
	pci_read_config_byte(dev, (drive_pci)|0x01, &BP);
	pci_read_config_byte(dev, (drive_pci)|0x02, &CP);

	switch(speed) {
		case XFER_PIO_4:	TA = 0x01; TB = 0x04; break;
		case XFER_PIO_3:	TA = 0x02; TB = 0x06; break;
		case XFER_PIO_2:	TA = 0x03; TB = 0x08; break;
		case XFER_PIO_1:	TA = 0x05; TB = 0x0C; break;
		case XFER_PIO_0:
		default:		TA = 0x09; TB = 0x13; break;
	}

	pci_write_config_byte(dev, (drive_pci), AP|TA);
	pci_write_config_byte(dev, (drive_pci)|0x01, BP|TB);

	pci_read_config_byte(dev, (drive_pci), &AP);
	pci_read_config_byte(dev, (drive_pci)|0x01, &BP);
	pci_read_config_byte(dev, (drive_pci)|0x02, &CP);
	pci_read_config_byte(dev, (drive_pci)|0x03, &DP);


#ifdef PDC202XX_DEBUG
	pdc202xx_decode_registers(REG_A, AP);
	pdc202xx_decode_registers(REG_B, BP);
	pdc202xx_decode_registers(REG_C, CP);
	pdc202xx_decode_registers(REG_D, DP);
#endif
	return err;
}
/*
 * Show/Init PCI devices on the specified bus number.
 */

void pci_mousse_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	unsigned int line;

	switch(PCI_DEV(dev)) {
	case 0x0d:
		line = 0x00000101;
		break;

	case 0x0e:
	default:
		line = 0x00000303;
		break;
	}

	pci_write_config_dword(dev, PCI_INTERRUPT_LINE, line);
}

void pci_mousse_setup_pdc202xx(struct pci_controller *hose, pci_dev_t dev,
			       struct pci_config_table *_)
{
	unsigned short vendorId;
	unsigned int mbar0, cmd;
	int bar, a;

	pci_read_config_word(dev, PCI_VENDOR_ID, &vendorId);

	if(vendorId == PCI_VENDOR_ID_PROMISE || vendorId == PCI_VENDOR_ID_CMD){
		/* PDC 202xx card is handled differently, it is a bootable
		 * device and needs all 5 MBAR's configured
		 */
		for(bar = 0; bar < 5; bar++){
			pci_read_config_dword(dev, PCI_BASE_ADDRESS_0+bar*4, &mbar0);
			pci_write_config_dword(dev, PCI_BASE_ADDRESS_0+bar*4, ~0);
			pci_read_config_dword(dev, PCI_BASE_ADDRESS_0+bar*4, &mbar0);
#ifdef DEBUG
			printf("  ATA_bar[%d] = %dbytes\n", bar,
			       ~(mbar0 & PCI_BASE_ADDRESS_MEM_MASK) + 1);
#endif
		}

		/* Program all BAR's */
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, PROMISE_MBAR0);
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_1, PROMISE_MBAR1);
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_2,	PROMISE_MBAR2);
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_3,	PROMISE_MBAR3);
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_4,	PROMISE_MBAR4);
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_5,	PROMISE_MBAR5);

		for(bar = 0; bar < 5; bar++){
			pci_read_config_dword(dev, PCI_BASE_ADDRESS_0+bar*4, &mbar0);
#ifdef DEBUG
			printf("  ATA_bar[%d]@0x%x\n", bar, mbar0);
#endif
		}

		/* Enable ROM Expansion base */
		pci_write_config_dword(dev, PCI_ROM_ADDRESS, PROMISE_MBAR5|1);

		/* Io enable, Memory enable, master enable */
		pci_read_config_dword(dev, PCI_COMMAND, &cmd);
		cmd &= ~0xffff0000;
		cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY | PCI_COMMAND_IO;
		pci_write_config_dword(dev, PCI_COMMAND, cmd);

		/* Breath some life into the controller */
		for( a = 0; a < 4; a++)
			pdc202xx_tune_chipset(dev, a, XFER_PIO_0);
	}
}

static struct pci_config_table pci_sandpoint_config_table[] = {
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x00, 0x0e, 0x00,
	  pci_mousse_setup_pdc202xx },
#ifndef CONFIG_PCI_PNP
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x00, 0x0d, 0x00,
	  pci_cfgfunc_config_device, {PCI_ENET_IOADDR,
				      PCI_ENET_MEMADDR,
				      PCI_COMMAND_MEMORY |
				      PCI_COMMAND_MASTER}},
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	  pci_cfgfunc_config_device, {PCI_SLOT_IOADDR,
				      PCI_SLOT_MEMADDR,
				      PCI_COMMAND_MEMORY |
				      PCI_COMMAND_MASTER}},
#endif
	{ }
};

struct pci_controller hose = {
	config_table: pci_sandpoint_config_table,
	fixup_irq: pci_mousse_fixup_irq,
};

void pci_init_board(void)
{
	pci_mpc824x_init(&hose);
}
