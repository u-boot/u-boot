/*
**=====================================================================
**
** Copyright (C) 2000, 2001, 2002, 2003
** The LEOX team <team@leox.org>, http://www.leox.org
**
** LEOX.org is about the development of free hardware and software resources
**   for system on chip.
**
** Description: U-Boot port on the LEOX's ELPT860 CPU board
** ~~~~~~~~~~~
**
**=====================================================================
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307 USA
**
**=====================================================================
*/

/*
** Note 1: In this file, you have to provide the following variable:
** ------
**              flash_info_t    flash_info[CONFIG_SYS_MAX_FLASH_BANKS]
** 'flash_info_t' structure is defined into 'include/flash.h'
** and defined as extern into 'common/cmd_flash.c'
**
** Note 2: In this file, you have to provide the following functions:
** ------
**              unsigned long   flash_init(void)
** called from 'board_init_r()' into 'common/board.c'
**
**              void            flash_print_info(flash_info_t *info)
** called from 'do_flinfo()' into 'common/cmd_flash.c'
**
**              int             flash_erase(flash_info_t *info,
**                                          int           s_first,
**                                          int           s_last)
** called from 'do_flerase()' & 'flash_sect_erase()' into 'common/cmd_flash.c'
**
**              int             write_buff (flash_info_t *info,
**                                          uchar        *src,
**                                          ulong         addr,
**                                          ulong         cnt)
** called from 'flash_write()' into 'common/cmd_flash.c'
*/

#include <common.h>
#include <mpc8xx.h>


#ifndef	CONFIG_ENV_ADDR
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#endif

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */

/*-----------------------------------------------------------------------
 * Internal Functions
 */
static void   flash_get_offsets (ulong base, flash_info_t *info);
static ulong  flash_get_size (volatile unsigned char *addr, flash_info_t *info);

static int write_word (flash_info_t *info, ulong dest, ulong data);
static int write_byte (flash_info_t *info, ulong dest, uchar data);

/*-----------------------------------------------------------------------
 */

unsigned long
flash_init (void)
{
  volatile immap_t     *immap  = (immap_t *)CONFIG_SYS_IMMR;
  volatile memctl8xx_t *memctl = &immap->im_memctl;
  unsigned long         size_b0;
  int i;

  /* Init: no FLASHes known */
  for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i)
    {
      flash_info[i].flash_id = FLASH_UNKNOWN;
    }

  /* Static FLASH Bank configuration here - FIXME XXX */

  size_b0 = flash_get_size ((volatile unsigned char *)FLASH_BASE0_PRELIM,
			    &flash_info[0]);

  if ( flash_info[0].flash_id == FLASH_UNKNOWN )
    {
      printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
	      size_b0, size_b0<<20);
    }

  /* Remap FLASH according to real size */
  memctl->memc_or0 = CONFIG_SYS_OR_TIMING_FLASH | (-size_b0 & OR_AM_MSK);
  memctl->memc_br0 = (CONFIG_SYS_FLASH_BASE & BR_BA_MSK) | BR_MS_GPCM | BR_PS_8 | BR_V;

  /* Re-do sizing to get full correct info */
  size_b0 = flash_get_size ((volatile unsigned char *)CONFIG_SYS_FLASH_BASE,
			    &flash_info[0]);

  flash_get_offsets (CONFIG_SYS_FLASH_BASE, &flash_info[0]);

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
  /* monitor protection ON by default */
  flash_protect (FLAG_PROTECT_SET,
		 CONFIG_SYS_MONITOR_BASE,
		 CONFIG_SYS_MONITOR_BASE + monitor_flash_len-1,
		 &flash_info[0]);
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
  /* ENV protection ON by default */
  flash_protect(FLAG_PROTECT_SET,
		CONFIG_ENV_ADDR,
		CONFIG_ENV_ADDR + CONFIG_ENV_SIZE-1,
		&flash_info[0]);
#endif

  flash_info[0].size = size_b0;

  return (size_b0);
}

/*-----------------------------------------------------------------------
 */
static void
flash_get_offsets (ulong          base,
		   flash_info_t  *info)
{
  int i;

#define SECTOR_64KB    0x00010000

  /* set up sector start adress table */
  for (i = 0; i < info->sector_count; i++)
    {
      info->start[i] = base + (i * SECTOR_64KB);
    }
}

/*-----------------------------------------------------------------------
 */
void
flash_print_info (flash_info_t *info)
{
  int i;

  if ( info->flash_id == FLASH_UNKNOWN )
    {
      printf ("missing or unknown FLASH type\n");
      return;
    }

  switch ( info->flash_id & FLASH_VENDMASK )
    {
    case FLASH_MAN_AMD:	printf ("AMD ");		break;
    case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
    case FLASH_MAN_STM: printf ("STM (Thomson) ");      break;
    default:		printf ("Unknown Vendor ");	break;
    }

  switch ( info->flash_id & FLASH_TYPEMASK )
    {
    case FLASH_AM040:   printf ("AM29F040   (4 Mbits)\n");
      break;
    default:	        printf ("Unknown Chip Type\n");
      break;
    }

  printf ("  Size: %ld KB in %d Sectors\n",
	  info->size >> 10, info->sector_count);

  printf ("  Sector Start Addresses:");
  for (i=0; i<info->sector_count; ++i)
    {
      if ((i % 5) == 0)
	printf ("\n   ");
      printf (" %08lX%s",
	      info->start[i],
	      info->protect[i] ? " (RO)" : "     "
	      );
    }
  printf ("\n");

  return;
}

/*-----------------------------------------------------------------------
 */


/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

static ulong
flash_get_size (volatile unsigned char *addr,
		flash_info_t           *info)
{
  short i;
  uchar value;
  ulong base = (ulong)addr;

  /* Write auto select command: read Manufacturer ID */
  addr[0x0555] = 0xAA;
  addr[0x02AA] = 0x55;
  addr[0x0555] = 0x90;

  value = addr[0];

  switch ( value )
    {
      /*    case AMD_MANUFACT: */
    case 0x01:
      info->flash_id = FLASH_MAN_AMD;
      break;
      /*    case FUJ_MANUFACT: */
    case 0x04:
      info->flash_id = FLASH_MAN_FUJ;
      break;
      /*    case STM_MANUFACT: */
    case 0x20:
      info->flash_id = FLASH_MAN_STM;
      break;

    default:
      info->flash_id = FLASH_UNKNOWN;
      info->sector_count = 0;
      info->size = 0;
      return (0);			/* no or unknown flash	*/
    }

  value = addr[1];			/* device ID		*/

  switch ( value )
    {
    case STM_ID_F040B:
    case AMD_ID_F040B:
      info->flash_id += FLASH_AM040;    /* 4 Mbits = 512k * 8  */
      info->sector_count = 8;
      info->size = 0x00080000;
      break;

    default:
      info->flash_id = FLASH_UNKNOWN;
      return (0);			/* => no or unknown flash */
    }

  /* set up sector start adress table */
  for (i = 0; i < info->sector_count; i++)
    {
      info->start[i] = base + (i * 0x00010000);
    }

  /* check for protected sectors */
  for (i = 0; i < info->sector_count; i++)
    {
      /* read sector protection at sector address, (A7 .. A0) = 0x02 */
      /* D0 = 1 if protected */
      addr = (volatile unsigned char *)(info->start[i]);
      info->protect[i] = addr[2] & 1;
    }

  /*
   * Prevent writes to uninitialized FLASH.
   */
  if ( info->flash_id != FLASH_UNKNOWN )
    {
      addr = (volatile unsigned char *)info->start[0];

      *addr = 0xF0;	/* reset bank */
    }

  return (info->size);
}


/*-----------------------------------------------------------------------
 */

int
flash_erase (flash_info_t  *info,
	     int            s_first,
	     int            s_last)
{
  volatile unsigned char *addr = (volatile unsigned char *)(info->start[0]);
  int flag, prot, sect, l_sect;
  ulong start, now, last;

  if ( (s_first < 0) || (s_first > s_last) )
    {
      if ( info->flash_id == FLASH_UNKNOWN )
	{
	  printf ("- missing\n");
	}
      else
	{
	  printf ("- no sectors to erase\n");
	}
      return ( 1 );
    }

  if ( (info->flash_id == FLASH_UNKNOWN) ||
       (info->flash_id > FLASH_AMD_COMP) )
    {
      printf ("Can't erase unknown flash type %08lx - aborted\n",
	      info->flash_id);
      return ( 1 );
    }

  prot = 0;
  for (sect=s_first; sect<=s_last; ++sect)
    {
      if ( info->protect[sect] )
	{
	  prot++;
	}
    }

  if ( prot )
    {
      printf ("- Warning: %d protected sectors will not be erased!\n", prot);
    }
  else
    {
      printf ("\n");
    }

  l_sect = -1;

  /* Disable interrupts which might cause a timeout here */
  flag = disable_interrupts();

  addr[0x0555] = 0xAA;
  addr[0x02AA] = 0x55;
  addr[0x0555] = 0x80;
  addr[0x0555] = 0xAA;
  addr[0x02AA] = 0x55;

  /* Start erase on unprotected sectors */
  for (sect = s_first; sect<=s_last; sect++)
    {
      if (info->protect[sect] == 0)    /* not protected */
	{
	  addr = (volatile unsigned char *)(info->start[sect]);
	  addr[0] = 0x30;
	  l_sect = sect;
	}
    }

  /* re-enable interrupts if necessary */
  if ( flag )
    enable_interrupts();

  /* wait at least 80us - let's wait 1 ms */
  udelay (1000);

  /*
   * We wait for the last triggered sector
   */
  if ( l_sect < 0 )
    goto DONE;

  start = get_timer (0);
  last  = start;
  addr = (volatile unsigned char *)(info->start[l_sect]);
  while ( (addr[0] & 0x80) != 0x80 )
    {
      if ( (now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT )
	{
	  printf ("Timeout\n");
	  return ( 1 );
	}
      /* show that we're waiting */
      if ( (now - last) > 1000 )     /* every second */
	{
	  putc ('.');
	  last = now;
	}
    }

DONE:
  /* reset to read mode */
  addr = (volatile unsigned char *)info->start[0];
  addr[0] = 0xF0;	/* reset bank */

  printf (" done\n");

  return ( 0 );
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int
write_buff (flash_info_t  *info,
	    uchar         *src,
	    ulong          addr,
	    ulong          cnt)
{
  ulong cp, wp, data;
  uchar bdata;
  int i, l, rc;

  if ( (info->flash_id & FLASH_TYPEMASK) == FLASH_AM040 )
    {
      /* Width of the data bus: 8 bits */

      wp = addr;

      while ( cnt )
	{
	  bdata = *src++;

	  if ( (rc = write_byte(info, wp, bdata)) != 0 )
	    {
	      return (rc);
	    }

	  ++wp;
	  --cnt;
	}

      return ( 0 );
    }
  else
    {
      /* Width of the data bus: 32 bits */

      wp = (addr & ~3);	/* get lower word aligned address */

      /*
       * handle unaligned start bytes
       */
      if ( (l = addr - wp) != 0 )
	{
	  data = 0;
	  for (i=0, cp=wp; i<l; ++i, ++cp)
	    {
	      data = (data << 8) | (*(uchar *)cp);
	    }
	  for (; i<4 && cnt>0; ++i)
	    {
	      data = (data << 8) | *src++;
	      --cnt;
	      ++cp;
	    }
	  for (; cnt==0 && i<4; ++i, ++cp)
	    {
	      data = (data << 8) | (*(uchar *)cp);
	    }

	  if ( (rc = write_word(info, wp, data)) != 0 )
	    {
	      return (rc);
	    }
	  wp += 4;
	}

      /*
       * handle word aligned part
       */
      while ( cnt >= 4 )
	{
	  data = 0;
	  for (i=0; i<4; ++i)
	    {
	      data = (data << 8) | *src++;
	    }
	  if ( (rc = write_word(info, wp, data)) != 0 )
	    {
	      return (rc);
	    }
	  wp  += 4;
	  cnt -= 4;
	}

      if ( cnt == 0 )
	{
	  return (0);
	}

      /*
       * handle unaligned tail bytes
       */
      data = 0;
      for (i=0, cp=wp; i<4 && cnt>0; ++i, ++cp)
	{
	  data = (data << 8) | *src++;
	  --cnt;
	}
      for (; i<4; ++i, ++cp)
	{
	  data = (data << 8) | (*(uchar *)cp);
	}

      return (write_word(info, wp, data));
    }
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int
write_word (flash_info_t  *info,
	    ulong          dest,
	    ulong          data)
{
  vu_long *addr = (vu_long*)(info->start[0]);
  ulong start;
  int flag;

  /* Check if Flash is (sufficiently) erased */
  if ( (*((vu_long *)dest) & data) != data )
    {
      return (2);
    }
  /* Disable interrupts which might cause a timeout here */
  flag = disable_interrupts();

  addr[0x0555] = 0x00AA00AA;
  addr[0x02AA] = 0x00550055;
  addr[0x0555] = 0x00A000A0;

  *((vu_long *)dest) = data;

  /* re-enable interrupts if necessary */
  if ( flag )
    enable_interrupts();

  /* data polling for D7 */
  start = get_timer (0);
  while ( (*((vu_long *)dest) & 0x00800080) != (data & 0x00800080) )
    {
      if ( get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT )
	{
	  return (1);
	}
    }

  return (0);
}

/*-----------------------------------------------------------------------
 * Write a byte to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int
write_byte (flash_info_t  *info,
	    ulong          dest,
	    uchar          data)
{
  volatile unsigned char *addr = (volatile unsigned char *)(info->start[0]);
  ulong  start;
  int    flag;

  /* Check if Flash is (sufficiently) erased */
  if ( (*((volatile unsigned char *)dest) & data) != data )
    {
      return (2);
    }
  /* Disable interrupts which might cause a timeout here */
  flag = disable_interrupts();

  addr[0x0555] = 0xAA;
  addr[0x02AA] = 0x55;
  addr[0x0555] = 0xA0;

  *((volatile unsigned char *)dest) = data;

  /* re-enable interrupts if necessary */
  if ( flag )
    enable_interrupts();

  /* data polling for D7 */
  start = get_timer (0);
  while ( (*((volatile unsigned char *)dest) & 0x80) != (data & 0x80) )
    {
      if ( get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT )
	{
	  return (1);
	}
    }

  return (0);
}

/*-----------------------------------------------------------------------
 */
