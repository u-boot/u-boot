/*
 * (C) Copyright 2001-2004
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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

#include <common.h>
#include <asm/processor.h>
#include <command.h>

/* ------------------------------------------------------------------------- */

#ifdef FPGA_DEBUG
#define DBG(x...) printf(x)
#else
#define DBG(x...)
#endif /* DEBUG */

#define MAX_ONES               226

#ifdef CFG_FPGA_PRG
# define FPGA_PRG              CFG_FPGA_PRG /* FPGA program pin (ppc output)*/
# define FPGA_CLK              CFG_FPGA_CLK /* FPGA clk pin (ppc output)    */
# define FPGA_DATA             CFG_FPGA_DATA /* FPGA data pin (ppc output)  */
# define FPGA_DONE             CFG_FPGA_DONE /* FPGA done pin (ppc input)   */
# define FPGA_INIT             CFG_FPGA_INIT /* FPGA init pin (ppc input)   */
#else
# define FPGA_PRG              0x04000000  /* FPGA program pin (ppc output) */
# define FPGA_CLK              0x02000000  /* FPGA clk pin (ppc output)     */
# define FPGA_DATA             0x01000000  /* FPGA data pin (ppc output)    */
# define FPGA_DONE             0x00800000  /* FPGA done pin (ppc input)     */
# define FPGA_INIT             0x00400000  /* FPGA init pin (ppc input)     */
#endif

#define ERROR_FPGA_PRG_INIT_LOW  -1        /* Timeout after PRG* asserted   */
#define ERROR_FPGA_PRG_INIT_HIGH -2        /* Timeout after PRG* deasserted */
#define ERROR_FPGA_PRG_DONE      -3        /* Timeout after programming     */

#ifndef SET_FPGA
# define SET_FPGA(data)         out32(GPIO0_OR, data)
#endif

#ifdef FPGA_PROG_ACTIVE_HIGH
# define FPGA_PRG_LOW           FPGA_PRG
# define FPGA_PRG_HIGH          0
#else
# define FPGA_PRG_LOW           0
# define FPGA_PRG_HIGH          FPGA_PRG
#endif

#define FPGA_CLK_LOW            0
#define FPGA_CLK_HIGH           FPGA_CLK

#define FPGA_DATA_LOW           0
#define FPGA_DATA_HIGH          FPGA_DATA

#define FPGA_WRITE_1 {                                                                   \
	SET_FPGA(FPGA_PRG_HIGH | FPGA_CLK_LOW  | FPGA_DATA_HIGH);  /* set clock to 0 */  \
	SET_FPGA(FPGA_PRG_HIGH | FPGA_CLK_LOW  | FPGA_DATA_HIGH);  /* set data to 1  */  \
	SET_FPGA(FPGA_PRG_HIGH | FPGA_CLK_HIGH | FPGA_DATA_HIGH);  /* set clock to 1 */  \
	SET_FPGA(FPGA_PRG_HIGH | FPGA_CLK_HIGH | FPGA_DATA_HIGH);} /* set data to 1  */

#define FPGA_WRITE_0 {                                                    \
	SET_FPGA(FPGA_PRG_HIGH | FPGA_CLK_LOW  | FPGA_DATA_HIGH);  /* set clock to 0 */  \
	SET_FPGA(FPGA_PRG_HIGH | FPGA_CLK_LOW  | FPGA_DATA_LOW);   /* set data to 0  */  \
	SET_FPGA(FPGA_PRG_HIGH | FPGA_CLK_HIGH | FPGA_DATA_LOW);   /* set clock to 1 */  \
	SET_FPGA(FPGA_PRG_HIGH | FPGA_CLK_HIGH | FPGA_DATA_HIGH);} /* set data to 1  */

#ifndef FPGA_DONE_STATE
# define FPGA_DONE_STATE (in32(GPIO0_IR) & FPGA_DONE)
#endif
#ifndef FPGA_INIT_STATE
# define FPGA_INIT_STATE (in32(GPIO0_IR) & FPGA_INIT)
#endif


static int fpga_boot(const unsigned char *fpgadata, int size)
{
  int i,index,len;
  int count;
#ifdef CFG_FPGA_SPARTAN2
  int j;
#else
  unsigned char b;
  int bit;
#endif

  /* display infos on fpgaimage */
  index = 15;
  for (i=0; i<4; i++)
    {
      len = fpgadata[index];
      DBG("FPGA: %s\n", &(fpgadata[index+1]));
      index += len+3;
    }

#ifdef CFG_FPGA_SPARTAN2
  /* search for preamble 0xFFFFFFFF */
  while (1)
    {
      if ((fpgadata[index] == 0xff) && (fpgadata[index+1] == 0xff) &&
	  (fpgadata[index+2] == 0xff) && (fpgadata[index+3] == 0xff))
	break; /* preamble found */
      else
	index++;
    }
#else
  /* search for preamble 0xFF2X */
  for (index = 0; index < size-1 ; index++)
    {
      if ((fpgadata[index] == 0xff) && ((fpgadata[index+1] & 0xf0) == 0x30))
	break;
    }
  index += 2;
#endif

  DBG("FPGA: configdata starts at position 0x%x\n",index);
  DBG("FPGA: length of fpga-data %d\n", size-index);

  /*
   * Setup port pins for fpga programming
   */
#ifndef CONFIG_M5249
  out32(GPIO0_ODR, 0x00000000);                                      /* no open drain pins */
  out32(GPIO0_TCR, in32(GPIO0_TCR) | FPGA_PRG | FPGA_CLK | FPGA_DATA); /* setup for output */
#endif
  SET_FPGA(FPGA_PRG_HIGH | FPGA_CLK_HIGH | FPGA_DATA_HIGH);            /* set pins to high */

  DBG("%s, ",(FPGA_DONE_STATE == 0) ? "NOT DONE" : "DONE" );
  DBG("%s\n",(FPGA_INIT_STATE == 0) ? "NOT INIT" : "INIT" );

  /*
   * Init fpga by asserting and deasserting PROGRAM*
   */
  SET_FPGA(FPGA_PRG_LOW  | FPGA_CLK_HIGH | FPGA_DATA_HIGH);             /* set prog active */

  /* Wait for FPGA init line low */
  count = 0;
  while (FPGA_INIT_STATE)
    {
      udelay(1000); /* wait 1ms */
      /* Check for timeout - 100us max, so use 3ms */
      if (count++ > 3)
	{
	  DBG("FPGA: Booting failed!\n");
	  return ERROR_FPGA_PRG_INIT_LOW;
	}
    }

  DBG("%s, ",(FPGA_DONE_STATE == 0) ? "NOT DONE" : "DONE" );
  DBG("%s\n",(FPGA_INIT_STATE == 0) ? "NOT INIT" : "INIT" );

  /* deassert PROGRAM* */
  SET_FPGA(FPGA_PRG_HIGH | FPGA_CLK_HIGH | FPGA_DATA_HIGH);           /* set prog inactive */

  /* Wait for FPGA end of init period .  */
  count = 0;
  while (!(FPGA_INIT_STATE))
    {
      udelay(1000); /* wait 1ms */
      /* Check for timeout */
      if (count++ > 3)
	{
	  DBG("FPGA: Booting failed!\n");
	  return ERROR_FPGA_PRG_INIT_HIGH;
	}
    }

  DBG("%s, ",(FPGA_DONE_STATE == 0) ? "NOT DONE" : "DONE" );
  DBG("%s\n",(FPGA_INIT_STATE == 0) ? "NOT INIT" : "INIT" );

  DBG("write configuration data into fpga\n");
  /* write configuration-data into fpga... */

#ifdef CFG_FPGA_SPARTAN2
  /*
   * Load uncompressed image into fpga
   */
  for (i=index; i<size; i++)
    {
      for (j=0; j<8; j++)
	{
	  if ((fpgadata[i] & 0x80) == 0x80)
	    {
	      FPGA_WRITE_1;
	    }
	  else
	    {
	      FPGA_WRITE_0;
	    }
	  fpgadata[i] <<= 1;
	}
    }
#else
  /* send 0xff 0x20 */
  FPGA_WRITE_1; FPGA_WRITE_1; FPGA_WRITE_1; FPGA_WRITE_1;
  FPGA_WRITE_1; FPGA_WRITE_1; FPGA_WRITE_1; FPGA_WRITE_1;
  FPGA_WRITE_0; FPGA_WRITE_0; FPGA_WRITE_1; FPGA_WRITE_0;
  FPGA_WRITE_0; FPGA_WRITE_0; FPGA_WRITE_0; FPGA_WRITE_0;

  /*
  ** Bit_DeCompression
  **   Code 1           .. maxOnes     : n                 '1's followed by '0'
  **        maxOnes + 1 .. maxOnes + 1 : n - 1             '1's no '0'
  **        maxOnes + 2 .. 254         : n - (maxOnes + 2) '0's followed by '1'
  **        255                        :                   '1'
  */

  for (i=index; i<size; i++)
    {
      b = fpgadata[i];
      if ((b >= 1) && (b <= MAX_ONES))
	{
	  for(bit=0; bit<b; bit++)
	    {
	      FPGA_WRITE_1;
	    }
	  FPGA_WRITE_0;
	}
      else if (b == (MAX_ONES+1))
	{
	  for(bit=1; bit<b; bit++)
	    {
	      FPGA_WRITE_1;
	    }
	}
      else if ((b >= (MAX_ONES+2)) && (b <= 254))
	{
	  for(bit=0; bit<(b-(MAX_ONES+2)); bit++)
	    {
	      FPGA_WRITE_0;
	    }
	  FPGA_WRITE_1;
	}
      else if (b == 255)
	{
	  FPGA_WRITE_1;
	}
    }
#endif

  DBG("%s, ",(FPGA_DONE_STATE == 0) ? "NOT DONE" : "DONE" );
  DBG("%s\n",(FPGA_INIT_STATE == 0) ? "NOT INIT" : "INIT" );

  /*
   * Check if fpga's DONE signal - correctly booted ?
   */

  /* Wait for FPGA end of programming period .  */
  count = 0;
  while (!(FPGA_DONE_STATE))
    {
      udelay(1000); /* wait 1ms */
      /* Check for timeout */
      if (count++ > 3)
	{
	  DBG("FPGA: Booting failed!\n");
	  return ERROR_FPGA_PRG_DONE;
	}
    }

  DBG("FPGA: Booting successful!\n");
  return 0;
}
