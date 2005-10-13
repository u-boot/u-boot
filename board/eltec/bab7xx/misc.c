/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* includes */
#include <common.h>
#include <linux/ctype.h>
#include <pci.h>
#include <net.h>
#include <mpc106.h>
#include <w83c553f.h>
#include "srom.h"

/* imports  */
extern char console_buffer[CFG_CBSIZE];
extern int l2_cache_enable (int l2control);
extern void *nvram_read (void *dest, const short src, size_t count);
extern void nvram_write (short dest, const void *src, size_t count);

/* globals */
unsigned int ata_reset_time = 60;
unsigned int scsi_reset_time = 10;
unsigned int eltec_board;

/* BAB750 uses SYM53C875(default) and BAB740 uses SYM53C860
 * values fixed after board identification
 */
unsigned short scsi_dev_id = PCI_DEVICE_ID_NCR_53C875;
unsigned int   scsi_max_scsi_id = 15;
unsigned char  scsi_sym53c8xx_ccf = 0x13;

/*----------------------------------------------------------------------------*/
/*
 * handle sroms on BAB740/750
 * fix ether address
 * L2 cache initialization
 * ide dma control
 */
int misc_init_r (void)
{
    revinfo eerev;
    char *ptr;
    u_int  i, l, initSrom, copyNv;
    char buf[256];
    char hex[23] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0,
	     0, 0, 0, 0, 10, 11, 12, 13, 14, 15 };
    pci_dev_t bdf;

    char sromSYM[] = {
#ifdef TULIP_BUG
    /* 10BaseT, 100BaseTx no full duplex modes */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x08,
    0x02, 0x86, 0x02, 0x00, 0xaf, 0x08, 0xa5, 0x00,
    0x88, 0x04, 0x03, 0x27, 0x08, 0x25, 0x00, 0x61,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0xe8
#endif
    /* 10BaseT, 10BaseT-FD, 100BaseTx, 100BaseTx-FD */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x08,
    0x04, 0x86, 0x02, 0x00, 0xaf, 0x08, 0xa5, 0x00,
    0x86, 0x02, 0x04, 0xaf, 0x08, 0xa5, 0x00, 0x88,
    0x04, 0x03, 0x27, 0x08, 0x25, 0x00, 0x61, 0x80,
    0x88, 0x04, 0x05, 0x27, 0x08, 0x25, 0x00, 0x61,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x77
    };

    char sromMII[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x5b, 0x00,
    0x2e, 0x4d, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x08,
    0x01, 0x95, 0x03, 0x00, 0x00, 0x04, 0x01, 0x08,
    0x00, 0x00, 0x02, 0x08, 0x02, 0x00, 0x00, 0x78,
    0xe0, 0x01, 0x00, 0x50, 0x00, 0x18, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0x41
    };

    /*
     * Check/Remake revision info
     */
    initSrom = 0;
    copyNv   = 0;

    /* read out current revision srom contens */
    el_srom_load (0x0000, (u_char*)&eerev, sizeof(revinfo),
		SECOND_DEVICE, FIRST_BLOCK);

    /* read out current nvram shadow image */
    nvram_read (buf, CFG_NV_SROM_COPY_ADDR, CFG_SROM_SIZE);

    if (strcmp (eerev.magic, "ELTEC") != 0)
    {
	/* srom is not initialized -> create a default revision info */
	for (i = 0, ptr = (char *)&eerev; i < sizeof(revinfo); i++)
	    *ptr++ = 0x00;
	strcpy(eerev.magic, "ELTEC");
	eerev.revrev[0] = 1;
	eerev.revrev[1] = 0;
	eerev.size = 0x00E0;
	eerev.category[0] = 0x01;

	/* node id from dead e128 as default */
	eerev.etheraddr[0] = 0x00;
	eerev.etheraddr[1] = 0x00;
	eerev.etheraddr[2] = 0x5B;
	eerev.etheraddr[3] = 0x00;
	eerev.etheraddr[4] = 0x2E;
	eerev.etheraddr[5] = 0x4D;

	/* cache config word for bab750 */
	*(int*)&eerev.res[0] = CLK2P0TO1_1MB_PB_0P5DH;

	initSrom = 1;  /* force dialog */
	copyNv   = 1;  /* copy to nvram */
    }

    if ((copyNv == 0) &&   (el_srom_checksum((u_char*)&eerev, CFG_SROM_SIZE) !=
		el_srom_checksum((u_char*)buf, CFG_SROM_SIZE)))
    {
	printf ("Invalid revision info copy in nvram !\n");
	printf ("Press key:\n  <c> to copy current revision info to nvram.\n");
	printf ("  <r> to reenter revision info.\n");
	printf ("=> ");
	if (0 != readline (NULL))
	{
	    switch ((char)toupper(console_buffer[0]))
	    {
	    case 'C':
		copyNv = 1;
		break;
	    case 'R':
		copyNv = 1;
		initSrom = 1;
		break;
	    }
	}
    }

    if (initSrom)
    {
	memcpy (buf, &eerev.revision[0][0], 14);     /* save all revision info */
	printf ("Enter revision number (0-9): %c  ", eerev.revision[0][0]);
	if (0 != readline (NULL))
	{
	    eerev.revision[0][0] = (char)toupper(console_buffer[0]);
	    memcpy (&eerev.revision[1][0], buf, 12); /* shift rest of rev info */
	}

	printf ("Enter revision character (A-Z): %c  ", eerev.revision[0][1]);
	if (1 == readline (NULL))
	{
	    eerev.revision[0][1] = (char)toupper(console_buffer[0]);
	}

	printf ("Enter board name (V-XXXX-XXXX): %s  ", (char *)&eerev.board);
	if (11 == readline (NULL))
	{
	    for (i=0; i<11; i++)
		eerev.board[i] =  (char)toupper(console_buffer[i]);
	    eerev.board[11] = '\0';
	}

	printf ("Enter serial number: %s ", (char *)&eerev.serial );
	if (6 == readline (NULL))
	{
	    for (i=0; i<6; i++)
		eerev.serial[i] = console_buffer[i];
	    eerev.serial[6] = '\0';
	}

	printf ("Enter ether node ID with leading zero (HEX): %02x%02x%02x%02x%02x%02x  ",
	    eerev.etheraddr[0], eerev.etheraddr[1],
	    eerev.etheraddr[2], eerev.etheraddr[3],
	    eerev.etheraddr[4], eerev.etheraddr[5]);
	if (12 == readline (NULL))
	{
	    for (i=0; i<12; i+=2)
	    eerev.etheraddr[i>>1] = (char)(16*hex[toupper(console_buffer[i])-'0'] +
			       hex[toupper(console_buffer[i+1])-'0']);
	}

	l = strlen ((char *)&eerev.text);
	printf("Add to text section (max 64 chr): %s ", (char *)&eerev.text );
	if (0 != readline (NULL))
	{
	    for (i = l; i<63; i++)
		eerev.text[i] = console_buffer[i-l];
	    eerev.text[63] = '\0';
	}

	if (strstr ((char *)&eerev.board, "75") != NULL)
	    eltec_board = 750;
	else
	    eltec_board = 740;

	if (eltec_board == 750)
	{
	    if (CPU_TYPE == CPU_TYPE_750)
		*(int*)&eerev.res[0] = CLK2P0TO1_1MB_PB_0P5DH;
		else
		*(int*)&eerev.res[0] = CLK2P5TO1_1MB_PB_0P5DH;

	    printf("Enter L2Cache config word with leading zero (HEX): %08X  ",
		    *(int*)&eerev.res[0] );
	    if (0 != readline (NULL))
	    {
		for (i=0; i<7; i+=2)
		{
		    eerev.res[i>>1] =
		    (char)(16*hex[toupper(console_buffer[i])-'0'] +
		    hex[toupper(console_buffer[i+1])-'0']);
		}
	    }

	    /* prepare network eeprom */
	    sromMII[20] = eerev.etheraddr[0];
	    sromMII[21] = eerev.etheraddr[1];
	    sromMII[22] = eerev.etheraddr[2];
	    sromMII[23] = eerev.etheraddr[3];
	    sromMII[24] = eerev.etheraddr[4];
	    sromMII[25] = eerev.etheraddr[5];
	    printf("\nSRom:  Writing DEC21143 MII info .. ");

	    if (dc_srom_store ((u_short *)sromMII) == -1)
		printf("FAILED\n");
	    else
		printf("OK\n");
	}

	if (eltec_board == 740)
	{
	    *(int *)&eerev.res[0] = 0;
	    sromSYM[20] = eerev.etheraddr[0];
	    sromSYM[21] = eerev.etheraddr[1];
	    sromSYM[22] = eerev.etheraddr[2];
	    sromSYM[23] = eerev.etheraddr[3];
	    sromSYM[24] = eerev.etheraddr[4];
	    sromSYM[25] = eerev.etheraddr[5];
	    printf("\nSRom:  Writing DEC21143 SYM info .. ");

	    if (dc_srom_store ((u_short *)sromSYM) == -1)
		printf("FAILED\n");
	    else
		printf("OK\n");
	}

	/* update CRC */
	eerev.crc = el_srom_checksum((u_char *)eerev.board, eerev.size);

	/* write new values */
	printf("\nSRom:  Writing revision info ...... ");
	if (el_srom_store((BLOCK_SIZE-sizeof(revinfo)), (u_char *)&eerev,
			    sizeof(revinfo), SECOND_DEVICE, FIRST_BLOCK) == -1)
	    printf("FAILED\n\n");
	else
	    printf("OK\n\n");

	/* write new values as shadow image to nvram */
	nvram_write (CFG_NV_SROM_COPY_ADDR, (void *)&eerev, CFG_SROM_SIZE);

    } /*if (initSrom) */

    /* copy current values as shadow image to nvram */
    if (initSrom == 0 && copyNv == 1)
	nvram_write (CFG_NV_SROM_COPY_ADDR, (void *)&eerev, CFG_SROM_SIZE);

    /* update environment */
    sprintf (buf, "%02x:%02x:%02x:%02x:%02x:%02x",
	    eerev.etheraddr[0], eerev.etheraddr[1],
	    eerev.etheraddr[2], eerev.etheraddr[3],
	    eerev.etheraddr[4], eerev.etheraddr[5]);
    setenv ("ethaddr", buf);

    /* print actual board identification */
    printf("Ident: %s  Ser %s  Rev %c%c\n",
	    eerev.board, (char *)&eerev.serial,
	    eerev.revision[0][0], eerev.revision[0][1]);

    /* global board ident */
    if (strstr ((char *)&eerev.board, "75") != NULL)
	eltec_board = 750;
    else
	eltec_board = 740;

   /*
    * L2 cache configuration
    */
#if defined(CFG_L2_BAB7xx)
    ptr = getenv("l2cache");
    if (*ptr == '0')
    {
	printf ("Cache: L2 NOT activated on BAB%d\n", eltec_board);
    }
    else
    {
	printf ("Cache: L2 activated on BAB%d\n", eltec_board);
	l2_cache_enable(*(int*)&eerev.res[0]);
    }
#endif

   /*
    * Reconfig ata reset timeout from environment
    */
    if ((ptr = getenv ("ata_reset_time")) != NULL)
    {
	ata_reset_time = (int)simple_strtoul (ptr, NULL, 10);
    }
    else
    {
	sprintf (buf, "%d", ata_reset_time);
	setenv ("ata_reset_time", buf);
    }

   /*
    * Reconfig scsi reset timeout from environment
    */
    if ((ptr = getenv ("scsi_reset_time")) != NULL)
    {
	scsi_reset_time = (int)simple_strtoul (ptr, NULL, 10);
    }
    else
    {
	sprintf (buf, "%d", scsi_reset_time);
	setenv ("scsi_reset_time", buf);
    }


    if ((bdf = pci_find_device(PCI_VENDOR_ID_WINBOND, PCI_DEVICE_ID_WINBOND_83C553, 0)) > 0)
    {
	if (pci_find_device(PCI_VENDOR_ID_NCR, PCI_DEVICE_ID_NCR_53C860, 0) > 0)
	{
	    /* BAB740 with SCSI=IRQ 11; SCC=IRQ 9; no IDE; NCR860 at 80 Mhz */
	    scsi_dev_id = PCI_DEVICE_ID_NCR_53C860;
	    scsi_max_scsi_id = 7;
	    scsi_sym53c8xx_ccf = 0x15;
	    pci_write_config_byte (bdf, WINBOND_IDEIRCR, 0xb0);
	}

	if ((ptr = getenv ("ide_dma_off")) != NULL)
	{
	    u_long dma_off = simple_strtoul (ptr, NULL, 10);
	    /*
	    * setup user defined registers
	    * s.a. linux/drivers/ide/sl82c105.c
	    */
	    bdf |= PCI_BDF(0,0,1);            /* ide user reg at bdf function 1 */
	    if (dma_off & 1)
	    {
		pci_write_config_byte (bdf, 0x46, 1);
		printf("IDE:   DMA off flag set: Bus 0 : Dev 0\n");
	    }
	    if (dma_off & 2)
	    {
		pci_write_config_byte (bdf, 0x4a, 1);
		printf("IDE:   DMA off flag set: Bus 0 : Dev 1\n");
	    }
	    if (dma_off & 4)
	    {
		pci_write_config_byte (bdf, 0x4e, 1);
		printf("IDE:   DMA off flag set: Bus 1 : Dev 0\n");
	    }
	    if (dma_off & 8)
	    {
		pci_write_config_byte (bdf, 0x52, 1);
		printf("IDE:   DMA off flag set: Bus 1 : Dev 1\n");
	    }
	}
    }
    return (0);
}

/*----------------------------------------------------------------------------*/
/*
 * BAB740 uses KENDIN KS8761 modem chip with not common setup values
 */
#ifdef CONFIG_TULIP_SELECT_MEDIA

/* Register bits.
 */
#define BMR_SWR         0x00000001      /* Software Reset */
#define STS_TS          0x00700000      /* Transmit Process State */
#define STS_RS          0x000e0000      /* Receive Process State */
#define OMR_ST          0x00002000      /* Start/Stop Transmission Command */
#define OMR_SR          0x00000002      /* Start/Stop Receive */
#define OMR_PS          0x00040000      /* Port Select */
#define OMR_SDP         0x02000000      /* SD Polarity - MUST BE ASSERTED */
#define OMR_PM          0x00000080      /* Pass All Multicast */
#define OMR_PR          0x00000040      /* Promiscuous Mode */
#define OMR_PCS         0x00800000      /* PCS Function */
#define OMR_TTM         0x00400000      /* Transmit Threshold Mode */

/* Ethernet chip registers.
 */
#define DE4X5_BMR       0x000           /* Bus Mode Register */
#define DE4X5_TPD       0x008           /* Transmit Poll Demand Reg */
#define DE4X5_RRBA      0x018           /* RX Ring Base Address Reg */
#define DE4X5_TRBA      0x020           /* TX Ring Base Address Reg */
#define DE4X5_STS       0x028           /* Status Register */
#define DE4X5_OMR       0x030           /* Operation Mode Register */
#define DE4X5_SISR      0x060           /* SIA Status Register */
#define DE4X5_SICR      0x068           /* SIA Connectivity Register */
#define DE4X5_TXRX      0x070           /* SIA Transmit and Receive Register */
#define DE4X5_GPPR      0x078           /* General Purpose Port register */
#define DE4X5_APROM      0x048          /* Ethernet Address PROM */

/*----------------------------------------------------------------------------*/

static int INL(struct eth_device* dev, u_long addr)
{
    return le32_to_cpu(*(volatile u_long *)(addr + dev->iobase));
}

/*----------------------------------------------------------------------------*/

static void OUTL(struct eth_device* dev, int command, u_long addr)
{
    *(volatile u_long *)(addr + dev->iobase) = cpu_to_le32(command);
}

/*----------------------------------------------------------------------------*/

static void media_reg_init (
    struct eth_device* dev,
    u32 csr14,
    u32 csr15_dir,
    u32 csr15_v0,
    u32 csr15_v1,
    u32 csr6 )
{
    OUTL(dev, 0, DE4X5_OMR);            /* CSR6  */
    udelay(10 * 1000);
    OUTL(dev, 0, DE4X5_SICR);           /* CSR13 */
    OUTL(dev, 1, DE4X5_SICR);           /* CSR13 */
    udelay(10 * 1000);
    OUTL(dev, csr14, DE4X5_TXRX);       /* CSR14 */
    OUTL(dev, csr15_dir, DE4X5_GPPR);   /* CSR15 */
    OUTL(dev, csr15_v0,  DE4X5_GPPR);   /* CSR15 */
    udelay(10 * 1000);
    OUTL(dev, csr15_v1,  DE4X5_GPPR);   /* CSR15 */
    OUTL(dev, 0x00000301, DE4X5_SISR);  /* CSR12 */
    OUTL(dev, csr6, DE4X5_OMR);         /* CSR6 */
}

/*----------------------------------------------------------------------------*/

void dc21x4x_select_media(struct eth_device* dev)
{
    int i, status, ext;
    extern unsigned int eltec_board;

    if (eltec_board == 740)
    {
	printf("SYM media select "); /* BAB740 */
	/* start autoneg. with 10 mbit */
	media_reg_init (dev, 0x3ffff, 0x08af0008, 0x00a10008, 0x00a50008, 0x02400080);
	ext = status = 0;
	for (i=0; i<2000+ext; i++)
	{
	    status = INL(dev, DE4X5_SISR);
	    udelay(1000);
	    if (status & 0x2000) ext = 2000;
	    if ((status & 0x7000) == 0x5000) break;
	}

	/* autoneg. ok -> 100MB FD */
	if ((status & 0x0100f000) == 0x0100d000)
	{
	    media_reg_init (dev, 0x37f7f, 0x08270008, 0x00210008, 0x00250008, 0x03c40280);
	    printf("100baseTx-FD\n");
	}
	/* autoneg. ok -> 100MB HD */
	else if ((status & 0x0080f000) == 0x0080d000)
	{
	    media_reg_init (dev, 0x17f7f, 0x08270008, 0x00210008, 0x00250008, 0x03c40080);
	    printf("100baseTx\n");
	}
	/* autoneg. ok -> 10MB FD */
	else if ((status & 0x0040f000) == 0x0040d000)
	{
	    media_reg_init (dev, 0x07f7f, 0x08af0008, 0x00a10008, 0x00a50008, 0x02400280);
	    printf("10baseT-FD\n");
	}
	/* autoneg. fail -> 10MB HD */
	else
	{
	    media_reg_init (dev, 0x7f7f, 0x08af0008, 0x00a10008, 0x00a50008,
			(OMR_SDP | OMR_TTM | OMR_PM));
	    printf("10baseT\n");
	}
    }
    else
    {
	printf("MII media selected\n");                     /* BAB750 */
	OUTL(dev, OMR_SDP | OMR_PS | OMR_PM, DE4X5_OMR);    /* CSR6 */
    }
}
#endif /* CONFIG_TULIP_SELECT_MEDIA */

/*---------------------------------------------------------------------------*/
