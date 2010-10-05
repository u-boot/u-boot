/*
 * smsc9303.c - routines to initialize SMSC 9303 switch
 *
 * Copyright (c) 2010 BCT Electronic GmbH
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <miiphy.h>

#include <asm/blackfin.h>
#include <asm/gpio.h>

static int smc9303i_write_mii(unsigned char addr, unsigned char reg, unsigned short data)
{
	const char *devname = miiphy_get_current_dev();

	if (!devname)
	    return 0;

	if (miiphy_write(devname, addr, reg, data) != 0)
	    return 0;

	return 1;
}

static int smc9303i_write_reg(unsigned short reg, unsigned int data)
{
	const char *devname = miiphy_get_current_dev();
	unsigned char mii_addr = 0x10 | (reg >> 6);
	unsigned char mii_reg = (reg & 0x3c) >> 1;

	if (!devname)
	    return 0;

	if (miiphy_write(devname, mii_addr, mii_reg|0, data & 0xffff) != 0)
	    return 0;

	if (miiphy_write(devname, mii_addr, mii_reg|1, data >> 16) != 0)
	    return 0;

	return 1;
}

static int smc9303i_read_reg(unsigned short reg, unsigned int *data)
{
	const char *devname = miiphy_get_current_dev();
	unsigned char mii_addr = 0x10 | (reg >> 6);
	unsigned char mii_reg = (reg & 0x3c) >> 1;
	unsigned short tmp1, tmp2;

	if (!devname)
	    return 0;

	if (miiphy_read(devname, mii_addr, mii_reg|0, &tmp1) != 0)
	    return 0;

	if (miiphy_read(devname, mii_addr, mii_reg|1, &tmp2) != 0)
	    return 0;

	*data = (tmp2 << 16) | tmp1;

	return 1;
}

#if 0
static int smc9303i_read_mii(unsigned char addr, unsigned char reg, unsigned short *data)
{
	const char *devname = miiphy_get_current_dev();

	if (!devname)
	    return 0;

	if (miiphy_read(devname, addr, reg, data) != 0)
	    return 0;

	return 1;
}
#endif

typedef struct {
	unsigned short reg;
	unsigned int value;
} smsc9303i_config_entry1_t;

static const smsc9303i_config_entry1_t smsc9303i_config_table1[] =
{
	{0x1a0, 0x00000006}, /* Port 1 Manual Flow Control Register */
	{0x1a4, 0x00000006}, /* Port 2 Manual Flow Control Register */
	{0x1a8, 0x00000006}, /* Port 0 Manual Flow Control Register */
};

typedef struct
{
	unsigned char addr;
	unsigned char reg;
	unsigned short value;
} smsc9303i_config_entry2_t;

static const smsc9303i_config_entry2_t smsc9303i_config_table2[] =
{
	{0x01, 0x00, 0x0100}, /* Port0 PHY Basic Control Register */
	{0x02, 0x00, 0x1100}, /* Port1 PHY Basic Control Register */
	{0x03, 0x00, 0x1100}, /* Port2 PHY Basic Control Register */

	{0x01, 0x04, 0x0001}, /* Port0 PHY Auto-Negotiation Advertisement Register */
	{0x02, 0x04, 0x2de1}, /* Port1 PHY Auto-Negotiation Advertisement Register */
	{0x03, 0x04, 0x2de1}, /* Port2 PHY Auto-Negotiation Advertisement Register */

	{0x01, 0x11, 0x0000}, /* Port0 PHY Mode Control/Status Register */
	{0x02, 0x11, 0x0000}, /* Port1 PHY Mode Control/Status Register */
	{0x03, 0x11, 0x0000}, /* Port2 PHY Mode Control/Status Register */

	{0x01, 0x12, 0x0021}, /* Port0 PHY Special Modes Register */
	{0x02, 0x12, 0x00e2}, /* Port1 PHY Special Modes Register */
	{0x03, 0x12, 0x00e3}, /* Port2 PHY Special Modes Register */
	{0x01, 0x1b, 0x0000}, /* Port0 PHY Special Control/Status Indication Register */
	{0x02, 0x1b, 0x0000}, /* Port1 PHY Special Control/Status Indication Register */
	{0x03, 0x1b, 0x0000}, /* Port2 PHY Special Control/Status Indication Register */
	{0x01, 0x1e, 0x0000}, /* Port0 PHY Interrupt Source Flags Register */
	{0x02, 0x1e, 0x0000}, /* Port1 PHY Interrupt Source Flags Register */
	{0x03, 0x1e, 0x0000}, /* Port2 PHY Interrupt Source Flags Register */
};

int init_smsc9303i_mii(void)
{
	unsigned int data;
	unsigned int i;

	printf("       reset SMSC LAN9303i\n");

	gpio_request(GPIO_PG10, "smsc9303");
	gpio_direction_output(GPIO_PG10, 0);
	udelay(10000);
	gpio_direction_output(GPIO_PG10, 1);
	udelay(10000);

	gpio_free(GPIO_PG10);

#if defined(CONFIG_MII_INIT)
	mii_init();
#endif

	printf("       write SMSC LAN9303i configuration\n");

	if (!smc9303i_read_reg(0x50, &data))
		return 0;

	if ((data >> 16) != 0x9303)	{
		/* chip id not found */
		printf("       error identifying SMSC LAN9303i\n");
		return 0;
	}

	for (i = 0; i < ARRAY_SIZE(smsc9303i_config_table1); i++) {
		const smsc9303i_config_entry1_t *entry = &smsc9303i_config_table1[i];

		if (!smc9303i_write_reg(entry->reg, entry->value)) {
			printf("       error writing SMSC LAN9303i configuration\n");
			return 0;
		}
	}

	for (i = 0; i < ARRAY_SIZE(smsc9303i_config_table2); i++) {
		const smsc9303i_config_entry2_t *entry = &smsc9303i_config_table2[i];

		if (!smc9303i_write_mii(entry->addr, entry->reg, entry->value)) {
			printf("       error writing SMSC LAN9303i configuration\n");
			return 0;
		}
	}

	return 1;
}
