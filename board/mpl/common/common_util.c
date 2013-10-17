/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#if defined(CONFIG_PIP405) || defined(CONFIG_MIP405)
/*-----------------------------------------------------------------------
 * On PIP/MIP405 we have 3 (4) possible boot mode
 *
 * - Boot from Flash (Flash CS = CS0, MPS CS = CS1)
 * - Boot from MPS   (Flash CS = CS1, MPS CS = CS0)
 * - Boot from PCI with Flash map (Flash CS = CS0, MPS CS = CS1)
 * - Boot from PCI with MPS map   (Flash CS = CS1, MPS CS = CS0)
 * The flash init is the first board specific routine which is called
 * after code relocation (running from SDRAM)
 * The first thing we do is to map the Flash CS to the Flash area and
 * the MPS CS to the MPS area. Since the flash size is unknown at this
 * point, we use the max flash size and the lowest flash address as base.
 *
 * After flash detection we adjust the size of the CS area accordingly.
 * update_flash_size() will fix in wrong values in the flash_info structure,
 * misc_init_r() will fix the values in the board info structure
 */
int get_boot_mode(void)
{
	unsigned long pbcr;
	int res = 0;
	pbcr = mfdcr(CPC0_PSR);
	if ((pbcr & PSR_ROM_WIDTH_MASK) == 0)
		/* boot via MPS or MPS mapping */
		res = BOOT_MPS;
	if (pbcr & PSR_ROM_LOC)
		/* boot via PCI.. */
		res |= BOOT_PCI;
	 return res;
}

/* Map the flash high (in boot area)
   This code can only be executed from SDRAM (after relocation).
*/
void setup_cs_reloc(void)
{
	int mode;
	/*
	 * since we are relocated, we can set-up the CS finaly
	 * but first of all, switch off PCI mapping (in case it
	 * was a PCI boot)
	 */
	out32r(PMM0MA, 0L);
	/* get boot mode */
	mode = get_boot_mode();
	/*
	 * we map the flash high in every case
	 * first find out to which CS the flash is attached to
	 */
	if (mode & BOOT_MPS) {
		/* map flash high on CS1 and MPS on CS0 */
		mtdcr(EBC0_CFGADDR, PB0AP);
		mtdcr(EBC0_CFGDATA, MPS_AP);
		mtdcr(EBC0_CFGADDR, PB0CR);
		mtdcr(EBC0_CFGDATA, MPS_CR);
		/*
		 * we use the default values (max values) for the flash
		 * because its real size is not yet known
		 */
		mtdcr(EBC0_CFGADDR, PB1AP);
		mtdcr(EBC0_CFGDATA, FLASH_AP);
		mtdcr(EBC0_CFGADDR, PB1CR);
		mtdcr(EBC0_CFGDATA, FLASH_CR_B);
	} else {
		/* map flash high on CS0 and MPS on CS1 */
		mtdcr(EBC0_CFGADDR, PB1AP);
		mtdcr(EBC0_CFGDATA, MPS_AP);
		mtdcr(EBC0_CFGADDR, PB1CR);
		mtdcr(EBC0_CFGDATA, MPS_CR);
		/*
		 * we use the default values (max values) for the flash
		 * because its real size is not yet known
		 */
		mtdcr(EBC0_CFGADDR, PB0AP);
		mtdcr(EBC0_CFGDATA, FLASH_AP);
		mtdcr(EBC0_CFGADDR, PB0CR);
		mtdcr(EBC0_CFGDATA, FLASH_CR_B);
	}
}
#endif /* #if defined(CONFIG_PIP405) || defined(CONFIG_MIP405) */

#ifdef CONFIG_SYS_UPDATE_FLASH_SIZE
/* adjust flash start and protection info */
int update_flash_size(int flash_size)
{
	int i = 0, mode;
	flash_info_t *info = &flash_info[0];
	unsigned long flashcr;
	unsigned long flash_base = (0 - flash_size) & 0xFFF00000;

	if (flash_size > 128*1024*1024) {
		printf("\n ### ERROR, wrong flash size: %X, reset board ###\n",
		       flash_size);
		hang();
	}

	if ((flash_size >> 20) != 0)
		i = __ilog2(flash_size >> 20);

	/* set up flash CS according to the size */
	mode = get_boot_mode();
	if (mode & BOOT_MPS) {
		/* flash is on CS1 */
		mtdcr(EBC0_CFGADDR, PB1CR);
		flashcr = mfdcr(EBC0_CFGDATA);
		/* we map the flash high in every case */
		flashcr &= 0x0001FFFF; /* mask out address bits */
		flashcr |= flash_base; /* start addr */
		flashcr |= (i << 17); /* size addr */
		mtdcr(EBC0_CFGADDR, PB1CR);
		mtdcr(EBC0_CFGDATA, flashcr);
	} else {
		/* flash is on CS0 */
		mtdcr(EBC0_CFGADDR, PB0CR);
		flashcr = mfdcr(EBC0_CFGDATA);
		/* we map the flash high in every case */
		flashcr &= 0x0001FFFF; /* mask out address bits */
		flashcr |= flash_base; /* start addr */
		flashcr |= (i << 17); /* size addr */
		mtdcr(EBC0_CFGADDR, PB0CR);
		mtdcr(EBC0_CFGDATA, flashcr);
	}

	for (i = 0; i < info->sector_count; i++)
		/* adjust sector start address */
		info->start[i] = flash_base +
				(info->start[i] - CONFIG_SYS_FLASH_BASE);

	/* unprotect all sectors */
	flash_protect(FLAG_PROTECT_CLEAR,
		      info->start[0],
		      0xFFFFFFFF,
		      info);
	flash_protect_default();
	/* protect reset vector too*/
	flash_protect(FLAG_PROTECT_SET,
		      info->start[info->sector_count-1],
		      0xFFFFFFFF,
		      info);

	return 0;
}
#endif

static int
mpl_prg(uchar *src, ulong size)
{
	ulong start;
	flash_info_t *info = &flash_info[0];
	int i, rc;
#if defined(CONFIG_PATI)
	int start_sect;
#endif
#if defined(CONFIG_PIP405) || defined(CONFIG_MIP405) || defined(CONFIG_PATI)
	char *copystr = (char *)src;
	ulong *magic = (ulong *)src;
#endif

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

	/* unprotect sectors used by u-boot */
	flash_protect(FLAG_PROTECT_CLEAR,
		      start,
		      0xFFFFFFFF,
		      info);

	/* search start sector */
	for (i = info->sector_count-1; i > 0; i--)
		if (start >= info->start[i])
			break;

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

	/* search start sector */
	for (i = info->sector_count-1; i > 0; i--)
		if (start >= info->start[i])
			break;

	start_sect = i;

	for (i = info->sector_count-1; i > 0; i--)
		if ((start + size) >= info->start[i])
			break;

	/* unprotect sectors used by u-boot */
	flash_protect(FLAG_PROTECT_CLEAR,
		      start,
		      start + size,
		      info);

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

	/* search end sector */
	for (i = 0; i < info->sector_count; i++)
		if (size < info->start[i])
		    break;

	flash_protect(FLAG_PROTECT_CLEAR,
		      start,
		      size,
		      info);

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
	ulong ld_addr;
	int result;
#if !defined(CONFIG_PATI)
	ulong size = IMAGE_SIZE;
	ulong src = MULTI_PURPOSE_SOCKET_ADDR;
	backup_t back;
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
