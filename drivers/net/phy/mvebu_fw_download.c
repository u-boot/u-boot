// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <common.h>
#include <miiphy.h>
#include <asm/io.h>
#include <linux/errno.h>

#define MV_MAX_APP_SIZE			(216 * 1024UL)
#define MV_HEADER_SIZE			32

struct device_para {
	u32 dev_id;
	u32 dev_rev;
	u32 mem_size;
};

enum device_errors {
	MVEBU_FLASH_UPDATE_OK			=  0,
	/* Slave code did not start */
	MVEBU_SLAVE_CODE_DID_NOT_START		= -1,
	/* Flash verified FAILED */
	MVEBU_VERIFY_ERR			= -2,
	/* Unknown error */
	MVEBU_UNKNOWN_DOWNLOAD_TO_FLASH_FAIL	= -3,
	/* Error reading or writing MDIO register */
	MVEBU_IO_ERROR				= -4,
	/*size must be an even number of bytes*/
	MVEBU_SIZE_NOT_EVEN			= -5,
	/* Slave encountered error while erasing flash */
	MVEBU_ERR_ERASING_FLASH			= -6,
	/* unexpected value read back from download code */
	MVEBU_ERR_VALUE_READ_BACK		= -7,
	/* Did not get MVEBU_SLAVE_OK for writing the data */
	MVEBU_ERR_START_WRITE_DATA		= -8,
	/* Slave failed to get all the data correctly*/
	MVEBU_START_WRITE_DATA			= -9,
	/* Some kind of error occurred on Slave */
	MVEBU_ERR_ON_SLAVE			= -10,
	/* Checksum error */
	MVEBU_ERR_CHECKSUM			= -11,
	/* Slave didn't write enough words to flash */
	MVEBU_ERR_SLAVE_WRITE_FULL		= -12,
	/* last transfer failed */
	MVEBU_ERR_LAST_TRANSFER			= -13,
	/* wrong checksum */
	MVEBU_RAM_HW_CHECKSUM_ERR		= -14,
	/* PHY wasn't waiting in download mode */
	MVEBU_PHY_NOT_IN_DOWNLOAD_MODE		= -15,
	MVEBU_IMAGE_TOO_LARGE_TO_DOWNLOAD	= -16,
	MVEBU_ERR_GET_DEVICE			= -17,
};

#define MDIO_DEVICE_ADDRESS		1
/* Master-Slave Protocol Definitions */
#define MDIO_MAX_BUFF_SIZE		49192
#define MDIO_ACTUAL_BUFF_SIZE		49193
#define MDIO_COMMAND			49194
#define MDIO_WORDS_WRITTEN		49195
#define MDIO_LOW_ADDRESS		49196
#define MDIO_HIGH_ADDRESS		49197
#define MDIO_DATA			49198
#define MDIO_CHECKSUM			49199
#define MDIO_WORDS_RCVD			49200
#define MDIO_NUM_SECTIONS		49202 /*0xC032*/
/* Host Commands */
#define MDIO_CMD_ERASE_FLASH		0x1
#define MDIO_CMD_FILL_BUFFER		0x2
#define MDIO_CMD_WRITE_BUFFER		0x6
#define MDIO_CMD_VERIFY_FLASH		0x7
/* Slave Responses */
#define MDIO_CMD_SLV_OK			0x100
#define MDIO_CMD_SLV_ERR		0x200
#define MDIO_CMD_SLV_FLASH_BUSY		0x300
#define MDIO_CMD_SLV_VERIFY_ERR		0x400
/* 88X3240/3220 Device Number Definitions */
#define MVEBU_T_UNIT_PMA_PMD		1
#define MVEBU_T_UNIT_PCS_CU		3
#define MVEBU_H_UNIT			4
#define MVEBU_C_UNIT_GENERAL		31
/* 88X3240/3220 T Unit Registers MMD 1 */
#define MVEBU_BOOT_STATUS_REG		0xC050
/* 88X3240/3220 C Unit Registers MMD 31 */
#define MVEBU_CUNIT_MODE_CONFIG		0xF000
#define MVEBU_CUNIT_PORT_CTRL		0xF001
/* Internal PHY Registers for downloading to RAM */
/* register to set the low part of the address */
#define MVEBU_LOW_WORD_REG		0xD0F0
/* register to set the hi part of the address */
#define MVEBU_HI_WORD_REG		0xD0F1
/* register to write or read to/from ram */
#define MVEBU_RAM_DATA_REG		0xD0F2
/* register to read the checksum from */
#define MVEBU_RAM_CHECKSUM_REG		0xD0F3

static u32 mvebu_get_reg_field_from_wrod(u16 data, u8 field_off, u8 field_len)
{
	u16 mask;

	if ((field_len + field_off) >= 16)
		mask = (0 - (1 << field_off));
	else
		mask = (((1 << (field_len + field_off))) - (1 << field_off));

	return (data & mask) >> field_off;
}

static void mvebu_set_phy_reg_field(struct mii_dev *bus, u16 port, u16 dev,
				    u16 reg_addr, u8 field_off, u8 field_len,
				    u16 data)
{
	u16 reg, mask;

	reg = bus->read(bus, port, dev, reg_addr);
	/* Set register field to word */
	if ((field_len + field_off) >= 16)
		mask = (0 - (1 << field_off));
	else
		mask = (((1 << (field_len + field_off))) - (1 << field_off));
	/* Set the desired bits to 0. */
	reg &= ~mask;
	/* Set the given data into the above reset bits. */
	reg |= ((data << field_off) & mask);
	bus->write(bus, port, dev, reg_addr, reg);
}

static void mvebu_phy_reset(struct mii_dev *bus, u16 port)
{
	mvebu_set_phy_reg_field(bus, port, MVEBU_C_UNIT_GENERAL,
				MVEBU_CUNIT_PORT_CTRL, 12, 1, 1);
}

static u32 mvebu_flash_transfer(struct mii_dev *bus, u16 port, u8 data[],
				u32 max_buff_size, u32 *byte_index, int error)
{
	u16 buf_checksum, tmp_checksum, reported_checksum;
	u16 words_rcvd, words_written, reg;
	u32 stop_index;

	/* Set the flash start address*/
	bus->write(bus, port, MDIO_DEVICE_ADDRESS,
		   MDIO_LOW_ADDRESS, (u16)(*byte_index));
	bus->write(bus, port, MDIO_DEVICE_ADDRESS,
		   MDIO_HIGH_ADDRESS, (u16)((*byte_index) >> 16));
	/* Set the size of the buffer we're going to send*/
	bus->write(bus, port, MDIO_DEVICE_ADDRESS,
		   MDIO_ACTUAL_BUFF_SIZE, (u16)(max_buff_size / 2));
	/* Tell the slave we've written the start address and size */
	/* and now we're going to start writing data*/
	bus->write(bus, port, MDIO_DEVICE_ADDRESS,
		   MDIO_COMMAND, MDIO_CMD_FILL_BUFFER);
	/* Wait for MDIO_CMD_SLV_OK*/
	if (error == MVEBU_ERR_LAST_TRANSFER) {
		do {
			reg = bus->read(bus, port,
					MDIO_DEVICE_ADDRESS, MDIO_COMMAND);
		} while (reg == MDIO_CMD_FILL_BUFFER);
	} else {
		do {
			reg = bus->read(bus, port,
					MDIO_DEVICE_ADDRESS, MDIO_COMMAND);
		} while (reg == MDIO_CMD_ERASE_FLASH ||
			 reg == MDIO_CMD_SLV_FLASH_BUSY);
	}

	if (reg != MDIO_CMD_SLV_OK)
		return error;

	/* Write a buffer of data to the slave RAM*/
	stop_index = (*byte_index) + max_buff_size;
	buf_checksum = 0;
	while ((*byte_index) < stop_index) {
		u16 value;

		value = data[(*byte_index)++];
		value |= (((u16)data[(*byte_index)++]) << 8);
		buf_checksum += value;
		bus->write(bus, port, MDIO_DEVICE_ADDRESS, MDIO_DATA, value);
	}
	if (error != MVEBU_ERR_LAST_TRANSFER) {
		/* check and see if we can go on to the write*/
		tmp_checksum = bus->read(bus, port,
					 MDIO_DEVICE_ADDRESS, MDIO_CHECKSUM);
		words_rcvd = bus->read(bus, port,
				       MDIO_DEVICE_ADDRESS, MDIO_WORDS_RCVD);
		if (tmp_checksum != buf_checksum ||
		    words_rcvd != (u16)(max_buff_size / 2)) {
			debug("ERROR: phy checksum=0x%x vs buf chksum 0x%x\n",
			      tmp_checksum, buf_checksum);
			return MVEBU_START_WRITE_DATA;
		}
	}

	/* One full RAM buffer inside DSP is ready to write to flash now*/
	/* Tell the slave to write it*/
	bus->write(bus, port, MDIO_DEVICE_ADDRESS,
		   MDIO_COMMAND, MDIO_CMD_WRITE_BUFFER);
	if (error == MVEBU_ERR_LAST_TRANSFER)
		debug("Waiting for slave to programe last buffer to flash\n");
	else
		debug("Waiting for slave to finish programming flash\n");

	/* Wait for MDIO_CMD_SLV_OK */
	do {
		/*
		 * This can take several 2-3 seconds, don't poll phy too
		 * frequently since every read causes an interrupt on the phy.
		 */
		reg = bus->read(bus, port, MDIO_DEVICE_ADDRESS, MDIO_COMMAND);
		mdelay(500);
	} while (reg == MDIO_CMD_WRITE_BUFFER ||
		 reg == MDIO_CMD_SLV_FLASH_BUSY);

	if (reg != MDIO_CMD_SLV_OK) {
		debug("ERROR: slave returns error 0x%x\n", reg);

		if (reg == MDIO_CMD_SLV_VERIFY_ERR) {
			reg = bus->read(bus, port, MDIO_DEVICE_ADDRESS,
					MDIO_NUM_SECTIONS);
			debug("ERROR: Verification failed on section nr %d\n",
			      reg);
		}

		return error;
	}

	/* readback checksum of what was stored in flash */
	reported_checksum = bus->read(bus, port,
				      MDIO_DEVICE_ADDRESS, MDIO_CHECKSUM);
	if (reported_checksum != buf_checksum)
		return MVEBU_ERR_CHECKSUM;

	words_written = bus->read(bus, port,
				  MDIO_DEVICE_ADDRESS, MDIO_WORDS_WRITTEN);
	if (words_written != (max_buff_size / 2))
		return MVEBU_ERR_SLAVE_WRITE_FULL;

	return 0;
}

/*
 * This handles downloading an image pointed to by data which is size bytes
 * long to the phy's flash interfacing with the slave code as a helper program.
 * Size must be an even number (the flash can only be written to in words).
 */
static u32 mvebu_mdio_flash_download(struct mii_dev *bus, u16 port,
				     u8 data[], u32 size)
{
	u32 max_buff_size, num_trans, last_trans_size, trans_index;
	u32 byte_index = 0;
	u16 reg;

	/* size must be an even number of bytes */
	if (size % 2)
		return MVEBU_SIZE_NOT_EVEN;

	/* first erase the flash*/
	printf("Slave will now erase flash. This may take up to 30 seconds.\n");
	bus->write(bus, port, MDIO_DEVICE_ADDRESS,
		   MDIO_COMMAND, MDIO_CMD_ERASE_FLASH);
	do {
		reg = bus->read(bus, port, MDIO_DEVICE_ADDRESS, MDIO_COMMAND);
	} while (reg == MDIO_CMD_ERASE_FLASH || reg == MDIO_CMD_SLV_FLASH_BUSY);

	if (reg == MDIO_CMD_SLV_ERR)
		return MVEBU_ERR_ERASING_FLASH;

	if (reg != MDIO_CMD_SLV_OK)
		return MVEBU_ERR_VALUE_READ_BACK;
	printf("Flash program have been erased.\n");

	/* read in the maximum buffer size from the slave */
	max_buff_size = bus->read(bus, port,
				  MDIO_DEVICE_ADDRESS, MDIO_MAX_BUFF_SIZE);
	max_buff_size *= 2;
	num_trans = size / max_buff_size;
	last_trans_size = size % max_buff_size;

	debug("trans num %d, max_buff_size %d, size %d, last trans size %d\n",
	      num_trans, max_buff_size, size, last_trans_size);

	/* handle all the full transfers */
	for (trans_index = 0; trans_index < num_trans; trans_index++)
		mvebu_flash_transfer(bus, port, data, max_buff_size,
				     &byte_index, MVEBU_ERR_ON_SLAVE);

	if (last_trans_size)
		mvebu_flash_transfer(bus, port, data, last_trans_size,
				     &byte_index, MVEBU_ERR_LAST_TRANSFER);

	return 0;
}

/*
 * This function downloads code to RAM in the DSP and then starts the
 * application which was downloaded. "size" should be an even number
 * (memory can only be written word-wise)
 */
static u32 mvebu_mdio_ram_download(struct mii_dev *bus, u8 data[],
				   u32 size, u16 port, u8 ram_checksum_flag)
{
	u32 buff_count = 0;
	u16 reg, ram_checksum, checksum = 0;
	u8  low_byte, high_byte;

	/* size must be an even number of bytes */
	if (size % 2)
		return MVEBU_SIZE_NOT_EVEN;

	/* Put PHY in download mode and reset PHY */
	mvebu_set_phy_reg_field(bus, port, MVEBU_C_UNIT_GENERAL,
				0xF008, 5, 1, 1);
	mvebu_phy_reset(bus, port);

	/* Allow reset to complete */
	mdelay(2500);

	/*
	 * Make sure we can access the PHY and it's in the correct mode
	 * (waiting for download)
	 */
	reg = bus->read(bus, port,
			MVEBU_T_UNIT_PMA_PMD, MVEBU_BOOT_STATUS_REG);
	if (reg != 0x000A)
		return MVEBU_PHY_NOT_IN_DOWNLOAD_MODE;

	printf("Downloading code to PHY RAM, please wait...\n");
	/* clear the checksum */
	if (ram_checksum_flag)
		bus->read(bus, port, MVEBU_T_UNIT_PCS_CU,
			  MVEBU_RAM_CHECKSUM_REG);

	/* Set starting address in RAM to 0x00100000 */
	bus->write(bus, port, MVEBU_T_UNIT_PCS_CU, MVEBU_LOW_WORD_REG, 0);
	bus->write(bus, port, MVEBU_T_UNIT_PCS_CU, MVEBU_HI_WORD_REG, 0x0010);

	/*
	 * Copy the code to the phy's internal RAM,
	 * calculating checksum as we go.
	 */
	while (buff_count < size) {
		low_byte = data[buff_count++];
		high_byte = data[buff_count++];
		checksum += (low_byte + high_byte);
		bus->write(bus, port, MVEBU_T_UNIT_PCS_CU, MVEBU_RAM_DATA_REG,
			   (((u16)high_byte) << 8) | low_byte);
	}

	if (ram_checksum_flag) {
		ram_checksum = bus->read(bus, port, MVEBU_T_UNIT_PCS_CU,
					 MVEBU_RAM_CHECKSUM_REG);
		if (checksum != ram_checksum)
			return MVEBU_RAM_HW_CHECKSUM_ERR;
	}

	/* Now start code which was downloaded */
	mvebu_set_phy_reg_field(bus, port, MVEBU_T_UNIT_PMA_PMD,
				MVEBU_BOOT_STATUS_REG, 6, 1, 1);
	/* Give application code time to start */
	mdelay(100);

	return 0;
}

static u32 mvebu_get_device(struct mii_dev *bus, u16 port,
			    struct device_para *dev_param)
{
	u16 reg3;
	u16 model_num;
	u16 rev_num;
	u16 oui1, oui2;

	/* Check if this is a Marvell PHY */
	oui1 = bus->read(bus, port, 1, 2);
	reg3 = bus->read(bus, port, 1, 3);
	oui2 = mvebu_get_reg_field_from_wrod(reg3, 10, 6);
	model_num = mvebu_get_reg_field_from_wrod(reg3, 4, 6);
	rev_num = mvebu_get_reg_field_from_wrod(reg3, 0, 4);

	debug("%s: OUI oui1 0x%x, oui2 0x%x, model_num 0x%x, rev_num 0x%x\n",
	      __func__, oui1, oui2, model_num, rev_num);
	/* Check if the PHY OUI belongs to Marvell */
	if (!(oui1 == 0x141 && oui2 == 3) && !(oui1 == 0x2b && oui2 == 2))
		return MVEBU_ERR_GET_DEVICE;

	dev_param->dev_id = model_num;
	dev_param->dev_rev = rev_num;
	dev_param->mem_size = MV_MAX_APP_SIZE;

	printf("device id = %x, device revision = %x, memory size = %x\n",
	       dev_param->dev_id, dev_param->dev_rev, dev_param->mem_size);
	return 0;
}

u32 mvebu_update_flash_image(struct mii_dev *bus, u16 port, u8 app_data[],
			     u32 app_size, u8 salve_data[], u32 slave_size)
{
	struct device_para dev;
	int error;
	u32 data;

	/*
	 * All X32X0 have a ram checksum register, can change to 0 if wish to
	 * ignore it for some reason (not advised but might want to for debug
	 * purposes)
	 */
	u8 ram_checksum = 1;

	error = mvebu_get_device(bus, port, &dev);
	if (error < 0)
		return error;

	/* Check if the code can fit into the device's memory */
	if (app_size > dev.mem_size + MV_HEADER_SIZE)
		return MVEBU_IMAGE_TOO_LARGE_TO_DOWNLOAD;

	/* Download slave code to PHY's RAM and start it */
	error = mvebu_mdio_ram_download(bus, salve_data,
					slave_size, port, ram_checksum);
	if (error < 0)
		return error;

	/* make sure the slave code started */
	data = bus->read(bus, port, MVEBU_T_UNIT_PMA_PMD,
			 MVEBU_BOOT_STATUS_REG);
	data = mvebu_get_reg_field_from_wrod(data, 4, 1);
	if (!data)
		return MVEBU_SLAVE_CODE_DID_NOT_START;

	/* Write the image to flash */
	error = mvebu_mdio_flash_download(bus, port, app_data, app_size);
	if (error < 0)
		return error;

	/*
	 * Using slave code to verify image.
	 * This commands slave to read in entire flash image and calculate
	 * checksum and make sure checksum matches the checksum in the header.
	 * A failure means flash was corrupted.
	 * Another method would be to reset the phy (with SPI_CONFIG[1]= 0)
	 * and see that the new code starts successfully, since a bad checksum
	 * will result in the code not being started
	 */
	printf("Flash programming complete. Verifying image via slave.\n");
	bus->write(bus, port, MDIO_DEVICE_ADDRESS,
		   MDIO_COMMAND, MDIO_CMD_VERIFY_FLASH);

	do {
		data = bus->read(bus, port, MDIO_DEVICE_ADDRESS, MDIO_COMMAND);
		mdelay(100);
	} while (data == MDIO_CMD_VERIFY_FLASH ||
		 data == MDIO_CMD_SLV_FLASH_BUSY);

	switch (data) {
	case MDIO_CMD_SLV_OK:
		printf("Flash image verified. ");
		printf("Reset F_CFG1 to 0 and reboot to execute new code\n");
		return MVEBU_FLASH_UPDATE_OK;
	case MDIO_CMD_SLV_VERIFY_ERR:
		return MVEBU_VERIFY_ERR;
	default:
		return MVEBU_UNKNOWN_DOWNLOAD_TO_FLASH_FAIL;
	}

	return MVEBU_UNKNOWN_DOWNLOAD_TO_FLASH_FAIL;
}

/*
 * mvebu_phy_firmware_download: Update flash image in the PHY
 *
 * @port:	MDIO port address, 0-31
 * @app_data:	application code to be downloaded in to the flash
 * @app_size:	file size
 * @slave_data:	slave code to be downloaded in to the RAM
 * @slave_size:	file size of the slave code
 * @returns 0 on success, error code otherwise.
 */
u32 mvebu_phy_firmware_download(u16 port, u8 app_data[],
				u32 app_size, u8 salve_data[], u32 slave_size)
{
	struct mii_dev *bus;
	int error;

	printf("Ethernet transceiver PHY firmware download started:\n");
	bus = mdio_get_current_dev();
	if (!bus) {
		pr_err("failed to detect MDIO bus\n");
		return -1;
	}

	error = mvebu_update_flash_image(bus, port, app_data, app_size,
					 salve_data, slave_size);
	switch (error) {
	case MVEBU_FLASH_UPDATE_OK:
		printf("mvebu_update_flash_image succeeded\n");
		break;
	case MVEBU_ERR_GET_DEVICE:
		printf("failed to read device id\n");
		break;
	case MVEBU_IMAGE_TOO_LARGE_TO_DOWNLOAD:
		printf("image is larger than the device memory size\n");
		break;
	case MVEBU_SIZE_NOT_EVEN:
		printf("size must be an even number of bytes\n");
		break;
	case MVEBU_VERIFY_ERR:
		printf("Flash verified FAILED! ");
		printf("Flash probably corrupted. Re-try download.\n");
		break;
	case MVEBU_SLAVE_CODE_DID_NOT_START:
		printf("Slave download failed. Exiting...\n");
		break;
	case MVEBU_PHY_NOT_IN_DOWNLOAD_MODE:
		printf("Download failed, ");
		printf("PHY is not in waiting on download mode. ");
		printf("Expected 0x000A\n");
		break;
	case MVEBU_RAM_HW_CHECKSUM_ERR:
		printf("Error downloading code. ");
		printf("Got another val from the Expected RAM HW checksum\n");
		break;
	case MVEBU_ERR_VALUE_READ_BACK:
		printf("Unexpected response from PHY. Exiting...\n");
		break;
	case MVEBU_ERR_ON_SLAVE:
		printf("Unexpected error occurred on slave. Exiting...\n");
		break;
	case MVEBU_ERR_LAST_TRANSFER:
		printf("Unexpected error occurred last transfer. ");
		printf("Exiting...\n");
		break;
	case MVEBU_START_WRITE_DATA:
		printf("Slave failed to get all the data correctly\n");
		break;
	case MVEBU_ERR_SLAVE_WRITE_FULL:
		printf("Slave didn't write enough words to flash. ");
		printf("Exiting...\n");
		break;
	case MVEBU_UNKNOWN_DOWNLOAD_TO_FLASH_FAIL:
	default:
		printf("Unknown download to flash fail. Exiting...\n");
		break;
	}

	if (error < 0) {
		printf("Ethernet transceiver PHY firmware download failed.\n");
		return -1;
	}

	printf("Ethernet transceiver PHY firmware download succeeded.\n");
	/* disable download mode in the PHY */
	mvebu_set_phy_reg_field(bus, port, MVEBU_C_UNIT_GENERAL,
				0xF008, 5, 1, 0);
	mvebu_phy_reset(bus, port);

	return 0;
}
