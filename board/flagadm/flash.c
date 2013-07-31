/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8xx.h>
#include <flash.h>

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */
ulong flash_recognize (vu_long *base);
int write_word (flash_info_t *info, ulong dest, ulong data);
void flash_get_geometry (vu_long *base, flash_info_t *info);
void flash_unprotect(flash_info_t *info);
int _flash_real_protect(flash_info_t *info, long idx, int on);


unsigned long flash_init (void)
{
	volatile immap_t	*immap  = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8xx_t	*memctl = &immap->im_memctl;
	int i;
	int rec;

	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	*((vu_short*)CONFIG_SYS_FLASH_BASE) = 0xffff;

	flash_get_geometry ((vu_long*)CONFIG_SYS_FLASH_BASE, &flash_info[0]);

	/* Remap FLASH according to real size */
	memctl->memc_or0 = CONFIG_SYS_OR_TIMING_FLASH | (-flash_info[0].size & 0xFFFF8000);
	memctl->memc_br0 = (CONFIG_SYS_FLASH_BASE & BR_BA_MSK) |
		(memctl->memc_br0 & ~(BR_BA_MSK));

	rec = flash_recognize((vu_long*)CONFIG_SYS_FLASH_BASE);

	if (rec == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
				flash_info[0].size, flash_info[0].size<<20);
	}

#if CONFIG_SYS_FLASH_PROTECTION
	/*Unprotect all the flash memory*/
	flash_unprotect(&flash_info[0]);
#endif

	*((vu_short*)CONFIG_SYS_FLASH_BASE) = 0xffff;

	return (flash_info[0].size);

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_OFFSET,
		      CONFIG_ENV_OFFSET+CONFIG_ENV_SIZE-1,
		      &flash_info[0]);
#endif
	return (flash_info[0].size);
}


int flash_get_protect_status(flash_info_t * info, long idx)
{
	vu_short * base;
	ushort res;

#ifdef DEBUG
	printf("\n Attempting to set protection info with %d sectors\n", info->sector_count);
#endif


	base = (vu_short*)info->start[idx];

	*(base) = 0xffff;

	*(base + 0x55) = 0x0098;
	res = base[0x2];

	*(base) = 0xffff;

	if(res != 0)
		res = 1;
	else
		res = 0;

	return res;
}

void flash_get_geometry (vu_long *base, flash_info_t *info)
{
	int i,j;
	ulong ner = 0;
	vu_short * sb  = (vu_short*)base;
	ulong offset = (ulong)base;

	/* Read Device geometry */

	*sb = 0xffff;

	*sb = 0x0090;

	info->flash_id = ((ulong)base[0x0]);
#ifdef DEBUG
	printf("Id is %x\n", (uint)(ulong)info->flash_id);
#endif

	*sb = 0xffff;

	*(sb+0x55) = 0x0098;

	info->size = 1 << (sb[0x27]); /* Read flash size */

#ifdef DEBUG
	printf("Size is %x\n", (uint)(ulong)info->size);
#endif

	*sb = 0xffff;

	*(sb + 0x55) = 0x0098;
	ner = sb[0x2c] ; /*Number of erase regions*/

#ifdef DEBUG
	printf("Number of erase regions %x\n", (uint)ner);
#endif

	info->sector_count = 0;

	for(i = 0; i < ner; i++)
	{
		uint s;
		uint count;
		uint t1,t2,t3,t4;

		*sb = 0xffff;

		*(sb + 0x55) = 0x0098;

		t1 = sb[0x2d + i*4];
		t2 = sb[0x2e + i*4];
		t3 = sb[0x2f + i*4];
		t4 = sb[0x30 + i*4];

		count = ((t1 & 0x00ff) | (((t2 & 0x00ff) << 8) & 0xff00) )+ 1; /*sector count*/
		s = ((t3 & 0x00ff) | (((t4 & 0x00ff) << 8) & 0xff00)) * 256;; /*Sector size*/

#ifdef DEBUG
		printf("count and size %x, %x\n", count, s);
		printf("sector count for erase region %d is %d\n", i, count);
#endif
		for(j = 0; j < count; j++)
		{
#ifdef DEBUG
			printf("%x, ", (uint)offset);
#endif
			info->start[ info->sector_count + j] = offset;
			offset += s;
		}
		info->sector_count += count;
	}

	if ((offset - (ulong)base) != info->size)
		printf("WARNING reported size %x does not match to calculted size %x.\n"
				, (uint)info->size, (uint)(offset - (ulong)base) );

	/* Next check if there are any sectors protected.*/

	for(i = 0; i < info->sector_count; i++)
		info->protect[i] = flash_get_protect_status(info, i);

	*sb = 0xffff;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return ;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case INTEL_MANUFACT & FLASH_VENDMASK:
		printf ("Intel ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case INTEL_ID_28F320C3B & FLASH_TYPEMASK:
		printf ("28F320RC3(4 MB)\n");
		break;
	case INTEL_ID_28F320J3A:
		printf("28F320J3A (4 MB)\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
			break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 4) == 0)
			printf ("\n   ");
		printf ("  %02d %08lX%s",
			i, info->start[i],
			info->protect[i]!=0 ? " (RO)" : "     "
		);
	}
	printf ("\n");
	return ;
}

ulong flash_recognize (vu_long *base)
{
	ulong id;
	ulong res = FLASH_UNKNOWN;
	vu_short * sb = (vu_short*)base;

	*sb = 0xffff;

	*sb = 0x0090;
	id = base[0];

	switch (id & 0x00FF0000)
	{
		case (MT_MANUFACT & 0x00FF0000):	/* MT or => Intel */
		case (INTEL_ALT_MANU & 0x00FF0000):
		res = FLASH_MAN_INTEL;
		break;
	default:
		res = FLASH_UNKNOWN;
	}

	*sb = 0xffff;

	return res;
}

/*-----------------------------------------------------------------------*/
#define INTEL_FLASH_STATUS_BLS	0x02
#define INTEL_FLASH_STATUS_PSS	0x04
#define INTEL_FLASH_STATUS_VPPS	0x08
#define INTEL_FLASH_STATUS_PS	0x10
#define INTEL_FLASH_STATUS_ES	0x20
#define INTEL_FLASH_STATUS_ESS	0x40
#define INTEL_FLASH_STATUS_WSMS	0x80

int	flash_decode_status_bits(char status)
{
	int err = 0;

	if(!(status & INTEL_FLASH_STATUS_WSMS)) {
		printf("Busy\n");
		err = -1;
	}

	if(status & INTEL_FLASH_STATUS_ESS) {
		printf("Erase suspended\n");
		err = -1;
	}

	if(status & INTEL_FLASH_STATUS_ES) {
		printf("Error in block erase\n");
		err = -1;
	}

	if(status & INTEL_FLASH_STATUS_PS) {
		printf("Error in programming\n");
		err = -1;
	}

	if(status & INTEL_FLASH_STATUS_VPPS) {
		printf("Vpp low, operation aborted\n");
		err = -1;
	}

	if(status & INTEL_FLASH_STATUS_PSS) {
		printf("Program is suspended\n");
		err = -1;
	}

	if(status & INTEL_FLASH_STATUS_BLS) {
		printf("Attempting to program/erase a locked sector\n");
		err = -1;
	}

	if((status & INTEL_FLASH_STATUS_PS) &&
	   (status & INTEL_FLASH_STATUS_ES) &&
	   (status & INTEL_FLASH_STATUS_ESS)) {
		printf("A command sequence error\n");
		return -1;
	}

	return err;
}

/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	vu_short *addr;
	int flag, prot, sect;
	ulong start, now;
	int rcode = 0;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id & FLASH_VENDMASK) != (INTEL_MANUFACT & FLASH_VENDMASK)) {
		printf ("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		printf ("\n");
	}

	start = get_timer (0);

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		char tmp;

		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_short *)(info->start[sect]);

			/* Disable interrupts which might cause a timeout here */
			flag = disable_interrupts();

			/* Single Block Erase Command */
			*addr = 0x0020;
			/* Confirm */
			*addr = 0x00D0;
			/* Resume Command, as per errata update */
			*addr = 0x00D0;

			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();

			*addr = 0x70; /*Read status register command*/
			tmp = (short)*addr & 0x00FF; /* Read the status */
			while (!(tmp & INTEL_FLASH_STATUS_WSMS)) {
				if ((now=get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
					*addr = 0x0050; /* Reset the status register */
					*addr = 0xffff;
					printf ("Timeout\n");
					return 1;
				}
				/* show that we're waiting */
				if ((now - start) > 1000) {	/* every second */
					putc ('.');
				}
				udelay(100000); /* 100 ms */
				*addr = 0x0070; /*Read status register command*/
				tmp = (short)*addr & 0x00FF; /* Read status */
				start = get_timer(0);
			}
			if( tmp & INTEL_FLASH_STATUS_ES )
				flash_decode_status_bits(tmp);

			*addr = 0x0050; /* Reset the status register */
			*addr = 0xffff; /* Reset to read mode */
		}
	}


	printf (" done\n");
	return rcode;
}

void flash_unprotect (flash_info_t *info)
{
	/*We can only unprotect the whole flash at once*/
	/*Therefore we must prevent the _flash_real_protect()*/
	/*from re-protecting sectors, that ware protected before */
	/*we called flash_real_protect();*/

	int i;

	for(i = 0; i < info->sector_count; i++)
		info->protect[i] = 0;

#ifdef CONFIG_SYS_FLASH_PROTECTION
		_flash_real_protect(info, 0, 0);
#endif
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;

	wp = (addr & ~3);	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i=0, cp=wp; i<l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}
		for (; i<4 && cnt>0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt==0 && i<4; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}

		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp += 4;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
		data = 0;
		for (i=0; i<4; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp  += 4;
		cnt -= 4;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i=0, cp=wp; i<4 && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<4; ++i, ++cp) {
		data = (data << 8) | (*(uchar *)cp);
	}

	return (write_word(info, wp, data));
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_word (flash_info_t *info, ulong dest, ulong da)
{
	vu_short *addr = (vu_short *)dest;
	ulong start;
	char csr;
	int flag;
	int i;
	union {
		u32 data32;
		u16 data16[2];
	} data;

	data.data32 = da;

	/* Check if Flash is (sufficiently) erased */
	if (((*addr & data.data16[0]) != data.data16[0]) ||
	    ((*(addr+1) & data.data16[1]) != data.data16[1])) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	for(i = 0; i < 2; i++)
	{
		/* Write Command */
		*addr = 0x0010;

		/* Write Data */
		*addr = data.data16[i];

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		/* data polling for D7 */
		start = get_timer (0);
		flag  = 0;
		*addr = 0x0070; /*Read statusregister command */
		while (((csr = *addr) & INTEL_FLASH_STATUS_WSMS)!=INTEL_FLASH_STATUS_WSMS) {
			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
				flag = 1;
				break;
			}
			*addr = 0x0070; /*Read statusregister command */
		}
		if (csr & INTEL_FLASH_STATUS_PSS) {
			printf ("CSR indicates write error (%0x) at %08lx\n",
					csr, (ulong)addr);
			flag = 1;
		}

		/* Clear Status Registers Command */
		*addr = 0x0050;
		/* Reset to read array mode */
		*addr = 0xffff;
		addr++;
	}

	return (flag);
}

int flash_real_protect(flash_info_t *info, long offset, int prot)
{
	int i, idx;

	for(idx = 0; idx < info->sector_count; idx++)
		if(info->start[idx] == offset)
			break;

	if(idx==info->sector_count)
		return -1;

	if(prot == 0) {
		/* Unprotect one sector, which means unprotect all flash
		 * and reprotect the other protected sectors.
		 */
		_flash_real_protect(info, 0, 0); /* Unprotects the whole flash*/
		info->protect[idx] = 0;

		for(i = 0; i < info->sector_count; i++)
			if(info->protect[i])
				_flash_real_protect(info, i, 1);
		}
	else {
		/* We can protect individual sectors */
		_flash_real_protect(info, idx, 1);
	}

	for( i = 0; i < info->sector_count; i++)
		info->protect[i] = flash_get_protect_status(info, i);

	return 0;
}

int _flash_real_protect(flash_info_t *info, long idx, int prot)
{
	vu_short *addr;
	int flag;
	ushort cmd;
	ushort tmp;
	ulong now, start;

	if ((info->flash_id & FLASH_VENDMASK) != (INTEL_MANUFACT & FLASH_VENDMASK)) {
		printf ("Can't change protection for unknown flash type %08lx - aborted\n",
			info->flash_id);
		return -1;
	}

	if(prot == 0) {
		/*Unlock the sector*/
		cmd = 0x00D0;
	}
	else {
		/*Lock the sector*/
		cmd = 0x0001;
	}

	addr = (vu_short *)(info->start[idx]);

	/* If chip is busy, wait for it */
	start = get_timer(0);
	*addr = 0x0070; /*Read status register command*/
	tmp = ((ushort)(*addr))&0x00ff; /*Read the status*/
	while(!(tmp & INTEL_FLASH_STATUS_WSMS)) {
		/*Write State Machine Busy*/
		/*Wait untill done or timeout.*/
		if ((now=get_timer(start)) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			*addr = 0x0050; /* Reset the status register */
			*addr = 0xffff; /* Reset the chip */
			printf ("TTimeout\n");
			return 1;
		}
		*addr = 0x0070;
		tmp = ((ushort)(*addr))&0x00ff; /*Read the status*/
		start = get_timer(0);
	}

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/* Unlock block*/
	*addr = 0x0060;

	*addr = cmd;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	start = get_timer(0);
	*addr = 0x0070; /*Read status register command*/
	tmp = ((ushort)(*addr)) & 0x00FF; /* Read the status */
	while (!(tmp & INTEL_FLASH_STATUS_WSMS)) {
		/* Write State Machine Busy */
		if ((now=get_timer(start)) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			*addr = 0x0050; /* Reset the status register */
			*addr = 0xffff;
			printf ("Timeout\n");
			return 1;
		}
		/* show that we're waiting */
		if ((now - start) > 1000) {	/* every second */
			putc ('.');
		}
		udelay(100000); /* 100 ms */
		*addr = 0x70; /*Read status register command*/
		tmp = (short)*addr & 0x00FF; /* Read status */
		start = get_timer(0);
	}
	if( tmp & INTEL_FLASH_STATUS_PS )
		flash_decode_status_bits(tmp);

	*addr =0x0050; /*Clear status register*/

	/* reset to read mode */
	*addr = 0xffff;

	return 0;
}
