/*************************************************************************
|  COPYRIGHT (c) 2000 BY ABATRON AG
|*************************************************************************
|
|  PROJECT NAME: Linux Image to S-record Conversion Utility
|  FILENAME    : img2srec.c
|
|  COMPILER    : GCC
|
|  TARGET OS   : LINUX / UNIX
|  TARGET HW   : -
|
|  PROGRAMMER  : Abatron / RD
|  CREATION    : 07.07.00
|
|*************************************************************************
|
|  DESCRIPTION :
|
|  Utility to convert a Linux Boot Image to S-record:
|  ==================================================
|
|  This command line utility can be used to convert a Linux boot image
|  (zimage.initrd) to S-Record format used for flash programming.
|  This conversion takes care of the special sections "IMAGE" and INITRD".
|
|  img2srec [-o offset] image > image.srec
|
|
|  Build the utility:
|  ==================
|
|  To build the utility use GCC as follows:
|
|  gcc img2srec.c -o img2srec
|
|
|*************************************************************************
|
|
|  UPDATES     :
|
|  DATE      NAME  CHANGES
|  -----------------------------------------------------------
|  Latest update
|
|  07.07.00  aba   Initial release
|
|*************************************************************************/

/*************************************************************************
|  INCLUDES
|*************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <elf.h>
#include <unistd.h>
#include <errno.h>

extern int errno;

/*************************************************************************
|  DEFINES
|*************************************************************************/

#define FALSE           0
#define TRUE            1

/*************************************************************************
|  MACROS
|*************************************************************************/

/*************************************************************************
|  TYPEDEFS
|*************************************************************************/

typedef uint8_t   CHAR;
typedef uint8_t   BYTE;
typedef uint16_t  WORD;
typedef uint32_t  DWORD;
typedef int       BOOL;

/*************************************************************************
|  LOCALS
|*************************************************************************/

/*************************************************************************
|  PROTOTYPES
|*************************************************************************/

static char *ExtractHex(DWORD *value, char *getPtr);
static char *ExtractDecimal(DWORD *value, char *getPtr);
static void ExtractNumber(DWORD *value, char *getPtr);
static BYTE *ExtractWord(WORD *value, BYTE *buffer);
static BYTE *ExtractLong(DWORD *value, BYTE *buffer);
static BYTE *ExtractBlock(WORD count, BYTE *data, BYTE *buffer);
static char *WriteHex(char *pa, BYTE value, WORD *pCheckSum);
static char *BuildSRecord(char *pa, WORD sType, DWORD addr,
			  const BYTE *data, int nCount);
static void ConvertELF(char *fileName, DWORD loadOffset);
int main(int argc, char *argv[]);

/*************************************************************************
|  FUNCTIONS
|*************************************************************************/

static char* ExtractHex (DWORD* value,  char* getPtr)
{
  DWORD num;
  DWORD digit;
  BYTE  c;

  while (*getPtr == ' ') getPtr++;
  num = 0;
  for (;;) {
    c = *getPtr;
    if      ((c >= '0') && (c <= '9')) digit = (DWORD)(c - '0');
    else if ((c >= 'A') && (c <= 'F')) digit = (DWORD)(c - 'A' + 10);
    else if ((c >= 'a') && (c <= 'f')) digit = (DWORD)(c - 'a' + 10);
    else break;
    num <<= 4;
    num += digit;
    getPtr++;
  } /* for */
  *value = num;
  return getPtr;
} /* ExtractHex */

static char* ExtractDecimal (DWORD* value,  char* getPtr)
{
  DWORD num;
  DWORD digit;
  BYTE  c;

  while (*getPtr == ' ') getPtr++;
  num = 0;
  for (;;) {
    c = *getPtr;
    if      ((c >= '0') && (c <= '9')) digit = (DWORD)(c - '0');
    else break;
    num *= 10;
    num += digit;
    getPtr++;
  } /* for */
  *value = num;
  return getPtr;
} /* ExtractDecimal */


static void ExtractNumber (DWORD* value,  char* getPtr)
{
  BOOL  neg = FALSE;;

  while (*getPtr == ' ') getPtr++;
  if (*getPtr == '-') {
    neg = TRUE;
    getPtr++;
  } /* if */
  if ((*getPtr == '0') && ((*(getPtr+1) == 'x') || (*(getPtr+1) == 'X'))) {
    getPtr +=2;
    (void)ExtractHex(value, getPtr);
  } /* if */
  else {
    (void)ExtractDecimal(value, getPtr);
  } /* else */
  if (neg) *value = -(*value);
} /* ExtractNumber */


static BYTE* ExtractWord(WORD* value, BYTE* buffer)
{
  WORD x;
  x = (WORD)*buffer++;
  x = (x<<8) + (WORD)*buffer++;
  *value = x;
  return buffer;
} /* ExtractWord */


static BYTE* ExtractLong(DWORD* value, BYTE* buffer)
{
  DWORD x;
  x = (DWORD)*buffer++;
  x = (x<<8) + (DWORD)*buffer++;
  x = (x<<8) + (DWORD)*buffer++;
  x = (x<<8) + (DWORD)*buffer++;
  *value = x;
  return buffer;
} /* ExtractLong */


static BYTE* ExtractBlock(WORD count, BYTE* data, BYTE* buffer)
{
  while (count--) *data++ = *buffer++;
  return buffer;
} /* ExtractBlock */


static char* WriteHex(char* pa, BYTE value, WORD* pCheckSum)
{
  WORD  temp;

  static  char ByteToHex[] = "0123456789ABCDEF";

  *pCheckSum += value;
  temp  = value / 16;
  *pa++ = ByteToHex[temp];
  temp  = value % 16;
  *pa++ = ByteToHex[temp];
  return pa;
}


static char* BuildSRecord(char* pa, WORD sType, DWORD addr,
			  const BYTE* data, int nCount)
{
  WORD  addrLen;
  WORD  sRLen;
  WORD  checkSum;
  WORD  i;

  switch (sType) {
  case 0:
  case 1:
  case 9:
    addrLen = 2;
    break;
  case 2:
  case 8:
    addrLen = 3;
    break;
  case 3:
  case 7:
    addrLen = 4;
    break;
  default:
    return pa;
  } /* switch */

  *pa++ = 'S';
  *pa++ = (char)(sType + '0');
  sRLen = addrLen + nCount + 1;
  checkSum = 0;
  pa = WriteHex(pa, (BYTE)sRLen, &checkSum);

  /* Write address field */
  for (i = 1; i <= addrLen; i++) {
    pa = WriteHex(pa, (BYTE)(addr >> (8 * (addrLen - i))), &checkSum);
  } /* for */

  /* Write code/data fields */
  for (i = 0; i < nCount; i++) {
    pa = WriteHex(pa, *data++, &checkSum);
  } /* for */

  /* Write checksum field */
  checkSum = ~checkSum;
  pa = WriteHex(pa, (BYTE)checkSum, &checkSum);
  *pa++ = '\0';
  return pa;
}


static void ConvertELF(char* fileName, DWORD loadOffset)
{
  FILE*         file;
  int           i;
  int           rxCount;
  BYTE          rxBlock[1024];
  DWORD         loadSize;
  DWORD         firstAddr;
  DWORD         loadAddr;
  DWORD         loadDiff = 0;
  Elf32_Ehdr    elfHeader;
  Elf32_Shdr    sectHeader[32];
  BYTE*         getPtr;
  char          srecLine[128];
  char		*hdr_name;


  /* open file */
  if ((file = fopen(fileName,"rb")) == NULL) {
    fprintf (stderr, "Can't open %s: %s\n", fileName, strerror(errno));
    return;
  } /* if */

  /* read ELF header */
  rxCount = fread(rxBlock, 1, sizeof elfHeader, file);
  getPtr = ExtractBlock(sizeof elfHeader.e_ident, elfHeader.e_ident, rxBlock);
  getPtr = ExtractWord(&elfHeader.e_type, getPtr);
  getPtr = ExtractWord(&elfHeader.e_machine, getPtr);
  getPtr = ExtractLong((DWORD *)&elfHeader.e_version, getPtr);
  getPtr = ExtractLong((DWORD *)&elfHeader.e_entry, getPtr);
  getPtr = ExtractLong((DWORD *)&elfHeader.e_phoff, getPtr);
  getPtr = ExtractLong((DWORD *)&elfHeader.e_shoff, getPtr);
  getPtr = ExtractLong((DWORD *)&elfHeader.e_flags, getPtr);
  getPtr = ExtractWord(&elfHeader.e_ehsize, getPtr);
  getPtr = ExtractWord(&elfHeader.e_phentsize, getPtr);
  getPtr = ExtractWord(&elfHeader.e_phnum, getPtr);
  getPtr = ExtractWord(&elfHeader.e_shentsize, getPtr);
  getPtr = ExtractWord(&elfHeader.e_shnum, getPtr);
  getPtr = ExtractWord(&elfHeader.e_shstrndx, getPtr);
  if (    (rxCount              != sizeof elfHeader)
       || (elfHeader.e_ident[0] != ELFMAG0)
       || (elfHeader.e_ident[1] != ELFMAG1)
       || (elfHeader.e_ident[2] != ELFMAG2)
       || (elfHeader.e_ident[3] != ELFMAG3)
       || (elfHeader.e_type     != ET_EXEC)
     ) {
    fclose(file);
    fprintf (stderr, "*** illegal file format\n");
    return;
  } /* if */

  /* read all section headers */
  fseek(file, elfHeader.e_shoff, SEEK_SET);
  for (i = 0; i < elfHeader.e_shnum; i++) {
    rxCount = fread(rxBlock, 1, sizeof sectHeader[0], file);
    getPtr = ExtractLong((DWORD *)&sectHeader[i].sh_name, rxBlock);
    getPtr = ExtractLong((DWORD *)&sectHeader[i].sh_type, getPtr);
    getPtr = ExtractLong((DWORD *)&sectHeader[i].sh_flags, getPtr);
    getPtr = ExtractLong((DWORD *)&sectHeader[i].sh_addr, getPtr);
    getPtr = ExtractLong((DWORD *)&sectHeader[i].sh_offset, getPtr);
    getPtr = ExtractLong((DWORD *)&sectHeader[i].sh_size, getPtr);
    getPtr = ExtractLong((DWORD *)&sectHeader[i].sh_link, getPtr);
    getPtr = ExtractLong((DWORD *)&sectHeader[i].sh_info, getPtr);
    getPtr = ExtractLong((DWORD *)&sectHeader[i].sh_addralign, getPtr);
    getPtr = ExtractLong((DWORD *)&sectHeader[i].sh_entsize, getPtr);
    if (rxCount != sizeof sectHeader[0]) {
      fclose(file);
      fprintf (stderr, "*** illegal file format\n");
      return;
    } /* if */
  } /* for */

  if ((hdr_name = strrchr(fileName, '/')) == NULL) {
    hdr_name = fileName;
  } else {
    ++hdr_name;
  }
  /* write start record */
  (void)BuildSRecord(srecLine, 0, 0, (BYTE *)hdr_name, strlen(hdr_name));
  printf("%s\r\n",srecLine);

  /* write data records */
  firstAddr = ~0;
  loadAddr  =  0;
  for (i = 0; i < elfHeader.e_shnum; i++) {
    if (    (sectHeader[i].sh_type == SHT_PROGBITS)
         && (sectHeader[i].sh_size != 0)
         ) {
      loadSize = sectHeader[i].sh_size;
      if (sectHeader[i].sh_flags != 0) {
        loadAddr = sectHeader[i].sh_addr;
        loadDiff = loadAddr - sectHeader[i].sh_offset;
      } /* if */
      else {
        loadAddr = sectHeader[i].sh_offset + loadDiff;
      } /* else */

      if (loadAddr < firstAddr)
        firstAddr = loadAddr;

      /* build s-records */
      loadSize = sectHeader[i].sh_size;
      fseek(file, sectHeader[i].sh_offset, SEEK_SET);
      while (loadSize) {
        rxCount = fread(rxBlock, 1, (loadSize > 32) ? 32 : loadSize, file);
        if (rxCount < 0) {
          fclose(file);
          fprintf (stderr, "*** illegal file format\n");
        return;
        } /* if */
        (void)BuildSRecord(srecLine, 3, loadAddr + loadOffset, rxBlock, rxCount);
        loadSize -= rxCount;
        loadAddr += rxCount;
        printf("%s\r\n",srecLine);
      } /* while */
    } /* if */
  } /* for */

  /* add end record */
  (void)BuildSRecord(srecLine, 7, firstAddr + loadOffset, 0, 0);
  printf("%s\r\n",srecLine);
  fclose(file);
} /* ConvertELF */


/*************************************************************************
|  MAIN
|*************************************************************************/

int main( int argc, char *argv[ ])
{
  DWORD offset;

  if (argc == 2) {
    ConvertELF(argv[1], 0);
  } /* if */
  else if ((argc == 4) && (strcmp(argv[1], "-o") == 0)) {
    ExtractNumber(&offset, argv[2]);
    ConvertELF(argv[3], offset);
  } /* if */
  else {
    fprintf (stderr, "Usage: img2srec [-o offset] <image>\n");
  } /* if */

  return 0;
} /* main */
