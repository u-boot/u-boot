#include "memio.h"
#include "articiaS.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


void sm_write_mode(void)
{
    out_byte(0xA539, 0x00);
    out_byte(0xA53A, 0x03);
}

void sm_read_mode(void)
{
    out_byte(0xA53A, 0x02);
    out_byte(0xA539, 0x02);
}

void sm_write_byte(uint8 writeme)
{
    int i;
    int level;

    out_byte(0xA539, 0x00);

    level = 0;

    for (i=0; i<8; i++)
    {
	if ((writeme & 0x80) == (level<<7))
	{
	    /* Bit did not change, rewrite strobe */
	    out_byte(0xA539, level | 0x02);
	    out_byte(0xA539, level);
	}
	else
	{
	    /* Bit changed, set bit, then strobe */
	    level = (writeme & 0x80) >> 7;
	    out_byte(0xA539, level);
	    out_byte(0xA539, level | 0x02);
	    out_byte(0xA539, level);
	}
	writeme <<= 1;
    }
    out_byte(0xA539, 0x00);
}

uint8 sm_read_byte(void)
{
    uint8 retme, r;
    int i;

    retme = 0;
    for (i=0; i<8; i++)
    {
	retme <<= 1;
	out_byte(0xA539, 0x00);
	out_byte(0xA539, 0x02);
	r = in_byte(0xA538) & 0x01;
	retme |= r;
    }

    return retme;
}

int sm_get_ack(void)
{
    uint8 r;
    r = in_byte(0xA538);
    if ((r&0x01) == 0) return TRUE;
    else return FALSE;
}

void sm_write_ack(void)
{
    out_byte(0xA539, 0x00);
    out_byte(0xA539, 0x02);
    out_byte(0xA539, 0x00);
}

void sm_write_nack(void)
{
    out_byte(0xA539, 0x01);
    out_byte(0xA539, 0x03);
    out_byte(0xA539, 0x01);
}

void sm_send_start(void)
{
    out_byte(0xA539, 0x03);
    out_byte(0xA539, 0x02);
}

void sm_send_stop(void)
{
    out_byte(0xA539, 0x02);
    out_byte(0xA539, 0x03);
}

int sm_read_byte_from_device(uint8 addr, uint8 reg, uint8 *storage)
{
    /* S Addr Wr */
    sm_write_mode();
    sm_send_start();
    sm_write_byte((addr<<1));

    /* [A] */
    sm_read_mode();
    if (sm_get_ack() == FALSE) return FALSE;

    /* Comm */
    sm_write_mode();
    sm_write_byte(reg);

    /* [A] */
    sm_read_mode();
    if (sm_get_ack() == FALSE) return FALSE;

    /* S Addr Rd */
    sm_write_mode();
    sm_send_start();
    sm_write_byte((addr<<1)|1);

    /* [A] */
    sm_read_mode();
    if (sm_get_ack() == FALSE) return FALSE;

    /* [Data] */
    *storage = sm_read_byte();

    /* NA */
    sm_write_mode();
    sm_write_nack();
    sm_send_stop();

    return TRUE;
}

void sm_init(void)
{
    /* Switch to PMC mode */
    pci_write_cfg_byte(0, 0, REG_GROUP, (uint8)(REG_GROUP_SPECIAL|REG_GROUP_POWER));

    /* Set GPIO Base */
    pci_write_cfg_long(0, 0, 0x40, 0xa500);

    /* Enable GPIO */
    pci_write_cfg_byte(0, 0, 0x44, 0x11);

    /* Set both GPIO 0 and 1 as output */
    out_byte(0xA53A, 0x03);
}


void sm_term(void)
{
    /* Switch to normal mode */
    pci_write_cfg_byte(0, 0, REG_GROUP, 0);
}


int sm_get_data(uint8 *DataArray, int dimm_socket)
{
    int j;

#if 0
    /* Switch to PMC mode */
    pci_write_cfg_byte(0, 0, REG_GROUP, (uint8)(REG_GROUP_SPECIAL|REG_GROUP_POWER));

    /* Set GPIO Base */
    pci_write_cfg_long(0, 0, 0x40, 0xa500);

    /* Enable GPIO */
    pci_write_cfg_byte(0, 0, 0x44, 0x11);

    /* Set both GPIO 0 and 1 as output */
    out_byte(0xA53A, 0x03);
#endif

    sm_init();
    /* Start reading the rom */

    j = 0;

    do
    {
	if (sm_read_byte_from_device(dimm_socket, (uint8)j, DataArray) == FALSE)
	{
	    sm_term();
	    return FALSE;
	}

	DataArray++;
	j++;
    } while (j < 128);

    sm_term();
    return TRUE;
}
