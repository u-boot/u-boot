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
#include <asm/byteorder.h>
#include <i2c.h>
#include <pci.h>
#include <malloc.h>
#include <bzlib.h>

#ifdef CONFIG_PIP405
#include "../pip405/pip405.h"
#include <asm/4xx_pci.h>
#endif
#ifdef CONFIG_MIP405
#include "../mip405/mip405.h"
#include <asm/4xx_pci.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_PATI)
#define FIRM_START 0xFFF00000
#endif

extern int mem_test(ulong start, ulong ramsize, int quiet);

#define I2C_BACKUP_ADDR 0x7C00		/* 0x200 bytes for backup */
#define IMAGE_SIZE CONFIG_SYS_MONITOR_LEN	/* ugly, but it works for now */

extern flash_info_t flash_info[];	/* info for FLASH chips */

static int
mpl_prg(uchar *src, ulong size)
{
	ulong start;
	flash_info_t *info;
	int i, rc;
#if defined(CONFIG_PATI)
	int start_sect;
#endif
#if defined(CONFIG_PIP405) || defined(CONFIG_MIP405) || defined(CONFIG_PATI)
	char *copystr = (char *)src;
	ulong *magic = (ulong *)src;
#endif

	info = &flash_info[0];

#if defined(CONFIG_PIP405) || defined(CONFIG_MIP405) || defined(CONFIG_PATI)
	if (uimage_to_cpu (magic[0]) != IH_MAGIC) {
		puts("Bad Magic number\n");
		return -1;
	}
	/* some more checks before we delete the Flash... */
	/* Checking the ISO_STRING prevents to program a
	 * wrong Firmware Image into the flash.
	 */
	i = 4; /* skip Magic number */
	while (1) {
		if (strncmp(&copystr[i], "MEV-", 4) == 0)
			break;
		if (i++ >= 0x100) {
			puts("Firmware Image for unknown Target\n");
			return -1;
		}
	}
	/* we have the ISO STRING, check */
	if (strncmp(&copystr[i], CONFIG_ISO_STRING, sizeof(CONFIG_ISO_STRING)-1) != 0) {
		printf("Wrong Firmware Image: %s\n", &copystr[i]);
		return -1;
	}
#if !defined(CONFIG_PATI)
	start = 0 - size;
	for (i = info->sector_count-1; i > 0; i--) {
		info->protect[i] = 0; /* unprotect this sector */
		if (start >= info->start[i])
			break;
	}
	/* set-up flash location */
	/* now erase flash */
	printf("Erasing at %lx (sector %d) (start %lx)\n",
				start,i,info->start[i]);
	if ((rc = flash_erase (info, i, info->sector_count-1)) != 0) {
		puts("ERROR ");
		flash_perror(rc);
		return (1);
	}

#else /* #if !defined(CONFIG_PATI */
	start = FIRM_START;
	start_sect = -1;
	for (i = 0; i < info->sector_count; i++) {
		if (start < info->start[i]) {
			start_sect = i - 1;
			break;
		}
	}

	info->protect[i - 1] = 0;	/* unprotect this sector */
	for (; i < info->sector_count; i++) {
		if ((start + size) < info->start[i])
			break;
		info->protect[i] = 0;	/* unprotect this sector */
	}

	i--;
	/* set-up flash location */
	/* now erase flash */
	printf ("Erasing at %lx to %lx (sector %d to %d) (%lx to %lx)\n",
		start, start + size, start_sect, i,
		info->start[start_sect], info->start[i]);
	if ((rc = flash_erase (info, start_sect, i)) != 0) {
		puts ("ERROR ");
		flash_perror (rc);
		return (1);
	}
#endif /* defined(CONFIG_PATI) */

#elif defined(CONFIG_VCMA9)
	start = 0;
	for (i = 0; i <info->sector_count; i++) {
		info->protect[i] = 0; /* unprotect this sector */
		if (size < info->start[i])
		    break;
	}
	/* set-up flash location */
	/* now erase flash */
	printf("Erasing at %lx (sector %d) (start %lx)\n",
				start,0,info->start[0]);
	if ((rc = flash_erase (info, 0, i)) != 0) {
		puts("ERROR ");
		flash_perror(rc);
		return (1);
	}

#endif
	printf("flash erased, programming from 0x%lx 0x%lx Bytes\n",
		(ulong)src, size);
	if ((rc = flash_write ((char *)src, start, size)) != 0) {
		puts("ERROR ");
		flash_perror(rc);
		return (1);
	}
	puts("OK programming done\n");
	return 0;
}


static int
mpl_prg_image(uchar *ld_addr)
{
	unsigned long len;
	uchar *data;
	image_header_t *hdr = (image_header_t *)ld_addr;
	int rc;

#if defined(CONFIG_FIT)
	if (genimg_get_format ((void *)hdr) != IMAGE_FORMAT_LEGACY) {
		puts ("Non legacy image format not supported\n");
		return -1;
	}
#endif

	if (!image_check_magic (hdr)) {
		puts("Bad Magic Number\n");
		return 1;
	}
	image_print_contents (hdr);
	if (!image_check_os (hdr, IH_OS_U_BOOT)) {
		puts("No U-Boot Image\n");
		return 1;
	}
	if (!image_check_type (hdr, IH_TYPE_FIRMWARE)) {
		puts("No Firmware Image\n");
		return 1;
	}
	if (!image_check_hcrc (hdr)) {
		puts("Bad Header Checksum\n");
		return 1;
	}
	puts("Verifying Checksum ... ");
	if (!image_check_dcrc (hdr)) {
		puts("Bad Data CRC\n");
		return 1;
	}
	puts("OK\n");

	data = (uchar *)image_get_data (hdr);
	len = image_get_data_size (hdr);

	if (image_get_comp (hdr) != IH_COMP_NONE) {
		uchar *buf;
		/* reserve space for uncompressed image */
		if ((buf = malloc(IMAGE_SIZE)) == NULL) {
			puts("Insufficient space for decompression\n");
			return 1;
		}

		switch (image_get_comp (hdr)) {
		case IH_COMP_GZIP:
			puts("Uncompressing (GZIP) ... ");
			rc = gunzip ((void *)(buf), IMAGE_SIZE, data, &len);
			if (rc != 0) {
				puts("GUNZIP ERROR\n");
				free(buf);
				return 1;
			}
			puts("OK\n");
			break;
#ifdef CONFIG_BZIP2
		case IH_COMP_BZIP2:
			puts("Uncompressing (BZIP2) ... ");
			{
			uint retlen = IMAGE_SIZE;
			rc = BZ2_bzBuffToBuffDecompress ((char *)(buf), &retlen,
				(char *)data, len, 0, 0);
			len = retlen;
			}
			if (rc != BZ_OK) {
				printf ("BUNZIP2 ERROR: %d\n", rc);
				free(buf);
				return 1;
			}
			puts("OK\n");
			break;
#endif
		default:
			printf ("Unimplemented compression type %d\n",
				image_get_comp (hdr));
			free(buf);
			return 1;
		}

		rc = mpl_prg(buf, len);
		free(buf);
	} else {
		rc = mpl_prg(data, len);
	}

	return(rc);
}

#if !defined(CONFIG_PATI)
void get_backup_values(backup_t *buf)
{
	i2c_read(CONFIG_SYS_DEF_EEPROM_ADDR, I2C_BACKUP_ADDR,2,(void *)buf,sizeof(backup_t));
}

void set_backup_values(int overwrite)
{
	backup_t back;
	int i;

	get_backup_values(&back);
	if(!overwrite) {
		if(strncmp(back.signature,"MPL\0",4)==0) {
			puts("Not possible to write Backup\n");
			return;
		}
	}
	memcpy(back.signature,"MPL\0",4);
	i = getenv_f("serial#",back.serial_name,16);
	if(i < 0) {
		puts("Not possible to write Backup\n");
		return;
	}
	back.serial_name[16]=0;
	i = getenv_f("ethaddr",back.eth_addr,20);
	if(i < 0) {
		puts("Not possible to write Backup\n");
		return;
	}
	back.eth_addr[20]=0;
	i2c_write(CONFIG_SYS_DEF_EEPROM_ADDR, I2C_BACKUP_ADDR,2,(void *)&back,sizeof(backup_t));
}

void clear_env_values(void)
{
	backup_t back;
	unsigned char env_crc[4];

	memset(&back,0xff,sizeof(backup_t));
	memset(env_crc,0x00,4);
	i2c_write(CONFIG_SYS_DEF_EEPROM_ADDR,I2C_BACKUP_ADDR,2,(void *)&back,sizeof(backup_t));
	i2c_write(CONFIG_SYS_DEF_EEPROM_ADDR,CONFIG_ENV_OFFSET,2,(void *)env_crc,4);
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
	eeprom_read (CONFIG_SYS_DEF_EEPROM_ADDR,
		     CONFIG_ENV_OFFSET,
		     (uchar *)&crc, sizeof(ulong));

	new = 0;
	len = oldsize;
	off = sizeof(long);
	len = oldsize-off;
	while (len > 0) {
		int n = (len > sizeof(buf)) ? sizeof(buf) : len;

		eeprom_read (CONFIG_SYS_DEF_EEPROM_ADDR, CONFIG_ENV_OFFSET+off, buf, n);
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

	name = &name_buf[0];
	value = &value_buf[0];
	len=size;
	off = sizeof(long);
	while (len > off) {
		eeprom_read (CONFIG_SYS_DEF_EEPROM_ADDR, CONFIG_ENV_OFFSET+off, &c, 1);
		if(c != '=') {
			*name++=c;
			off++;
		}
		else {
			*name++='\0';
			off++;
			do {
				eeprom_read (CONFIG_SYS_DEF_EEPROM_ADDR, CONFIG_ENV_OFFSET+off, &c, 1);
				*value++=c;
				off++;
				if(c == '\0')
					break;
			} while(len > off);
			name = &name_buf[0];
			value = &value_buf[0];
			if(strncmp((char *)name,"baudrate",8)!=0) {
				setenv((char *)name,(char *)value);
			}

		}
	}
}


void check_env(void)
{
	char *s;
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
			puts("INFO:  old environment ajusted, use saveenv\n");
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

#endif /* #if !defined(CONFIG_PATI) */

int do_mplcommon(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong size,src,ld_addr;
	int result;
#if !defined(CONFIG_PATI)
	backup_t back;
	src = MULTI_PURPOSE_SOCKET_ADDR;
	size = IMAGE_SIZE;
#endif

	if (strcmp(argv[1], "flash") == 0)
	{
#if defined(CONFIG_CMD_FDC)
		if (strcmp(argv[2], "floppy") == 0) {
			char *local_args[3];
			extern int do_fdcboot (cmd_tbl_t *, int, int, char *[]);
			puts("\nupdating bootloader image from floppy\n");
			local_args[0] = argv[0];
			if(argc==4) {
				local_args[1] = argv[3];
				local_args[2] = NULL;
				ld_addr=simple_strtoul(argv[3], NULL, 16);
				result=do_fdcboot(cmdtp, 0, 2, local_args);
			}
			else {
				local_args[1] = NULL;
				ld_addr=CONFIG_SYS_LOAD_ADDR;
				result=do_fdcboot(cmdtp, 0, 1, local_args);
			}
			result=mpl_prg_image((uchar *)ld_addr);
			return result;
		}
#endif
		if (strcmp(argv[2], "mem") == 0) {
			if(argc==4) {
				ld_addr=simple_strtoul(argv[3], NULL, 16);
			}
			else {
				ld_addr=load_addr;
			}
			printf ("\nupdating bootloader image from memory at %lX\n",ld_addr);
			result=mpl_prg_image((uchar *)ld_addr);
			return result;
		}
#if !defined(CONFIG_PATI)
		if (strcmp(argv[2], "mps") == 0) {
			puts("\nupdating bootloader image from MPS\n");
			result=mpl_prg((uchar *)src,size);
			return result;
		}
#endif /* #if !defined(CONFIG_PATI)	*/
	}
	if (strcmp(argv[1], "mem") == 0)
	{
		result=0;
		if(argc==3)
		{
			result = (int)simple_strtol(argv[2], NULL, 16);
	    }
	    src=(unsigned long)&result;
	    src-=CONFIG_SYS_MEMTEST_START;
	    src-=(100*1024); /* - 100k */
	    src&=0xfff00000;
	    size=0;
	    do {
		size++;
			printf("\n\nPass %ld\n",size);
			mem_test(CONFIG_SYS_MEMTEST_START,src,1);
			if(ctrlc())
				break;
			if(result>0)
				result--;

		}while(result);
		return 0;
	}
#if !defined(CONFIG_PATI)
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
#endif
	return cmd_usage(cmdtp);
}


#if defined(CONFIG_CMD_DOC)
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
extern int get_boot_mode(void);

void video_get_info_str (int line_number, char *info)
{
	/* init video info strings for graphic console */
	PPC4xx_SYS_INFO sys_info;
	char rev;
	int i,boot;
	unsigned long pvr;
	char buf[64];
	char buf1[32], buf2[32], buf3[32], buf4[32];
	char cpustr[16];
	char *s, *e, bc;
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
			case PVR_405GPR_RB: rev='B'; break;
			default:           rev='?'; break;
		}
		if(pvr==PVR_405GPR_RB)
			sprintf(cpustr,"PPC405GPr %c",rev);
		else
			sprintf(cpustr,"PPC405GP %c",rev);
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
				buf[i++] = *s;
			}
			sprintf(&buf[i]," SN ");
			i+=4;
			for (; s < e; ++s) {
				buf[i++] = *s;
			}
			buf[i++]=0;
		}
		sprintf (info," %s %s %s MHz (%s/%s/%s MHz)",
			buf, cpustr,
			strmhz (buf1, gd->cpu_clk),
			strmhz (buf2, sys_info.freqPLB),
			strmhz (buf3, sys_info.freqPLB / sys_info.pllOpbDiv),
			strmhz (buf4, sys_info.freqPLB / sys_info.pllExtBusDiv));
		return;
	case 3:
		/* Memory Info */
		boot = get_boot_mode();
		bc = in8 (CONFIG_PORT_ADDR);
		sprintf(info, " %luMB RAM, %luMB Flash Cfg 0x%02X %s %s",
			gd->bd->bi_memsize / 0x100000,
			gd->bd->bi_flashsize / 0x100000,
			bc,
			(boot & BOOT_MPS) ? "MPS boot" : "Flash boot",
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
