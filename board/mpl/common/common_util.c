/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
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
 *
 */

#include <common.h>
#include <command.h>
#include <video_fb.h>
#include "common_util.h"
#include <asm/processor.h>
#include <i2c.h>
#include <devices.h>
#include <pci.h>

extern int  gunzip (void *, int, unsigned char *, int *);
extern int mem_test(unsigned long start, unsigned long ramsize, int quiet);

#define I2C_BACKUP_ADDR 0x7C00 /* 0x200 bytes for backup */
#define IMAGE_SIZE 0x80000

extern flash_info_t flash_info[];	/* info for FLASH chips */

image_header_t header;



int mpl_prg(unsigned long src,unsigned long size)
{
	unsigned long start;
	flash_info_t *info;
	int i,rc;
	unsigned long *magic = (unsigned long *)src;

	info = &flash_info[0];
  	start = 0 - size;
	for(i=info->sector_count-1;i>0;i--)
	{
		info->protect[i] = 0; /* unprotect this sector */
		if(start>=info->start[i])
		break;
	}
	/* set-up flash location */
	/* now erase flash */
	if(magic[0]!=IH_MAGIC) {
		printf("Bad Magic number\n");
		return -1;
	}
	printf("Erasing at %lx (sector %d) (start %lx)\n",
				start,i,info->start[i]);
	flash_erase (info, i, info->sector_count-1);
	printf("flash erased, programming from 0x%lx 0x%lx Bytes\n",src,size);
	if ((rc = flash_write ((uchar *)src, start, size)) != 0) {
		puts ("ERROR ");
		flash_perror (rc);
		return (1);
	}
	puts ("OK programming done\n");
	return 0;
}


int mpl_prg_image(unsigned long ld_addr)
{
	unsigned long data,len,checksum;
	image_header_t *hdr=&header;
	/* Copy header so we can blank CRC field for re-calculation */
	memcpy (&header, (char *)ld_addr, sizeof(image_header_t));
	if (hdr->ih_magic  != IH_MAGIC) {
		printf ("Bad Magic Number\n");
		return 1;
	}
	print_image_hdr(hdr);
	if (hdr->ih_os  != IH_OS_U_BOOT) {
		printf ("No U-Boot Image\n");
		return 1;
	}
	if (hdr->ih_type  != IH_TYPE_FIRMWARE) {
		printf ("No Firmware Image\n");
		return 1;
	}
	data = (ulong)&header;
	len  = sizeof(image_header_t);
	checksum = hdr->ih_hcrc;
	hdr->ih_hcrc = 0;
	if (crc32 (0, (char *)data, len) != checksum) {
		printf ("Bad Header Checksum\n");
		return 1;
	}
	data = ld_addr + sizeof(image_header_t);
	len  = hdr->ih_size;
	printf ("Verifying Checksum ... ");
	if (crc32 (0, (char *)data, len) != hdr->ih_dcrc) {
		printf ("Bad Data CRC\n");
		return 1;
	}
	switch (hdr->ih_comp) {
	case IH_COMP_NONE:
		break;
	case IH_COMP_GZIP:
		printf ("  Uncompressing  ... ");
		if (gunzip ((void *)(data+0x100000), 0x400000,
			    (uchar *)data, (int *)&len) != 0) {
			printf ("GUNZIP ERROR\n");
			return 1;
		}
		data+=0x100000;
		break;
	default:
		printf ("   Unimplemented compression type %d\n", hdr->ih_comp);
		return 1;
	}

	printf ("  OK\n");
	return(mpl_prg(data,len));
}


void get_backup_values(backup_t *buf)
{
	i2c_read(CFG_DEF_EEPROM_ADDR, I2C_BACKUP_ADDR,2,(void *)buf,sizeof(backup_t));
}

void set_backup_values(int overwrite)
{
	backup_t back;
	int i;

	get_backup_values(&back);
	if(!overwrite) {
		if(strncmp(back.signature,"MPL\0",4)==0) {
			printf("Not possible to write Backup\n");
			return;
		}
	}
	memcpy(back.signature,"MPL\0",4);
	i=getenv_r("serial#",back.serial_name,16);
	if(i==0) {
		printf("Not possible to write Backup\n");
		return;
	}
	back.serial_name[16]=0;
	i=getenv_r("ethaddr",back.eth_addr,20);
	if(i==0) {
		printf("Not possible to write Backup\n");
		return;
	}
	back.eth_addr[20]=0;
	i2c_write(CFG_DEF_EEPROM_ADDR, I2C_BACKUP_ADDR,2,(void *)&back,sizeof(backup_t));
}

void clear_env_values(void)
{
	backup_t back;
	unsigned char env_crc[4];

	memset(&back,0xff,sizeof(backup_t));
	memset(env_crc,0x00,4);
	i2c_write(CFG_DEF_EEPROM_ADDR,I2C_BACKUP_ADDR,2,(void *)&back,sizeof(backup_t));
	i2c_write(CFG_DEF_EEPROM_ADDR,CFG_ENV_OFFSET,2,(void *)env_crc,4);
}

/*
 * check crc of "older" environment
 */
int check_env_old_size(ulong oldsize)
{
	ulong crc, len, new;
	unsigned off;
	uchar buf[64];

	/* read old CRC */
	eeprom_read (CFG_DEF_EEPROM_ADDR,
		     CFG_ENV_OFFSET,
		     (uchar *)&crc, sizeof(ulong));

	new = 0;
	len = oldsize;
	off = sizeof(long);
	len = oldsize-off;
	while (len > 0) {
		int n = (len > sizeof(buf)) ? sizeof(buf) : len;

		eeprom_read (CFG_DEF_EEPROM_ADDR, CFG_ENV_OFFSET+off, buf, n);
		new = crc32 (new, buf, n);
		len -= n;
		off += n;
	}

	return (crc == new);
}

static ulong oldsizes[] = {
	0x200,
	0x800,
	0
};

void copy_old_env(ulong size)
{
	uchar name_buf[64];
	uchar value_buf[0x800];
	uchar c;
	ulong len;
	unsigned off;
	uchar *name, *value;

	name=&name_buf[0];
	value=&value_buf[0];
	len=size;
	off = sizeof(long);
	while (len > off) {
		eeprom_read (CFG_DEF_EEPROM_ADDR, CFG_ENV_OFFSET+off, &c, 1);
		if(c != '=') {
			*name++=c;
			off++;
		}
		else {
			*name++='\0';
			off++;
			do {
				eeprom_read (CFG_DEF_EEPROM_ADDR, CFG_ENV_OFFSET+off, &c, 1);
				*value++=c;
				off++;
				if(c == '\0')
					break;
			} while(len > off);
			name=&name_buf[0];
			value=&value_buf[0];
			if(strncmp(name,"baudrate",8)!=0) {
				setenv(name,value);
			}

		}
	}
}


void check_env(void)
{
	unsigned char *s;
	int i=0;
	char buf[32];
	backup_t back;

	s=getenv("serial#");
	if(!s) {
		while(oldsizes[i]) {
			if(check_env_old_size(oldsizes[i]))
				break;
			i++;
		}
		if(!oldsizes[i]) {
			/* no old environment has been found */
			get_backup_values (&back);
			if (strncmp (back.signature, "MPL\0", 4) == 0) {
				sprintf (buf, "%s", back.serial_name);
				setenv ("serial#", buf);
				sprintf (buf, "%s", back.eth_addr);
				setenv ("ethaddr", buf);
				printf ("INFO:  serial# and ethaddr recovered, use saveenv\n");
				return;
			}
		}
		else {
			copy_old_env(oldsizes[i]);
			printf ("INFO:  old environment ajusted, use saveenv\n");
		}
	}
	else {
		/* check if back up is set */
		get_backup_values(&back);
		if(strncmp(back.signature,"MPL\0",4)!=0) {
			set_backup_values(0);
		}
	}
}



extern device_t *stdio_devices[];
extern char *stdio_names[];

void show_stdio_dev(void)
{
	/* Print informations */
	printf ("In:    ");
	if (stdio_devices[stdin] == NULL) {
		printf ("No input devices available!\n");
	} else {
		printf ("%s\n", stdio_devices[stdin]->name);
	}

	printf ("Out:   ");
	if (stdio_devices[stdout] == NULL) {
		printf ("No output devices available!\n");
	} else {
		printf ("%s\n", stdio_devices[stdout]->name);
	}

	printf ("Err:   ");
	if (stdio_devices[stderr] == NULL) {
		printf ("No error devices available!\n");
	} else {
		printf ("%s\n", stdio_devices[stderr]->name);
	}
}

/* ------------------------------------------------------------------------- */

	/* switches the cs0 and the cs1 to the locations.
	   When boot is TRUE, the the mapping is switched
	   to the boot configuration, If it is FALSE, the
	   flash will be switched in the boot area */

#undef SW_CS_DBG
#ifdef SW_CS_DBG
#define	SW_CS_PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define SW_CS_PRINTF(fmt,args...)
#endif


int switch_cs(unsigned char boot)
{
  	unsigned long pbcr;
	mtdcr(ebccfga, pb0cr); /* get cs0 config reg */
	pbcr = mfdcr(ebccfgd);
	if((pbcr&0x00002000)==0) {
		/* we need only to switch if boot from MPS */
		/*printf(" MPS boot mode detected. ");*/
		/* printf("cs0 cfg: %lx\n",pbcr); */
		if(boot) {
			/* switch to boot configuration */
			/* this is a 8bit boot, switch cs0 to flash location */
			SW_CS_PRINTF("switch to boot mode (MPS on High address\n");
			pbcr&=0x000FFFFF; /*mask base address of the cs0 */
			pbcr|=(FLASH_BASE0_PRELIM & 0xFFF00000);
			mtdcr(ebccfga, pb0cr);
			mtdcr(ebccfgd, pbcr);
			SW_CS_PRINTF("  new cs0 cfg: %lx\n",pbcr);
			mtdcr(ebccfga, pb1cr); /* get cs1 config reg (flash) */
			pbcr = mfdcr(ebccfgd);
			SW_CS_PRINTF(" old cs1 cfg: %lx\n",pbcr);
			pbcr&=0x000FFFFF; /*mask base address of the cs1 */
			pbcr|=(MULTI_PURPOSE_SOCKET_ADDR & 0xFFF00000);
			mtdcr(ebccfga, pb1cr);
			mtdcr(ebccfgd, pbcr);
			SW_CS_PRINTF("  new cs1 cfg: %lx, MPS is on High Address\n",pbcr);
		}
		else
		{
			/* map flash to boot area, */
			SW_CS_PRINTF("map Flash to boot area\n");
			pbcr&=0x000FFFFF; /*mask base address of the cs0 */
			pbcr|=(MULTI_PURPOSE_SOCKET_ADDR & 0xFFF00000);
			mtdcr(ebccfga, pb0cr);
			mtdcr(ebccfgd, pbcr);
			SW_CS_PRINTF("  new cs0 cfg: %lx\n",pbcr);
			mtdcr(ebccfga, pb1cr); /* get cs1 config reg (flash) */
			pbcr = mfdcr(ebccfgd);
			SW_CS_PRINTF("  cs1 cfg: %lx\n",pbcr);
			pbcr&=0x000FFFFF; /*mask base address of the cs1 */
			pbcr|=(FLASH_BASE0_PRELIM & 0xFFF00000);
			mtdcr(ebccfga, pb1cr);
			mtdcr(ebccfgd, pbcr);
			SW_CS_PRINTF("  new cs1 cfg: %lx Flash is on High Address\n",pbcr);
		}
		return 1;
	}
	else {
		SW_CS_PRINTF("Normal boot, no switching necessary\n");
		return 0;
	}
}


int do_mplcommon(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
 	ulong size,src,ld_addr;
	int result;
	backup_t back;
	char sw;
	src = MULTI_PURPOSE_SOCKET_ADDR;
	size = IMAGE_SIZE;

	if (strcmp(argv[1], "flash") == 0)
	{
		sw = switch_cs(0); /* Switch flash to normal location */
#if (CONFIG_COMMANDS & CFG_CMD_FDC)
		if (strcmp(argv[2], "floppy") == 0) {
 			char *local_args[3];
			extern int do_fdcboot (cmd_tbl_t *, int, int, char *[]);
			printf ("\nupdating bootloader image from floppy\n");
			local_args[0] = argv[0];
	    		if(argc==4) {
				local_args[1] = argv[3];
				local_args[2] = NULL;
				ld_addr=simple_strtoul(argv[3], NULL, 16);
				result=do_fdcboot(cmdtp, 0, 2, local_args);
			}
			else {
				local_args[1] = NULL;
				ld_addr=CFG_LOAD_ADDR;
				result=do_fdcboot(cmdtp, 0, 1, local_args);
			}
			result=mpl_prg_image(ld_addr);
			switch_cs(sw); /* Switch flash back */
			return result;
		}
#endif /* (CONFIG_COMMANDS & CFG_CMD_FDC) */
		if (strcmp(argv[2], "mem") == 0) {
	    		if(argc==4) {
				ld_addr=simple_strtoul(argv[3], NULL, 16);
			}
			else {
				ld_addr=load_addr;
			}
			printf ("\nupdating bootloader image from memory at %lX\n",ld_addr);
			result=mpl_prg_image(ld_addr);
			switch_cs(sw); /* Switch flash back */
			return result;
		}
		if (strcmp(argv[2], "mps") == 0) {
			printf ("\nupdating bootloader image from MSP\n");
			result=mpl_prg(src,size);
			switch_cs(sw); /* Switch flash back */
			return result;
		}
		switch_cs(sw); /* Switch flash back */

	}
	if (strcmp(argv[1], "mem") == 0)
	{
		result=0;
		if(argc==3)
		{
			result = (int)simple_strtol(argv[2], NULL, 16);
	    }
	    src=(unsigned long)&result;
	    src-=CFG_MEMTEST_START;
	    src-=(100*1024); /* - 100k */
	    src&=0xfff00000;
	    size=0;
	    do {
	    	size++;
			printf("\n\nPass %ld\n",size);
			mem_test(CFG_MEMTEST_START,src,1);
			if(ctrlc())
				break;
			if(result>0)
				result--;

		}while(result);
		return 0;
	}
	if (strcmp(argv[1], "clearenvvalues") == 0)
	{
 		if (strcmp(argv[2], "yes") == 0)
		{
			clear_env_values();
			return 0;
		}
	}
	if (strcmp(argv[1], "getback") == 0) {
		get_backup_values(&back);
		back.signature[3]=0;
		back.serial_name[16]=0;
		back.eth_addr[20]=0;
		printf("GetBackUp: signature: %s\n",back.signature);
		printf("           serial#:   %s\n",back.serial_name);
		printf("           ethaddr:   %s\n",back.eth_addr);
		return 0;
	}
	if (strcmp(argv[1], "setback") == 0) {
		set_backup_values(1);
		return 0;
	}
	printf("Usage:\n%s\n", cmdtp->usage);
	return 1;
}


#if (CONFIG_COMMANDS & CFG_CMD_DOC)
extern void doc_probe(ulong physadr);
void doc_init (void)
{
  doc_probe(MULTI_PURPOSE_SOCKET_ADDR);
}
#endif


#ifdef CONFIG_VIDEO
/******************************************************
 * Routines to display the Board information
 * to the screen (since the VGA will be initialized as last,
 * we must resend the infos)
 */

#ifdef CONFIG_CONSOLE_EXTRA_INFO
extern GraphicDevice ctfb;

void video_get_info_str (int line_number, char *info)
{
	/* init video info strings for graphic console */
	DECLARE_GLOBAL_DATA_PTR;
	PPC405_SYS_INFO sys_info;
	char rev;
	int i;
	unsigned long pvr;
	char buf[64];
	char tmp[16];
	unsigned char *s, *e, bc, sw;
	switch (line_number)
	{
	case 2:
		/* CPU and board infos */
		pvr=get_pvr();
		get_sys_info (&sys_info);
		switch (pvr) {
			case PVR_405GP_RB: rev='B'; break;
			case PVR_405GP_RC: rev='C'; break;
			case PVR_405GP_RD: rev='D'; break;
			case PVR_405GP_RE: rev='E'; break;
			default:           rev='?'; break;
		}
		/* Board info */
		i=0;
		s=getenv ("serial#");
#ifdef CONFIG_PIP405
		if (!s || strncmp (s, "PIP405", 6)) {
			sprintf(buf,"### No HW ID - assuming PIP405");
		}
#endif
#ifdef CONFIG_MIP405
		if (!s || strncmp (s, "MIP405", 6)) {
			sprintf(buf,"### No HW ID - assuming MIP405");
		}
#endif
		else {
			for (e = s; *e; ++e) {
				if (*e == ' ')
					break;
			}
			for (; s < e; ++s) {
				if (*s == '_') {
					++s;
					break;
				}
				buf[i++]=*s;
			}
			sprintf(&buf[i]," SN ");
			i+=4;
			for (; s < e; ++s) {
				buf[i++]=*s;
			}
			buf[i++]=0;
		}
		sprintf (info," %s PPC405GP %c %s MHz (%lu/%lu/%lu MHz)",
			buf,rev,
			strmhz (tmp, gd->cpu_clk), sys_info.freqPLB / 1000000,
			sys_info.freqPLB / sys_info.pllOpbDiv / 1000000,
			sys_info.freqPLB / sys_info.pllExtBusDiv / 1000000);
		return;
	case 3:
		/* Memory Info */
		sw = switch_cs (0);
		switch_cs (sw);
		bc = in8 (CONFIG_PORT_ADDR);
		sprintf(info, " %luMB RAM, %luMB Flash Cfg 0x%02X %s %s",
			gd->bd->bi_memsize / 0x100000,
			gd->bd->bi_flashsize / 0x100000,
			bc,
			sw ? "MPS boot" : "Flash boot",
			ctfb.modeIdent);
		return;
	case 1:
		sprintf	(buf, "%s",CONFIG_IDENT_STRING);
		sprintf (info, " %s", &buf[1]);
		return;
    }
    /* no more info lines */
    *info = 0;
    return;
}
#endif /* CONFIG_CONSOLE_EXTRA_INFO */

#endif /* CONFIG_VIDEO */
