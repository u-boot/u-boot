/****************************************************************************
 *  SPI flash driver for M25P64
 ****************************************************************************/
#include <common.h>
#include <linux/ctype.h>
#include <asm/io.h>
#include <asm/mach-common/bits/spi.h>

#if defined(CONFIG_SPI)

/* Application definitions */

#define	NUM_SECTORS	128	/* number of sectors */
#define SECTOR_SIZE	0x10000
#define NOP_NUM		1000

#define COMMON_SPI_SETTINGS (SPE|MSTR|CPHA|CPOL) /* Settings to the SPI_CTL */
#define TIMOD01 (0x01)	/* stes the SPI to work with core instructions */

/* Flash commands */
#define	SPI_WREN	(0x06)	/*Set Write Enable Latch */
#define	SPI_WRDI	(0x04)	/*Reset Write Enable Latch */
#define	SPI_RDSR	(0x05)	/*Read Status Register */
#define	SPI_WRSR	(0x01)	/*Write Status Register */
#define	SPI_READ	(0x03)	/*Read data from memory */
#define	SPI_FAST_READ	(0x0B)	/*Read data from memory */
#define	SPI_PP		(0x02)	/*Program Data into memory */
#define	SPI_SE		(0xD8)	/*Erase one sector in memory */
#define	SPI_BE		(0xC7)	/*Erase all memory */
#define	WIP		(0x1)	/*Check the write in progress bit of the SPI status register */
#define	WEL		(0x2)	/*Check the write enable bit of the SPI status register */

#define	TIMEOUT		350000000

typedef enum {
	NO_ERR,
	POLL_TIMEOUT,
	INVALID_SECTOR,
	INVALID_BLOCK,
} ERROR_CODE;

void spi_init_f(void);
void spi_init_r(void);
ssize_t spi_read(uchar *, int, uchar *, int);
ssize_t spi_write(uchar *, int, uchar *, int);

char ReadStatusRegister(void);
void Wait_For_SPIF(void);
void SetupSPI(const int spi_setting);
void SPI_OFF(void);
void SendSingleCommand(const int iCommand);

ERROR_CODE GetSectorNumber(unsigned long ulOffset, int *pnSector);
ERROR_CODE EraseBlock(int nBlock);
ERROR_CODE ReadData(unsigned long ulStart, long lCount, int *pnData);
ERROR_CODE WriteData(unsigned long ulStart, long lCount, int *pnData);
ERROR_CODE Wait_For_Status(char Statusbit);
ERROR_CODE Wait_For_WEL(void);

/*
 * Function:    spi_init_f
 * Description: Init SPI-Controller (ROM part)
 * return:      ---
 */
void spi_init_f(void)
{
}

/*
 * Function:    spi_init_r
 * Description: Init SPI-Controller (RAM part) -
 *		 The malloc engine is ready and we can move our buffers to
 *		 normal RAM
 *  return:      ---
 */
void spi_init_r(void)
{
	return;
}

/*
 * Function:    spi_write
 */
ssize_t spi_write(uchar * addr, int alen, uchar * buffer, int len)
{
	unsigned long offset;
	int start_block, end_block;
	int start_byte, end_byte;
	ERROR_CODE result = NO_ERR;
	uchar temp[SECTOR_SIZE];
	int i, num;

	offset = addr[0] << 16 | addr[1] << 8 | addr[2];
	/* Get the start block number */
	result = GetSectorNumber(offset, &start_block);
	if (result == INVALID_SECTOR) {
		printf("Invalid sector! ");
		return 0;
	}
	/* Get the end block number */
	result = GetSectorNumber(offset + len - 1, &end_block);
	if (result == INVALID_SECTOR) {
		printf("Invalid sector! ");
		return 0;
	}

	for (num = start_block; num <= end_block; num++) {
		ReadData(num * SECTOR_SIZE, SECTOR_SIZE, (int *)temp);
		start_byte = num * SECTOR_SIZE;
		end_byte = (num + 1) * SECTOR_SIZE - 1;
		if (start_byte < offset)
			start_byte = offset;
		if (end_byte > (offset + len))
			end_byte = (offset + len - 1);
		for (i = start_byte; i <= end_byte; i++)
			temp[i - num * SECTOR_SIZE] = buffer[i - offset];
		EraseBlock(num);
		result = WriteData(num * SECTOR_SIZE, SECTOR_SIZE, (int *)temp);
		if (result != NO_ERR)
			return 0;
		printf(".");
	}
	return len;
}

/*
 * Function: spi_read
 */
ssize_t spi_read(uchar * addr, int alen, uchar * buffer, int len)
{
	unsigned long offset;
	offset = addr[0] << 16 | addr[1] << 8 | addr[2];
	ReadData(offset, len, (int *)buffer);
	return len;
}

void SendSingleCommand(const int iCommand)
{
	unsigned short dummy;

	/* turns on the SPI in single write mode */
	SetupSPI((COMMON_SPI_SETTINGS | TIMOD01));

	/* sends the actual command to the SPI TX register */
	*pSPI_TDBR = iCommand;
	SSYNC();

	/* The SPI status register will be polled to check the SPIF bit */
	Wait_For_SPIF();

	dummy = *pSPI_RDBR;

	/* The SPI will be turned off */
	SPI_OFF();

}

void SetupSPI(const int spi_setting)
{

	if (icache_status() || dcache_status())
		udelay(CONFIG_CCLK_HZ / 50000000);
	/*sets up the PF10 to be the slave select of the SPI */
	*pPORTF_FER |= (PF10 | PF11 | PF12 | PF13);
	*pSPI_FLG = 0xFF02;
	*pSPI_BAUD = CONFIG_SPI_BAUD;
	*pSPI_CTL = spi_setting;
	SSYNC();

	*pSPI_FLG = 0xFD02;
	SSYNC();
}

void SPI_OFF(void)
{

	*pSPI_CTL = 0x0400;	/* disable SPI */
	*pSPI_FLG = 0;
	*pSPI_BAUD = 0;
	SSYNC();
	udelay(CONFIG_CCLK_HZ / 50000000);

}

void Wait_For_SPIF(void)
{
	unsigned short dummyread;
	while ((*pSPI_STAT & TXS)) ;
	while (!(*pSPI_STAT & SPIF)) ;
	while (!(*pSPI_STAT & RXS)) ;
	/* Read dummy to empty the receive register */
	dummyread = *pSPI_RDBR;
}

ERROR_CODE Wait_For_WEL(void)
{
	int i;
	char status_register = 0;
	ERROR_CODE ErrorCode = NO_ERR;

	for (i = 0; i < TIMEOUT; i++) {
		status_register = ReadStatusRegister();
		if ((status_register & WEL)) {
			ErrorCode = NO_ERR;
			break;
		}
		ErrorCode = POLL_TIMEOUT;	/* Time out error */
	};

	return ErrorCode;
}

ERROR_CODE Wait_For_Status(char Statusbit)
{
	int i;
	char status_register = 0xFF;
	ERROR_CODE ErrorCode = NO_ERR;

	for (i = 0; i < TIMEOUT; i++) {
		status_register = ReadStatusRegister();
		if (!(status_register & Statusbit)) {
			ErrorCode = NO_ERR;
			break;
		}
		ErrorCode = POLL_TIMEOUT;	/* Time out error */
	};

	return ErrorCode;
}

char ReadStatusRegister(void)
{
	char status_register = 0;

	SetupSPI((COMMON_SPI_SETTINGS | TIMOD01));	/* Turn on the SPI */

	*pSPI_TDBR = SPI_RDSR;	/* send instruction to read status register */
	SSYNC();
	Wait_For_SPIF();	/*wait until the instruction has been sent */
	*pSPI_TDBR = 0;		/*send dummy to receive the status register */
	SSYNC();
	Wait_For_SPIF();	/*wait until the data has been sent */
	status_register = *pSPI_RDBR;	/*read the status register */

	SPI_OFF();		/* Turn off the SPI */

	return status_register;
}

ERROR_CODE GetSectorNumber(unsigned long ulOffset, int *pnSector)
{
	int nSector = 0;
	ERROR_CODE ErrorCode = NO_ERR;

	if (ulOffset > (NUM_SECTORS * 0x10000 - 1)) {
		ErrorCode = INVALID_SECTOR;
		return ErrorCode;
	}

	nSector = (int)ulOffset / 0x10000;
	*pnSector = nSector;

	return ErrorCode;
}

ERROR_CODE EraseBlock(int nBlock)
{
	unsigned long ulSectorOff = 0x0, ShiftValue;
	ERROR_CODE ErrorCode = NO_ERR;

	/* if the block is invalid just return */
	if ((nBlock < 0) || (nBlock > NUM_SECTORS)) {
		ErrorCode = INVALID_BLOCK;
		return ErrorCode;
	}
	/* figure out the offset of the block in flash */
	if ((nBlock >= 0) && (nBlock < NUM_SECTORS)) {
		ulSectorOff = (nBlock * SECTOR_SIZE);

	} else {
		ErrorCode = INVALID_BLOCK;
		return ErrorCode;
	}

	/* A write enable instruction must previously have been executed */
	SendSingleCommand(SPI_WREN);

	/* The status register will be polled to check the write enable latch "WREN" */
	ErrorCode = Wait_For_WEL();

	if (POLL_TIMEOUT == ErrorCode) {
		printf("SPI Erase block error\n");
		return ErrorCode;
	} else

	/* Turn on the SPI to send single commands */
	SetupSPI((COMMON_SPI_SETTINGS | TIMOD01));

	/*
	 * Send the erase block command to the flash followed by the 24 address
	 * to point to the start of a sector
	 */
	*pSPI_TDBR = SPI_SE;
	SSYNC();
	Wait_For_SPIF();
	/* Send the highest byte of the 24 bit address at first */
	ShiftValue = (ulSectorOff >> 16);
	*pSPI_TDBR = ShiftValue;
	SSYNC();
	/* Wait until the instruction has been sent */
	Wait_For_SPIF();
	/* Send the middle byte of the 24 bit address  at second */
	ShiftValue = (ulSectorOff >> 8);
	*pSPI_TDBR = ShiftValue;
	SSYNC();
	/* Wait until the instruction has been sent */
	Wait_For_SPIF();
	/* Send the lowest byte of the 24 bit address finally */
	*pSPI_TDBR = ulSectorOff;
	SSYNC();
	/* Wait until the instruction has been sent */
	Wait_For_SPIF();

	/* Turns off the SPI */
	SPI_OFF();

	/* Poll the status register to check the Write in Progress bit */
	/* Sector erase takes time */
	ErrorCode = Wait_For_Status(WIP);

	/* block erase should be complete */
	return ErrorCode;
}

/*
 * ERROR_CODE ReadData()
 * Read a value from flash for verify purpose
 * Inputs:	unsigned long ulStart - holds the SPI start address
 *			int pnData - pointer to store value read from flash
 *			long lCount - number of elements to read
 */
ERROR_CODE ReadData(unsigned long ulStart, long lCount, int *pnData)
{
	unsigned long ShiftValue;
	char *cnData;
	int i;

	/* Pointer cast to be able to increment byte wise */

	cnData = (char *)pnData;
	/* Start SPI interface */
	SetupSPI((COMMON_SPI_SETTINGS | TIMOD01));

#ifdef CONFIG_SPI_FLASH_FAST_READ
	/* Send the read command to SPI device */
	*pSPI_TDBR = SPI_FAST_READ;
#else
	/* Send the read command to SPI device */
	*pSPI_TDBR = SPI_READ;
#endif
	SSYNC();
	/* Wait until the instruction has been sent */
	Wait_For_SPIF();
	/* Send the highest byte of the 24 bit address at first */
	ShiftValue = (ulStart >> 16);
	/* Send the byte to the SPI device */
	*pSPI_TDBR = ShiftValue;
	SSYNC();
	/* Wait until the instruction has been sent */
	Wait_For_SPIF();
	/* Send the middle byte of the 24 bit address  at second */
	ShiftValue = (ulStart >> 8);
	/* Send the byte to the SPI device */
	*pSPI_TDBR = ShiftValue;
	SSYNC();
	/* Wait until the instruction has been sent */
	Wait_For_SPIF();
	/* Send the lowest byte of the 24 bit address finally */
	*pSPI_TDBR = ulStart;
	SSYNC();
	/* Wait until the instruction has been sent */
	Wait_For_SPIF();

#ifdef CONFIG_SPI_FLASH_FAST_READ
	/* Send dummy for FAST_READ */
	*pSPI_TDBR = 0;
	SSYNC();
	/* Wait until the instruction has been sent */
	Wait_For_SPIF();
#endif

	/* After the SPI device address has been placed on the MOSI pin the data can be */
	/* received on the MISO pin. */
	for (i = 0; i < lCount; i++) {
		*pSPI_TDBR = 0;
		SSYNC();
		while (!(*pSPI_STAT & RXS)) ;
		*cnData++ = *pSPI_RDBR;

		if ((i >= SECTOR_SIZE) && (i % SECTOR_SIZE == 0))
			printf(".");
	}

	/* Turn off the SPI */
	SPI_OFF();

	return NO_ERR;
}

ERROR_CODE WriteFlash(unsigned long ulStartAddr, long lTransferCount,
		      int *iDataSource, long *lWriteCount)
{

	unsigned long ulWAddr;
	long lWTransferCount = 0;
	int i;
	char iData;
	char *temp = (char *)iDataSource;
	ERROR_CODE ErrorCode = NO_ERR;

	/* First, a Write Enable Command must be sent to the SPI. */
	SendSingleCommand(SPI_WREN);

	/*
	 * Second, the SPI Status Register will be tested whether the
	 * Write Enable Bit has been set
	 */
	ErrorCode = Wait_For_WEL();
	if (POLL_TIMEOUT == ErrorCode) {
		printf("SPI Write Time Out\n");
		return ErrorCode;
	} else
		/* Third, the 24 bit address will be shifted out
		 * the SPI MOSI bytewise.
		 * Turns the SPI on
		 */
		SetupSPI((COMMON_SPI_SETTINGS | TIMOD01));
	*pSPI_TDBR = SPI_PP;
	SSYNC();
	/*wait until the instruction has been sent */
	Wait_For_SPIF();
	ulWAddr = (ulStartAddr >> 16);
	*pSPI_TDBR = ulWAddr;
	SSYNC();
	/*wait until the instruction has been sent */
	Wait_For_SPIF();
	ulWAddr = (ulStartAddr >> 8);
	*pSPI_TDBR = ulWAddr;
	SSYNC();
	/*wait until the instruction has been sent */
	Wait_For_SPIF();
	ulWAddr = ulStartAddr;
	*pSPI_TDBR = ulWAddr;
	SSYNC();
	/*wait until the instruction has been sent */
	Wait_For_SPIF();
	/*
	 * Fourth, maximum number of 256 bytes will be taken from the Buffer
	 * and sent to the SPI device.
	 */
	for (i = 0; (i < lTransferCount) && (i < 256); i++, lWTransferCount++) {
		iData = *temp;
		*pSPI_TDBR = iData;
		SSYNC();
		/*wait until the instruction has been sent */
		Wait_For_SPIF();
		temp++;
	}

	/* Turns the SPI off */
	SPI_OFF();

	/*
	 * Sixth, the SPI Write in Progress Bit must be toggled to ensure the
	 * programming is done before start of next transfer
	 */
	ErrorCode = Wait_For_Status(WIP);

	if (POLL_TIMEOUT == ErrorCode) {
		printf("SPI Program Time out!\n");
		return ErrorCode;
	} else

		*lWriteCount = lWTransferCount;

	return ErrorCode;
}

ERROR_CODE WriteData(unsigned long ulStart, long lCount, int *pnData)
{

	unsigned long ulWStart = ulStart;
	long lWCount = lCount, lWriteCount;
	long *pnWriteCount = &lWriteCount;

	ERROR_CODE ErrorCode = NO_ERR;

	while (lWCount != 0) {
		ErrorCode = WriteFlash(ulWStart, lWCount, pnData, pnWriteCount);

		/*
		 * After each function call of WriteFlash the counter
		 * must be adjusted
		 */
		lWCount -= *pnWriteCount;

		/* Also, both address pointers must be recalculated. */
		ulWStart += *pnWriteCount;
		pnData += *pnWriteCount / 4;
	}

	/* return the appropriate error code */
	return ErrorCode;
}

#endif				/* CONFIG_SPI */
