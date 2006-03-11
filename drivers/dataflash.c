/* LowLevel function for ATMEL DataFlash support
 * Author : Hamid Ikdoumi (Atmel)
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
#include <config.h>
#ifdef CONFIG_HAS_DATAFLASH
#include <asm/hardware.h>
#include <dataflash.h>

AT91S_DATAFLASH_INFO dataflash_info[CFG_MAX_DATAFLASH_BANKS];
static AT91S_DataFlash DataFlashInst;

int cs[][CFG_MAX_DATAFLASH_BANKS] = {
	{CFG_DATAFLASH_LOGIC_ADDR_CS0, 0},	/* Logical adress, CS */
	{CFG_DATAFLASH_LOGIC_ADDR_CS3, 3}
};

/*define the area offsets*/
dataflash_protect_t area_list[NB_DATAFLASH_AREA] = {
	{0, 0x7fff, FLAG_PROTECT_SET},			/* ROM code */
	{0x8000, 0x1ffff, FLAG_PROTECT_SET},		/* u-boot code */
	{0x20000, 0x27fff, FLAG_PROTECT_CLEAR},		/* u-boot environment */
	{0x28000, 0x1fffff, FLAG_PROTECT_CLEAR},	/* data area size to tune */
};

extern void AT91F_SpiInit (void);
extern int AT91F_DataflashProbe (int i, AT91PS_DataflashDesc pDesc);
extern int AT91F_DataFlashRead (AT91PS_DataFlash pDataFlash,
				unsigned long addr,
				unsigned long size, char *buffer);
extern int AT91F_DataFlashWrite( AT91PS_DataFlash pDataFlash,
				    unsigned char *src,
			            int dest,
				    int size );

int AT91F_DataflashInit (void)
{
	int i, j;
	int dfcode;

	AT91F_SpiInit ();

	for (i = 0; i < CFG_MAX_DATAFLASH_BANKS; i++) {
		dataflash_info[i].Desc.state = IDLE;
		dataflash_info[i].id = 0;
		dataflash_info[i].Device.pages_number = 0;
		dfcode = AT91F_DataflashProbe (cs[i][1], &dataflash_info[i].Desc);

		switch (dfcode) {
		case AT45DB161:
			dataflash_info[i].Device.pages_number = 4096;
			dataflash_info[i].Device.pages_size = 528;
			dataflash_info[i].Device.page_offset = 10;
			dataflash_info[i].Device.byte_mask = 0x300;
			dataflash_info[i].Device.cs = cs[i][1];
			dataflash_info[i].Desc.DataFlash_state = IDLE;
			dataflash_info[i].logical_address = cs[i][0];
			dataflash_info[i].id = dfcode;
			break;

		case AT45DB321:
			dataflash_info[i].Device.pages_number = 8192;
			dataflash_info[i].Device.pages_size = 528;
			dataflash_info[i].Device.page_offset = 10;
			dataflash_info[i].Device.byte_mask = 0x300;
			dataflash_info[i].Device.cs = cs[i][1];
			dataflash_info[i].Desc.DataFlash_state = IDLE;
			dataflash_info[i].logical_address = cs[i][0];
			dataflash_info[i].id = dfcode;
			break;

		case AT45DB642:
			dataflash_info[i].Device.pages_number = 8192;
			dataflash_info[i].Device.pages_size = 1056;
			dataflash_info[i].Device.page_offset = 11;
			dataflash_info[i].Device.byte_mask = 0x700;
			dataflash_info[i].Device.cs = cs[i][1];
			dataflash_info[i].Desc.DataFlash_state = IDLE;
			dataflash_info[i].logical_address = cs[i][0];
			dataflash_info[i].id = dfcode;
			break;
		case AT45DB128:
			dataflash_info[i].Device.pages_number = 16384;
			dataflash_info[i].Device.pages_size = 1056;
			dataflash_info[i].Device.page_offset = 11;
			dataflash_info[i].Device.byte_mask = 0x700;
			dataflash_info[i].Device.cs = cs[i][1];
			dataflash_info[i].Desc.DataFlash_state = IDLE;
			dataflash_info[i].logical_address = cs[i][0];
			dataflash_info[i].id = dfcode;
			break;

		default:
			break;
		}
		/* set the last area end to the dataflash size*/
		area_list[NB_DATAFLASH_AREA -1].end =
				(dataflash_info[i].Device.pages_number *
				dataflash_info[i].Device.pages_size)-1;

		/* set the area addresses */
		for(j = 0; j<NB_DATAFLASH_AREA; j++) {
			dataflash_info[i].Device.area_list[j].start = area_list[j].start + dataflash_info[i].logical_address;
			dataflash_info[i].Device.area_list[j].end = area_list[j].end + dataflash_info[i].logical_address;
			dataflash_info[i].Device.area_list[j].protected = area_list[j].protected;
		}
	}
	return (1);
}


void dataflash_print_info (void)
{
	int i, j;

	for (i = 0; i < CFG_MAX_DATAFLASH_BANKS; i++) {
		if (dataflash_info[i].id != 0) {
			printf ("DataFlash:");
			switch (dataflash_info[i].id) {
			case AT45DB161:
				printf ("AT45DB161\n");
				break;

			case AT45DB321:
				printf ("AT45DB321\n");
				break;

			case AT45DB642:
				printf ("AT45DB642\n");
				break;
			case AT45DB128:
				printf ("AT45DB128\n");
				break;
			}

			printf ("Nb pages: %6d\n"
				"Page Size: %6d\n"
				"Size=%8d bytes\n"
				"Logical address: 0x%08X\n",
				(unsigned int) dataflash_info[i].Device.pages_number,
				(unsigned int) dataflash_info[i].Device.pages_size,
				(unsigned int) dataflash_info[i].Device.pages_number *
				dataflash_info[i].Device.pages_size,
				(unsigned int) dataflash_info[i].logical_address);
			for (j=0; j< NB_DATAFLASH_AREA; j++) {
				printf ("Area %i:\t%08lX to %08lX %s\n", j,
					dataflash_info[i].Device.area_list[j].start,
					dataflash_info[i].Device.area_list[j].end,
					(dataflash_info[i].Device.area_list[j].protected ==
					FLAG_PROTECT_SET) ? "(RO)" : "");
			}
		}
	}
}


/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_DataflashSelect 					*/
/* Object              : Select the correct device				*/
/*------------------------------------------------------------------------------*/
AT91PS_DataFlash AT91F_DataflashSelect (AT91PS_DataFlash pFlash, unsigned long *addr)
{
	char addr_valid = 0;
	int i;

	for (i = 0; i < CFG_MAX_DATAFLASH_BANKS; i++)
		if ((*addr & 0xFF000000) == dataflash_info[i].logical_address) {
			addr_valid = 1;
			break;
		}
	if (!addr_valid) {
		pFlash = (AT91PS_DataFlash) 0;
		return pFlash;
	}
	pFlash->pDataFlashDesc = &(dataflash_info[i].Desc);
	pFlash->pDevice = &(dataflash_info[i].Device);
	*addr -= dataflash_info[i].logical_address;
	return (pFlash);
}

/*------------------------------------------------------------------------------*/
/* Function Name       : addr_dataflash 					*/
/* Object              : Test if address is valid				*/
/*------------------------------------------------------------------------------*/
int addr_dataflash (unsigned long addr)
{
	int addr_valid = 0;
	int i;

	for (i = 0; i < CFG_MAX_DATAFLASH_BANKS; i++) {
		if ((((int) addr) & 0xFF000000) ==
			dataflash_info[i].logical_address) {
			addr_valid = 1;
			break;
		}
	}

	return addr_valid;
}
/*-----------------------------------------------------------------------------*/
/* Function Name       : size_dataflash 					*/
/* Object              : Test if address is valid regarding the size		*/
/*-----------------------------------------------------------------------------*/
int size_dataflash (AT91PS_DataFlash pdataFlash, unsigned long addr, unsigned long size)
{
	/* is outside the dataflash */
	if (((int)addr & 0x0FFFFFFF) > (pdataFlash->pDevice->pages_size *
		pdataFlash->pDevice->pages_number)) return 0;
	/* is too large for the dataflash */
	if (size > ((pdataFlash->pDevice->pages_size *
		pdataFlash->pDevice->pages_number) - ((int)addr & 0x0FFFFFFF))) return 0;

	return 1;
}
/*-----------------------------------------------------------------------------*/
/* Function Name       : prot_dataflash 					*/
/* Object              : Test if destination area is protected			*/
/*-----------------------------------------------------------------------------*/
int prot_dataflash (AT91PS_DataFlash pdataFlash, unsigned long addr)
{
int area;
	/* find area */
	for (area=0; area < NB_DATAFLASH_AREA; area++) {
		if ((addr >= pdataFlash->pDevice->area_list[area].start) &&
			(addr < pdataFlash->pDevice->area_list[area].end))
			break;
	}
	if (area == NB_DATAFLASH_AREA) return -1;
	/*test protection value*/
	if (pdataFlash->pDevice->area_list[area].protected == FLAG_PROTECT_SET) return 0;

	return 1;
}
/*-----------------------------------------------------------------------------*/
/* Function Name       : dataflash_real_protect				*/
/* Object              : protect/unprotect area				*/
/*-----------------------------------------------------------------------------*/
int dataflash_real_protect (int flag, unsigned long start_addr, unsigned long end_addr)
{
int i,j, area1, area2, addr_valid = 0;
	/* find dataflash */
	for (i = 0; i < CFG_MAX_DATAFLASH_BANKS; i++) {
		if ((((int) start_addr) & 0xF0000000) ==
			dataflash_info[i].logical_address) {
				addr_valid = 1;
				break;
		}
	}
	if (!addr_valid) {
		return -1;
	}
	/* find start area */
	for (area1=0; area1 < NB_DATAFLASH_AREA; area1++) {
		if (start_addr == dataflash_info[i].Device.area_list[area1].start) break;
	}
	if (area1 == NB_DATAFLASH_AREA) return -1;
	/* find end area */
	for (area2=0; area2 < NB_DATAFLASH_AREA; area2++) {
		if (end_addr == dataflash_info[i].Device.area_list[area2].end) break;
	}
	if (area2 == NB_DATAFLASH_AREA) return -1;

	/*set protection value*/
	for(j = area1; j < area2+1 ; j++)
		if (flag == 0) dataflash_info[i].Device.area_list[j].protected = FLAG_PROTECT_CLEAR;
		else dataflash_info[i].Device.area_list[j].protected = FLAG_PROTECT_SET;

	return (area2-area1+1);
}

/*------------------------------------------------------------------------------*/
/* Function Name       : read_dataflash 					*/
/* Object              : dataflash memory read					*/
/*------------------------------------------------------------------------------*/
int read_dataflash (unsigned long addr, unsigned long size, char *result)
{
	unsigned long AddrToRead = addr;
	AT91PS_DataFlash pFlash = &DataFlashInst;

	pFlash = AT91F_DataflashSelect (pFlash, &AddrToRead);

	if (pFlash == 0)
		return ERR_UNKNOWN_FLASH_TYPE;

	if (size_dataflash(pFlash,addr,size) == 0)
		return ERR_INVAL;

	return (AT91F_DataFlashRead (pFlash, AddrToRead, size, result));
}


/*-----------------------------------------------------------------------------*/
/* Function Name       : write_dataflash 				       */
/* Object              : write a block in dataflash			       */
/*-----------------------------------------------------------------------------*/
int write_dataflash (unsigned long addr_dest, unsigned long addr_src,
		     unsigned long size)
{
	unsigned long AddrToWrite = addr_dest;
	AT91PS_DataFlash pFlash = &DataFlashInst;

	pFlash = AT91F_DataflashSelect (pFlash, &AddrToWrite);

	if (pFlash == 0)
		return ERR_UNKNOWN_FLASH_TYPE;

	if (size_dataflash(pFlash,addr_dest,size) == 0)
		return ERR_INVAL;

	if (prot_dataflash(pFlash,addr_dest) == 0)
		return ERR_PROTECTED;

	if (AddrToWrite == -1)
		return -1;

	return AT91F_DataFlashWrite (pFlash, (uchar *)addr_src, AddrToWrite, size);
}


void dataflash_perror (int err)
{
	switch (err) {
	case ERR_OK:
		break;
	case ERR_TIMOUT:
		printf ("Timeout writing to DataFlash\n");
		break;
	case ERR_PROTECTED:
		printf ("Can't write to protected DataFlash sectors\n");
		break;
	case ERR_INVAL:
		printf ("Outside available DataFlash\n");
		break;
	case ERR_UNKNOWN_FLASH_TYPE:
		printf ("Unknown Type of DataFlash\n");
		break;
	case ERR_PROG_ERROR:
		printf ("General DataFlash Programming Error\n");
		break;
	default:
		printf ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		break;
	}
}

#endif
