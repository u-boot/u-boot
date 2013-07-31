/*
 * (C) Copyright 2001
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#define EEPROM_CAP              0x50000358
#define EEPROM_DATA             0x5000035c


unsigned int eepromReadLong(int offs)
{
  unsigned int value;
  unsigned short ret;
  int count;

  out_be16((void *)EEPROM_CAP, offs);

  count = 0;

  for (;;)
    {
      count++;
      ret = in_be16((void *)EEPROM_CAP);

      if ((ret & 0x8000) != 0)
	break;
    }

  value = in_be32((void *)EEPROM_DATA);

  return value;
}


unsigned char eepromReadByte(int offs)
{
  unsigned int valueLong;
  unsigned char *ptr;

  valueLong = eepromReadLong(offs & ~3);
  ptr = (unsigned char *)&valueLong;

  return ptr[offs & 3];
}


void eepromWriteLong(int offs, unsigned int value)
{
  unsigned short ret;
  int count;

  count = 0;

  out_be32((void *)EEPROM_DATA, value);
  out_be16((void *)EEPROM_CAP, 0x8000 + offs);

  for (;;)
    {
      count++;
      ret = in_be16((void *)EEPROM_CAP);

      if ((ret & 0x8000) == 0)
	break;
    }
}


void eepromWriteByte(int offs, unsigned char valueByte)
{
  unsigned int valueLong;
  unsigned char *ptr;

  valueLong = eepromReadLong(offs & ~3);
  ptr = (unsigned char *)&valueLong;

  ptr[offs & 3] = valueByte;

  eepromWriteLong(offs & ~3, valueLong);
}


void i2c_read (uchar *addr, int alen, uchar *buffer, int len)
{
  int i;
  int len2, ptr;

  /*  printf("\naddr=%x alen=%x buffer=%x len=%x", addr[0], addr[1], *(short *)addr, alen, buffer, len); /###* test-only */

  ptr = *(short *)addr;

  /*
   * Read till lword boundary
   */
  len2 = 4 - (*(short *)addr & 0x0003);
  for (i=0; i<len2; i++)
    {
      *buffer++ = eepromReadByte(ptr++);
    }

  /*
   * Read all lwords
   */
  len2 = (len - len2) >> 2;
  for (i=0; i<len2; i++)
    {
      *(unsigned int *)buffer = eepromReadLong(ptr);
      buffer += 4;
      ptr += 4;
    }

  /*
   * Read last bytes
   */
  len2 = (*(short *)addr + len) & 0x0003;
  for (i=0; i<len2; i++)
    {
      *buffer++ = eepromReadByte(ptr++);
    }
}

void i2c_write (uchar *addr, int alen, uchar *buffer, int len)
{
  int i;
  int len2, ptr;

  /*  printf("\naddr=%x alen=%x buffer=%x len=%x", addr[0], addr[1], *(short *)addr, alen, buffer, len); /###* test-only */

  ptr = *(short *)addr;

  /*
   * Write till lword boundary
   */
  len2 = 4 - (*(short *)addr & 0x0003);
  for (i=0; i<len2; i++)
    {
      eepromWriteByte(ptr++, *buffer++);
    }

  /*
   * Write all lwords
   */
  len2 = (len - len2) >> 2;
  for (i=0; i<len2; i++)
    {
      eepromWriteLong(ptr, *(unsigned int *)buffer);
      buffer += 4;
      ptr += 4;
    }

  /*
   * Write last bytes
   */
  len2 = (*(short *)addr + len) & 0x0003;
  for (i=0; i<len2; i++)
    {
      eepromWriteByte(ptr++, *buffer++);
    }
}
