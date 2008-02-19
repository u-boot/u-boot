/*
 * SPI flash driver
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

/* Configuration options:
 * CONFIG_SPI_BAUD - value to load into SPI_BAUD (divisor of SCLK to get SPI CLK)
 * CONFIG_SPI_FLASH_SLOW_READ - force usage of the slower read
 *		WARNING: make sure your SCLK + SPI_BAUD is slow enough
 */

#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/mach-common/bits/spi.h>

/* Forcibly phase out these */
#ifdef CONFIG_SPI_FLASH_NUM_SECTORS
# error do not set CONFIG_SPI_FLASH_NUM_SECTORS
#endif
#ifdef CONFIG_SPI_FLASH_SECTOR_SIZE
# error do not set CONFIG_SPI_FLASH_SECTOR_SIZE
#endif

#if defined(CONFIG_SPI)

struct flash_info {
	char     *name;
	uint16_t id;
	unsigned sector_size;
	unsigned num_sectors;
};

/* SPI Speeds: 50 MHz / 33 MHz */
static struct flash_info flash_spansion_serial_flash[] = {
	{ "S25FL016", 0x0215, 64 * 1024, 32 },
	{ "S25FL032", 0x0216, 64 * 1024, 64 },
	{ "S25FL064", 0x0217, 64 * 1024, 128 },
	{ "S25FL0128", 0x0218, 256 * 1024, 64 },
	{ NULL, 0, 0, 0 }
};

/* SPI Speeds: 50 MHz / 20 MHz */
static struct flash_info flash_st_serial_flash[] = {
	{ "m25p05", 0x2010, 32 * 1024, 2 },
	{ "m25p10", 0x2011, 32 * 1024, 4 },
	{ "m25p20", 0x2012, 64 * 1024, 4 },
	{ "m25p40", 0x2013, 64 * 1024, 8 },
	{ "m25p16", 0x2015, 64 * 1024, 32 },
	{ "m25p32", 0x2016, 64 * 1024, 64 },
	{ "m25p64", 0x2017, 64 * 1024, 128 },
	{ "m25p128", 0x2018, 256 * 1024, 64 },
	{ NULL, 0, 0, 0 }
};

/* SPI Speeds: 66 MHz / 33 MHz */
static struct flash_info flash_atmel_dataflash[] = {
	{ "AT45DB011x", 0x0c, 264, 512 },
	{ "AT45DB021x", 0x14, 264, 1025 },
	{ "AT45DB041x", 0x1c, 264, 2048 },
	{ "AT45DB081x", 0x24, 264, 4096 },
	{ "AT45DB161x", 0x2c, 528, 4096 },
	{ "AT45DB321x", 0x34, 528, 8192 },
	{ "AT45DB642x", 0x3c, 1056, 8192 },
	{ NULL, 0, 0, 0 }
};

/* SPI Speed: 50 MHz / 25 MHz or 40 MHz / 20 MHz */
static struct flash_info flash_winbond_serial_flash[] = {
	{ "W25X10", 0x3011, 16 * 256, 32 },
	{ "W25X20", 0x3012, 16 * 256, 64 },
	{ "W25X40", 0x3013, 16 * 256, 128 },
	{ "W25X80", 0x3014, 16 * 256, 256 },
	{ "W25P80", 0x2014, 256 * 256, 16 },
	{ "W25P16", 0x2015, 256 * 256, 32 },
	{ NULL, 0, 0, 0 }
};

struct flash_ops {
	uint8_t read, write, erase, status;
};

#ifdef CONFIG_SPI_FLASH_SLOW_READ
# define OP_READ 0x03
#else
# define OP_READ 0x0B
#endif
static struct flash_ops flash_st_ops = {
	.read = OP_READ,
	.write = 0x02,
	.erase = 0xD8,
	.status = 0x05,
};

static struct flash_ops flash_atmel_ops = {
	.read = OP_READ,
	.write = 0x82,
	.erase = 0x81,
	.status = 0xD7,
};

static struct flash_ops flash_winbond_ops = {
	.read = OP_READ,
	.write = 0x02,
	.erase = 0x20,
	.status = 0x05,
};

struct manufacturer_info {
	const char *name;
	uint8_t id;
	struct flash_info *flashes;
	struct flash_ops *ops;
};

static struct {
	struct manufacturer_info *manufacturer;
	struct flash_info *flash;
	struct flash_ops *ops;
	uint8_t manufacturer_id, device_id1, device_id2;
	unsigned int write_length;
	unsigned long sector_size, num_sectors;
} flash;

enum {
	JED_MANU_SPANSION = 0x01,
	JED_MANU_ST       = 0x20,
	JED_MANU_ATMEL    = 0x1F,
	JED_MANU_WINBOND  = 0xEF,
};

static struct manufacturer_info flash_manufacturers[] = {
	{
		.name = "Spansion",
		.id = JED_MANU_SPANSION,
		.flashes = flash_spansion_serial_flash,
		.ops = &flash_st_ops,
	},
	{
		.name = "ST",
		.id = JED_MANU_ST,
		.flashes = flash_st_serial_flash,
		.ops = &flash_st_ops,
	},
	{
		.name = "Atmel",
		.id = JED_MANU_ATMEL,
		.flashes = flash_atmel_dataflash,
		.ops = &flash_atmel_ops,
	},
	{
		.name = "Winbond",
		.id = JED_MANU_WINBOND,
		.flashes = flash_winbond_serial_flash,
		.ops = &flash_winbond_ops,
	},
};

#define	TIMEOUT	5000	/* timeout of 5 seconds */

/* BF54x support */
#ifndef pSPI_CTL
# define pSPI_CTL  pSPI0_CTL
# define pSPI_BAUD pSPI0_BAUD
# define pSPI_FLG  pSPI0_FLG
# define pSPI_RDBR pSPI0_RDBR
# define pSPI_STAT pSPI0_STAT
# define pSPI_TDBR pSPI0_TDBR
# define SPI0_SCK	0x0001
# define SPI0_MOSI	0x0004
# define SPI0_MISO	0x0002
# define SPI0_SEL1	0x0010
#endif

/* Default to the SPI SSEL that we boot off of:
 *	BF54x, BF537, (everything new?): SSEL1
 *	BF533, BF561: SSEL2
 */
#ifndef CONFIG_SPI_FLASH_SSEL
# if defined(__ADSPBF531__) || defined(__ADSPBF532__) || \
     defined(__ADSPBF533__) || defined(__ADSPBF561__)
#  define CONFIG_SPI_FLASH_SSEL 2
# else
#  define CONFIG_SPI_FLASH_SSEL 1
# endif
#endif
#define SSEL_MASK (1 << CONFIG_SPI_FLASH_SSEL)

static void SPI_INIT(void)
{
	/* [#3541] This delay appears to be necessary, but not sure
	 * exactly why as the history behind it is non-existant.
	 */
	udelay(CONFIG_CCLK_HZ / 25000000);

	/* enable SPI pins: SSEL, MOSI, MISO, SCK */
#ifdef __ADSPBF54x__
	*pPORTE_FER |= (SPI0_SCK | SPI0_MOSI | SPI0_MISO | SPI0_SEL1);
#elif defined(__ADSPBF534__) || defined(__ADSPBF536__) || defined(__ADSPBF537__)
	*pPORTF_FER |= (PF10 | PF11 | PF12 | PF13);
#elif defined(__ADSPBF52x__)
	bfin_write_PORTG_MUX((bfin_read_PORTG_MUX() & ~PORT_x_MUX_0_MASK) | PORT_x_MUX_0_FUNC_3);
	bfin_write_PORTG_FER(bfin_read_PORTG_FER() | PG1 | PG2 | PG3 | PG4);
#endif

	/* initate communication upon write of TDBR */
	*pSPI_CTL = (SPE|MSTR|CPHA|CPOL|0x01);
	*pSPI_BAUD = CONFIG_SPI_BAUD;
}

static void SPI_DEINIT(void)
{
	/* put SPI settings back to reset state */
	*pSPI_CTL = 0x0400;
	*pSPI_BAUD = 0;
	SSYNC();
}

static void SPI_ON(void)
{
	/* toggle SSEL to reset the device so it'll take a new command */
	*pSPI_FLG = 0xFF00 | SSEL_MASK;
	SSYNC();

	*pSPI_FLG = ((0xFF & ~SSEL_MASK) << 8) | SSEL_MASK;
	SSYNC();
}

static void SPI_OFF(void)
{
	/* put SPI settings back to reset state */
	*pSPI_FLG = 0xFF00;
	SSYNC();
}

static uint8_t spi_write_read_byte(uint8_t transmit)
{
	*pSPI_TDBR = transmit;
	SSYNC();

	while ((*pSPI_STAT & TXS))
		if (ctrlc())
			break;
	while (!(*pSPI_STAT & SPIF))
		if (ctrlc())
			break;
	while (!(*pSPI_STAT & RXS))
		if (ctrlc())
			break;

	/* Read dummy to empty the receive register */
	return *pSPI_RDBR;
}

static uint8_t read_status_register(void)
{
	uint8_t status_register;

	/* send instruction to read status register */
	SPI_ON();
	spi_write_read_byte(flash.ops->status);
	/* send dummy to receive the status register */
	status_register = spi_write_read_byte(0);
	SPI_OFF();

	return status_register;
}

static int wait_for_ready_status(void)
{
	ulong start = get_timer(0);

	while (get_timer(0) - start < TIMEOUT) {
		switch (flash.manufacturer_id) {
		case JED_MANU_SPANSION:
		case JED_MANU_ST:
		case JED_MANU_WINBOND:
			if (!(read_status_register() & 0x01))
				return 0;
			break;

		case JED_MANU_ATMEL:
			if (read_status_register() & 0x80)
				return 0;
			break;
		}

		if (ctrlc()) {
			puts("\nAbort\n");
			return -1;
		}
	}

	puts("Timeout\n");
	return -1;
}

/* Request and read the manufacturer and device id of parts which
 * are compatible with the JEDEC standard (JEP106) and use that to
 * setup other operating conditions.
 */
static int spi_detect_part(void)
{
	uint16_t dev_id;
	size_t i;

	static char called_init;
	if (called_init)
		return 0;

	SPI_ON();

	/* Send the request for the part identification */
	spi_write_read_byte(0x9F);

	/* Now read in the manufacturer id bytes */
	do {
		flash.manufacturer_id = spi_write_read_byte(0);
		if (flash.manufacturer_id == 0x7F)
			puts("Warning: unhandled manufacturer continuation byte!\n");
	} while (flash.manufacturer_id == 0x7F);

	/* Now read in the first device id byte */
	flash.device_id1 = spi_write_read_byte(0);

	/* Now read in the second device id byte */
	flash.device_id2 = spi_write_read_byte(0);

	SPI_OFF();

	dev_id = (flash.device_id1 << 8) | flash.device_id2;

	for (i = 0; i < ARRAY_SIZE(flash_manufacturers); ++i) {
		if (flash.manufacturer_id == flash_manufacturers[i].id)
			break;
	}
	if (i == ARRAY_SIZE(flash_manufacturers))
		goto unknown;

	flash.manufacturer = &flash_manufacturers[i];
	flash.ops = flash_manufacturers[i].ops;

	switch (flash.manufacturer_id) {
	case JED_MANU_SPANSION:
	case JED_MANU_ST:
	case JED_MANU_WINBOND:
		for (i = 0; flash.manufacturer->flashes[i].name; ++i) {
			if (dev_id == flash.manufacturer->flashes[i].id)
				break;
		}
		if (!flash.manufacturer->flashes[i].name)
			goto unknown;

		flash.flash = &flash.manufacturer->flashes[i];
		flash.sector_size = flash.flash->sector_size;
		flash.num_sectors = flash.flash->num_sectors;
		flash.write_length = 256;
		break;

	case JED_MANU_ATMEL: {
		uint8_t status = read_status_register();

		for (i = 0; flash.manufacturer->flashes[i].name; ++i) {
			if ((status & 0x3c) == flash.manufacturer->flashes[i].id)
				break;
		}
		if (!flash.manufacturer->flashes[i].name)
			goto unknown;

		flash.flash = &flash.manufacturer->flashes[i];
		flash.sector_size = flash.flash->sector_size;
		flash.num_sectors = flash.flash->num_sectors;

		/* see if flash is in "power of 2" mode */
		if (status & 0x1)
			flash.sector_size &= ~(1 << (ffs(flash.sector_size) - 1));

		flash.write_length = flash.sector_size;
		break;
	}
	}

	called_init = 1;
	return 0;

 unknown:
	printf("Unknown SPI device: 0x%02X 0x%02X 0x%02X\n",
		flash.manufacturer_id, flash.device_id1, flash.device_id2);
	return 1;
}

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
#if defined(CONFIG_POST) && (CONFIG_POST & CFG_POST_SPI)
	/* Our testing strategy here is pretty basic:
	 *  - fill src memory with an 8-bit pattern
	 *  - write the src memory to the SPI flash
	 *  - read the SPI flash into the dst memory
	 *  - compare src and dst memory regions
	 *  - repeat a few times
	 * The variations we test for:
	 *  - change the 8-bit pattern a bit
	 *  - change the read/write block size so we know:
	 *    - writes smaller/equal/larger than the buffer work
	 *    - writes smaller/equal/larger than the sector work
	 *  - change the SPI offsets so we know:
	 *    - writing partial sectors works
	 */
	uint8_t *mem_src, *mem_dst;
	size_t i, c, l, o;
	size_t test_count, errors;
	uint8_t pattern;

	SPI_INIT();

	if (spi_detect_part())
		goto out;
	eeprom_info();

	ulong lengths[] = {
		flash.write_length,
		flash.write_length * 2,
		flash.write_length / 2,
		flash.sector_size,
		flash.sector_size * 2,
		flash.sector_size / 2
	};
	ulong offsets[] = {
		0,
		flash.write_length,
		flash.write_length * 2,
		flash.write_length / 2,
		flash.write_length / 4,
		flash.sector_size,
		flash.sector_size * 2,
		flash.sector_size / 2,
		flash.sector_size / 4,
	};

	/* the exact addresses are arbitrary ... they just need to not overlap */
	mem_src = (void *)(0);
	mem_dst = (void *)(max(flash.write_length, flash.sector_size) * 2);

	test_count = 0;
	errors = 0;
	pattern = 0x00;

	for (i = 0; i < 16; ++i) {	/* 16 = 8 bits * 2 iterations */
		for (l = 0; l < ARRAY_SIZE(lengths); ++l) {
			for (o = 0; o < ARRAY_SIZE(offsets); ++o) {
				ulong len = lengths[l];
				ulong off = offsets[o];

				printf("Testing pattern 0x%02X of length %5lu and offset %5lu: ", pattern, len, off);

				/* setup the source memory region */
				memset(mem_src, pattern, len);

				test_count += 4;
				for (c = 0; c < 4; ++c) {	/* 4 is just a random repeat count */
					if (ctrlc()) {
						puts("\nAbort\n");
						goto out;
					}

					/* make sure background fill pattern != pattern */
					memset(mem_dst, pattern ^ 0xFF, len);

					/* write out the source memory and then read it back and compare */
					eeprom_write(0, off, mem_src, len);
					eeprom_read(0, off, mem_dst, len);

					if (memcmp(mem_src, mem_dst, len)) {
						for (c = 0; c < len; ++c)
							if (mem_src[c] != mem_dst[c])
								break;
						printf(" FAIL @ offset %u, skipping repeats ", c);
						++errors;
						break;
					}

					/* XXX: should shrink write region here to test with
					 * leading/trailing canaries so we know surrounding
					 * bytes don't get screwed.
					 */
				}
				puts("\n");
			}
		}

		/* invert the pattern every other run and shift out bits slowly */
		pattern ^= 0xFF;
		if (i % 2)
			pattern = (pattern | 0x01) << 1;
	}

	if (errors)
		printf("SPI FAIL: Out of %i tests, there were %i errors ;(\n", test_count, errors);
	else
		printf("SPI PASS: %i tests worked!\n", test_count);

 out:
	SPI_DEINIT();

#endif
}

static void transmit_address(uint32_t addr)
{
	/* Send the highest byte of the 24 bit address at first */
	spi_write_read_byte(addr >> 16);
	/* Send the middle byte of the 24 bit address  at second */
	spi_write_read_byte(addr >> 8);
	/* Send the lowest byte of the 24 bit address finally */
	spi_write_read_byte(addr);
}

/*
 * Read a value from flash for verify purpose
 * Inputs:	unsigned long ulStart - holds the SPI start address
 *			int pnData - pointer to store value read from flash
 *			long lCount - number of elements to read
 */
static int read_flash(unsigned long address, long count, uchar *buffer)
{
	size_t i;

	/* Send the read command to SPI device */
	SPI_ON();
	spi_write_read_byte(flash.ops->read);
	transmit_address(address);

#ifndef CONFIG_SPI_FLASH_SLOW_READ
	/* Send dummy byte when doing SPI fast reads */
	spi_write_read_byte(0);
#endif

	/* After the SPI device address has been placed on the MOSI pin the data can be */
	/* received on the MISO pin. */
	for (i = 1; i <= count; ++i) {
		*buffer++ = spi_write_read_byte(0);
		if (i % flash.sector_size == 0)
			puts(".");
	}

	SPI_OFF();

	return 0;
}

static int enable_writing(void)
{
	ulong start;

	if (flash.manufacturer_id == JED_MANU_ATMEL)
		return 0;

	/* A write enable instruction must previously have been executed */
	SPI_ON();
	spi_write_read_byte(0x06);
	SPI_OFF();

	/* The status register will be polled to check the write enable latch "WREN" */
	start = get_timer(0);
	while (get_timer(0) - start < TIMEOUT) {
		if (read_status_register() & 0x02)
			return 0;

		if (ctrlc()) {
			puts("\nAbort\n");
			return -1;
		}
	}

	puts("Timeout\n");
	return -1;
}

static long address_to_sector(unsigned long address)
{
	if (address > (flash.num_sectors * flash.sector_size) - 1)
		return -1;
	return address / flash.sector_size;
}

static int erase_sector(int address)
{
	/* sector gets checked in higher function, so assume it's valid
	 * here and figure out the offset of the sector in flash
	 */
	if (enable_writing())
		return -1;

	/*
	 * Send the erase block command to the flash followed by the 24 address
	 * to point to the start of a sector
	 */
	SPI_ON();
	spi_write_read_byte(flash.ops->erase);
	transmit_address(address);
	SPI_OFF();

	return wait_for_ready_status();
}

/* Write [count] bytes out of [buffer] into the given SPI [address] */
static long write_flash(unsigned long address, long count, uchar *buffer)
{
	long i, write_buffer_size;

	if (enable_writing())
		return -1;

	/* Send write command followed by the 24 bit address */
	SPI_ON();
	spi_write_read_byte(flash.ops->write);
	transmit_address(address);

	/* Shoot out a single write buffer */
	write_buffer_size = min(count, flash.write_length);
	for (i = 0; i < write_buffer_size; ++i)
		spi_write_read_byte(buffer[i]);

	SPI_OFF();

	/* Wait for the flash to do its thing */
	if (wait_for_ready_status()) {
		puts("SPI Program Time out! ");
		return -1;
	}

	return i;
}

/* Write [count] bytes out of [buffer] into the given SPI [address] */
static int write_sector(unsigned long address, long count, uchar *buffer)
{
	long write_cnt;

	while (count != 0) {
		write_cnt = write_flash(address, count, buffer);
		if (write_cnt == -1)
			return -1;

		/* Now that we've sent some bytes out to the flash, update
		 * our counters a bit
		 */
		count -= write_cnt;
		address += write_cnt;
		buffer += write_cnt;
	}

	/* return the appropriate error code */
	return 0;
}

/*
 * Function:    spi_write
 */
ssize_t spi_write(uchar *addr, int alen, uchar *buffer, int len)
{
	unsigned long offset;
	int start_sector, end_sector;
	int start_byte, end_byte;
	uchar *temp = NULL;
	int num, ret = 0;

	SPI_INIT();

	if (spi_detect_part())
		goto out;

	offset = addr[0] << 16 | addr[1] << 8 | addr[2];

	/* Get the start block number */
	start_sector = address_to_sector(offset);
	if (start_sector == -1) {
		puts("Invalid sector! ");
		goto out;
	}
	end_sector = address_to_sector(offset + len - 1);
	if (end_sector == -1) {
		puts("Invalid sector! ");
		goto out;
	}

	/* Since flashes operate in sector units but the eeprom command
	 * operates as a continuous stream of bytes, we need to emulate
	 * the eeprom behavior.  So here we read in the sector, overlay
	 * any bytes we're actually modifying, erase the sector, and
	 * then write back out the new sector.
	 */
	temp = malloc(flash.sector_size);
	if (!temp) {
		puts("Malloc for sector failed! ");
		goto out;
	}

	for (num = start_sector; num <= end_sector; num++) {
		unsigned long address = num * flash.sector_size;

		/* XXX: should add an optimization when spanning sectors:
		 * No point in reading in a sector if we're going to be
		 * clobbering the whole thing.  Need to also add a test
		 * case to make sure the optimization is correct.
		 */
		if (read_flash(address, flash.sector_size, temp)) {
			puts("Read sector failed! ");
			len = 0;
			break;
		}

		start_byte = max(address, offset);
		end_byte = address + flash.sector_size - 1;
		if (end_byte > (offset + len))
			end_byte = (offset + len - 1);

		memcpy(temp + start_byte - address,
			buffer + start_byte - offset,
			end_byte - start_byte + 1);

		if (erase_sector(address)) {
			puts("Erase sector failed! ");
			goto out;
		}

		if (write_sector(address, flash.sector_size, temp)) {
			puts("Write sector failed! ");
			goto out;
		}

		puts(".");
	}

	ret = len;

 out:
	free(temp);

	SPI_DEINIT();

	return ret;
}

/*
 * Function: spi_read
 */
ssize_t spi_read(uchar *addr, int alen, uchar *buffer, int len)
{
	unsigned long offset;

	SPI_INIT();

	if (spi_detect_part())
		len = 0;
	else {
		offset = addr[0] << 16 | addr[1] << 8 | addr[2];
		read_flash(offset, len, buffer);
	}

	SPI_DEINIT();

	return len;
}

/*
 *	Spit out some useful information about the SPI eeprom
 */
int eeprom_info(void)
{
	int ret = 0;

	SPI_INIT();

	if (spi_detect_part())
		ret = 1;
	else
		printf("SPI Device: %s 0x%02X (%s) 0x%02X 0x%02X\n"
			"Parameters: num sectors = %i, sector size = %i, write size = %i\n"
			"Flash Size: %i mbit (%i mbyte)\n"
			"Status: 0x%02X\n",
			flash.flash->name, flash.manufacturer_id, flash.manufacturer->name,
			flash.device_id1, flash.device_id2, flash.num_sectors,
			flash.sector_size, flash.write_length,
			(flash.num_sectors * flash.sector_size) >> 17,
			(flash.num_sectors * flash.sector_size) >> 20,
			read_status_register());

	SPI_DEINIT();

	return ret;
}

#endif
