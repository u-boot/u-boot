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

extern void AT91F_SpiInit (void);
extern int AT91F_DataflashProbe (int i, AT91PS_DataflashDesc pDesc);
extern int AT91F_DataFlashRead (AT91PS_DataFlash pDataFlash,
				unsigned long addr,
				unsigned long size, char *buffer);


int AT91F_DataflashInit (void)
{
	int i, j;
	int dfcode;

	AT91F_SpiInit ();

	for (i = 0; i < CFG_MAX_DATAFLASH_BANKS; i++) {

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

		default:
			break;
		}

		for (j = 0; j < dataflash_info[i].Device.pages_number; j++)
			dataflash_info[i].protect[j] = FLAG_PROTECT_SET;

	}
	return (1);
}



void dataflash_print_info (void)
{
	int i;

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
		}
	}
}


/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_DataflashSelect 					*/
/* Object              : Select the correct device				*/
/*------------------------------------------------------------------------------*/
AT91PS_DataFlash AT91F_DataflashSelect (AT91PS_DataFlash pFlash,
										unsigned int *addr)
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

/*------------------------------------------------------------------------------*/
/* Function Name       : read_dataflash 					*/
/* Object              : dataflash memory read					*/
/*------------------------------------------------------------------------------*/
int read_dataflash (unsigned long addr, unsigned long size, char *result)
{
	int AddrToRead = addr;
	AT91PS_DataFlash pFlash = &DataFlashInst;

	pFlash = AT91F_DataflashSelect (pFlash, &AddrToRead);
	if (pFlash == 0)
		return -1;

	return (AT91F_DataFlashRead (pFlash, AddrToRead, size, result));
}


/*-----------------------------------------------------------------------------*/
/* Function Name       : write_dataflash 				       */
/* Object              : write a block in dataflash			       */
/*-----------------------------------------------------------------------------*/
int write_dataflash (unsigned long addr_dest, unsigned long addr_src,
		     unsigned long size)
{
	extern AT91S_DataFlashStatus AT91F_DataFlashWrite(
			AT91PS_DataFlash, uchar *, int, int);
	int AddrToWrite = addr_dest;
	AT91PS_DataFlash pFlash = &DataFlashInst;

	pFlash = AT91F_DataflashSelect (pFlash, &AddrToWrite);
	if (AddrToWrite == -1)
		return -1;

	return AT91F_DataFlashWrite (pFlash, (char *) addr_src, AddrToWrite,
								 size);
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
