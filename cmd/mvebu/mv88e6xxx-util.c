// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 */

#include "mv88e6xxx-util.h"
/* If the switch's ADDR[4:0] strap pins are strapped to zero, it will
 * use all 32 SMI bus addresses on its SMI bus, and all switch registers
 * will be directly accessible on some {device address,register address}
 * pair.  If the ADDR[4:0] pins are not strapped to zero, the switch
 * will only respond to SMI transactions to that specific address, and
 * an indirect addressing mechanism needs to be used to access its
 * registers.
 */

static struct mv88e6xxx_dev soho_dev;
static struct mv88e6xxx_dev *soho_dev_handle;
static int REG_PORT_BASE = REG_PORT_BASE_UNDEFINED;

static int mv88e6xxx_reg_wait_ready(struct mv88e6xxx_dev *dev)
{
	int ret;
	int i;
	int loop_timeout = 50;
	unsigned short val;
	const char *name = miiphy_get_current_dev();

	if (!name)
		return -ENXIO;

	for (i = 0; i < loop_timeout; i++) {
		ret = miiphy_read(name, dev->phy_addr, SMI_CMD, &val);
		if (ret < 0)
			return ret;
		if ((val & SMI_CMD_BUSY) == 0)
			return 0;

		mdelay(10);
	}

	return -ETIMEDOUT;
}

static int mv88e6xxx_read_register(struct mv88e6xxx_dev *dev, int port, int reg)
{
	int ret;
	unsigned short val;
	const char *name = miiphy_get_current_dev();

	if (!name)
		return -ENXIO;

	if (!dev)
		return -ENODEV;

	if (dev->addr_mode == 0) {
		ret = miiphy_read(name, port, reg, &val);
		if (ret < 0)
			return ret;
		else
			return (int)val;
	}

	/* Wait for the bus to become free. */
	ret = mv88e6xxx_reg_wait_ready(dev);
	if (ret < 0)
		return ret;

	/* Transmit the read command. */
	ret = miiphy_write(name, dev->phy_addr, SMI_CMD,
			   SMI_CMD_OP_22_READ |
			   ((port & SMI_CMD_DEV_ADDR_MASK)
			   << SMI_CMD_DEV_ADDR_SIZE) |
			   (reg & SMI_CMD_REG_ADDR_MASK));
	if (ret < 0)
		return ret;

	/* Wait for the read command to complete. */
	ret = mv88e6xxx_reg_wait_ready(dev);
	if (ret < 0)
		return ret;

	/* Read the data. */
	ret = miiphy_read(name, dev->phy_addr, SMI_DATA, &val);
	if (ret < 0)
		return ret;

	return (int)val;
}

static int mv88e6xxx_reg_wait_ready_indirect(struct mv88e6xxx_dev *dev)
{
	int ret, i;
	int loop_timeout = 50;

	for (i = 0; i < loop_timeout; i++) {
		ret = mv88e6xxx_read_register(dev, REG_GLOBAL2, GLOBAL2_SMI_OP);
		if (ret < 0)
			return ret;
		if (!(ret & GLOBAL2_SMI_OP_BUSY))
			return 0;
		mdelay(10);
	}
	return -ETIMEDOUT;
}

static int mv88e6xxx_write_register(struct mv88e6xxx_dev *dev, int port,
				    int reg, unsigned short val)
{
	int ret;
	const char *name = miiphy_get_current_dev();

	if (!name)
		return -ENXIO;

	if (!dev)
		return -ENODEV;

	if (dev->addr_mode == 0) {
	/* Need wait for indriect ready before write command.*/
		ret = mv88e6xxx_reg_wait_ready_indirect(dev);
		if (ret < 0)
			return ret;
		else
			return miiphy_write(name, port, reg, val);
	}

	/* Wait for the bus to become free. */
	ret = mv88e6xxx_reg_wait_ready(dev);
	if (ret < 0)
		return ret;

	/* Transmit data to write. */
	ret = miiphy_write(name, dev->phy_addr, SMI_DATA, val);
	if (ret < 0)
		return ret;

	/* Transmit the write command. */
	ret = miiphy_write(name, dev->phy_addr, SMI_CMD,
			   SMI_CMD_OP_22_WRITE |
			   ((port & SMI_CMD_DEV_ADDR_MASK)
			   << SMI_CMD_DEV_ADDR_SIZE) |
			   (reg & SMI_CMD_REG_ADDR_MASK));
	if (ret < 0)
		return ret;

	/* Wait for the read command to complete. */
	ret = mv88e6xxx_reg_wait_ready(dev);
	if (ret < 0)
		return ret;

	return 0;
}

static int mv88e6xxx_read_indirect(struct mv88e6xxx_dev *dev, int port, int reg)
{
	int ret;

	ret = mv88e6xxx_write_register(dev,
				       REG_GLOBAL2, GLOBAL2_SMI_OP,
				       GLOBAL2_SMI_OP_22_READ |
				       ((port & SMI_CMD_DEV_ADDR_MASK)
				       << SMI_CMD_DEV_ADDR_SIZE) |
				       (reg & SMI_CMD_REG_ADDR_MASK));
	if (ret < 0)
		return ret;

	ret = mv88e6xxx_reg_wait_ready_indirect(dev);
	if (ret < 0)
		return ret;

	return mv88e6xxx_read_register(dev, REG_GLOBAL2, GLOBAL2_SMI_DATA);
}

static int mv88e6xxx_write_indirect(struct mv88e6xxx_dev *dev, int port,
				    int reg, unsigned short val)
{
	int ret;

	ret = mv88e6xxx_write_register(dev, REG_GLOBAL2, GLOBAL2_SMI_DATA, val);
	if (ret < 0)
		return ret;

	ret = mv88e6xxx_write_register(dev,
				       REG_GLOBAL2, GLOBAL2_SMI_OP,
				       GLOBAL2_SMI_OP_22_WRITE |
				       ((port & SMI_CMD_DEV_ADDR_MASK)
				       << SMI_CMD_DEV_ADDR_SIZE) |
				       (reg & SMI_CMD_REG_ADDR_MASK));

	return mv88e6xxx_reg_wait_ready_indirect(dev);
}

static int mv88e6xxx_read_phy_register(struct mv88e6xxx_dev *dev, int port,
				       int page, int reg)
{
	int ret;

	if (!dev)
		return -ENODEV;

	ret = mv88e6xxx_write_indirect(dev, port, SMI_PHY_PAGE_REG, page);
	if (ret >= 0)
		/* read if page loaded successfully  */
		ret = mv88e6xxx_read_indirect(dev, port, reg);
	/* restore page 0  */
	mv88e6xxx_write_indirect(dev, port, SMI_PHY_PAGE_REG, 0x0);

	return ret;
}

static int mv88e6xxx_write_phy_register(struct mv88e6xxx_dev *dev, int port,
					int page, int reg, unsigned short val)
{
	int ret;

	if (!dev) {
		printf("Soho dev not initialized\n");
		return -1;
	}

	ret = mv88e6xxx_write_indirect(dev, port, SMI_PHY_PAGE_REG, page);
	if (ret >= 0)
		/* write if page loaded successfully  */
		ret = mv88e6xxx_write_indirect(dev, port, reg, val);
	/* restore page 0  */
	mv88e6xxx_write_indirect(dev, port, SMI_PHY_PAGE_REG, 0x0);

	return ret;
}

static int mv88e6xxx_read_dev_register(struct mv88e6xxx_dev *dev, int port,
				       int devic, int reg)
{
	int ret;

	if (!dev)
		return -ENODEV;

	/* Write address. */
	ret = mv88e6xxx_write_register(dev, REG_GLOBAL2, GLOBAL2_SMI_DATA, reg);

	ret = mv88e6xxx_write_register(dev, REG_GLOBAL2, GLOBAL2_SMI_OP,
				       GLOBAL2_SMI_OP_45_WRITE_ADDR |
				       ((port & SMI_CMD_DEV_ADDR_MASK)
				       << SMI_CMD_DEV_ADDR_SIZE) |
				       (devic & SMI_CMD_REG_ADDR_MASK));

	/* Write read commdand.*/

	ret = mv88e6xxx_write_register(dev, REG_GLOBAL2, GLOBAL2_SMI_OP,
				       GLOBAL2_SMI_OP_45_READ_DATA |
				       ((port & SMI_CMD_DEV_ADDR_MASK)
				       << SMI_CMD_DEV_ADDR_SIZE) |
				       (devic & SMI_CMD_REG_ADDR_MASK));
	if (ret < 0)
		return ret;

	ret = mv88e6xxx_reg_wait_ready_indirect(dev);
	if (ret < 0)
		return ret;

	return mv88e6xxx_read_register(dev, REG_GLOBAL2, GLOBAL2_SMI_DATA);
}

static int mv88e6xxx_write_dev_register(struct mv88e6xxx_dev *dev, int port,
					int devic, int reg, unsigned short val)
{
	int ret;

	/* Write address. */
	ret = mv88e6xxx_write_register(dev, REG_GLOBAL2,
				       GLOBAL2_SMI_DATA, reg);
	if (ret < 0)
		return ret;

	ret = mv88e6xxx_write_register(dev, REG_GLOBAL2, GLOBAL2_SMI_OP,
				       GLOBAL2_SMI_OP_45_WRITE_ADDR |
				       ((port & SMI_CMD_DEV_ADDR_MASK)
				       << SMI_CMD_DEV_ADDR_SIZE) |
				       (devic & SMI_CMD_REG_ADDR_MASK));

	/* write data. */
	ret = mv88e6xxx_write_register(dev, REG_GLOBAL2, GLOBAL2_SMI_DATA, val);
	if (ret < 0)
		return ret;

	ret = mv88e6xxx_write_register(dev, REG_GLOBAL2, GLOBAL2_SMI_OP,
				       GLOBAL2_SMI_OP_45_WRITE_DATA |
				       ((port & SMI_CMD_DEV_ADDR_MASK)
				       << SMI_CMD_DEV_ADDR_SIZE) |
				       (devic & SMI_CMD_REG_ADDR_MASK));

	return mv88e6xxx_reg_wait_ready_indirect(dev);
}

static void mv88e6xxx_display_switch_info(struct mv88e6xxx_dev *dev)
{
	unsigned int product_num;

	if (dev->id < 0) {
		printf("No Switch Device Found\n");
		return;
	}

	product_num = ((unsigned int)dev->id) >> 4;
	if (product_num == PORT_SWITCH_ID_PROD_NUM_6390 ||
	    product_num == PORT_SWITCH_ID_PROD_NUM_6390X ||
	    product_num == PORT_SWITCH_ID_PROD_NUM_6290 ||
	    product_num == PORT_SWITCH_ID_PROD_NUM_6190 ||
	    product_num == PORT_SWITCH_ID_PROD_NUM_6193) {
		printf("Switch    : SOHO\n");
		printf("Series    : Amethyst\n");
		printf("Product # : %X\n", product_num);
		printf("Revision  : %X\n", dev->id & 0xf);
	} else if (product_num == PORT_SWITCH_ID_PROD_NUM_6141 ||
		   product_num == PORT_SWITCH_ID_PROD_NUM_6341) {
		printf("Switch    : SOHO\n");
		printf("Series    : Topaz\n");
		printf("Product # : %X\n", product_num);
		printf("Revision  : %X\n", dev->id & 0xf);
	} else {
		printf("Unknown switch with Device ID: 0x%X\n", dev->id);
	}
}

/* We expect the switch to perform auto negotiation if there is a real phy. */
static int mv88e6xxx_get_link_status(struct mv88e6xxx_dev *dev, int port)
{
int ret;

	ret = mv88e6xxx_read_register(dev, REG_PORT(port), PORT_STATUS);
	if (ret < 0)
		return ret;

	printf("Port: 0x%X, ", port);
	if (ret & PORT_STATUS_LINK) {
		printf("Link: UP, ");
	} else {
		printf("Link: Down\n");
		return 0;
	}

	if (ret & PORT_STATUS_DUPLEX)
		printf("Duplex: FULL, ");
	else
		printf("Duplex: HALF, ");

	if ((ret & PORT_STATUS_SPEED_MASK) == PORT_STATUS_SPEED_10) {
		printf("Speed: 10 Mbps\n");
	} else if ((ret & PORT_STATUS_SPEED_MASK) == PORT_STATUS_SPEED_100) {
		if (ret & PORT_STATUS_HD_FLOW)
			printf("Speed: 200 Mbps\n");
		else
			printf("Speed: 100 Mbps\n");
	} else if ((ret & PORT_STATUS_SPEED_MASK) == PORT_STATUS_SPEED_1000) {
		if (ret & PORT_STATUS_HD_FLOW)
			printf("Speed: 2500 Mbps\n");
		else
			printf("Speed: 1000 Mbps\n");
	} else if ((ret & PORT_STATUS_SPEED_MASK) ==
		 PORT_STATUS_SPEED_2500_10G) {
		if (ret & PORT_STATUS_HD_FLOW)
			printf("Speed: 5 Gbps\n");
		else
			printf("Speed: 10 Gbps\n");
	} else {
		printf("Speed: Unknown\n");
	}

	return 0;
}

static int mv88e6xxx_get_switch_id(struct mv88e6xxx_dev *dev)
{
	int id, product_num;

	/* Peridot switch port device address starts from 0
	 * Legacy switch port device address starts from 0x10
	 *
	 * In order to determine which switch is used, we need to
	 * read the ID, but inorder to read the ID, we need to know
	 * the port device address - classic chicken or the egg case.
	 *
	 * Let's read with both port device addresses, if we get 0xFFFF,
	 * the address is incorrect and we need to ready with the second
	 * address.
	 */
	id = mv88e6xxx_read_register(dev, REG_PORT_BASE_LEGACY,
				     PORT_SWITCH_ID);
	if (id == 0xFFFF)
		id = mv88e6xxx_read_register(dev, REG_PORT_BASE_PERIDOT,
					     PORT_SWITCH_ID);

	if (id < 0)
		return id;

	product_num = id >> 4;
	if (product_num == PORT_SWITCH_ID_PROD_NUM_6190 ||
	    product_num == PORT_SWITCH_ID_PROD_NUM_6193 ||
	    product_num == PORT_SWITCH_ID_PROD_NUM_6290 ||
	    product_num == PORT_SWITCH_ID_PROD_NUM_6390 ||
	    product_num == PORT_SWITCH_ID_PROD_NUM_6390X) {
		/* Peridot switch port device address starts from 0 */
		REG_PORT_BASE = REG_PORT_BASE_PERIDOT;
		return id;
	} else if (product_num == PORT_SWITCH_ID_PROD_NUM_6141 ||
		   product_num == PORT_SWITCH_ID_PROD_NUM_6341) {
		/* Legacy switch port device address starts from 0x10 */
		REG_PORT_BASE = REG_PORT_BASE_LEGACY;
		return id;
	}
	printf("Unknown product_num:0x%X\n", product_num);
	return -ENODEV;
}

static int sw_resolve_options(char *str)
{
	if (strcmp(str, "info") == 0)
		return SW_INFO;
	else if (strcmp(str, "read") == 0)
		return SW_READ;
	else if (strcmp(str, "write") == 0)
		return SW_WRITE;
	else if (strcmp(str, "phy_read") == 0)
		return SW_PHY_READ;
	else if (strcmp(str, "phy_write") == 0)
		return SW_PHY_WRITE;
	else if (strcmp(str, "dev_read") == 0)
		return SW_DEV_READ;
	else if (strcmp(str, "dev_write") == 0)
		return SW_DEV_WRITE;
	else if (strcmp(str, "link") == 0)
		return SW_LINK;
	else
		return SW_NA;
}

static int do_sw(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct mv88e6xxx_dev *dev;
	int port, reg, page, val = 0, ret = 0;

	if (argc < 3)
		return CMD_RET_USAGE;

	soho_dev.phy_addr = (int)simple_strtoul(argv[1], NULL, 16);

	if (soho_dev.phy_addr == 0)
		soho_dev.addr_mode = 0;  /* Single Addressing mode */
	else
		soho_dev.addr_mode = 1;  /* Multi Addressing mode */

	soho_dev.id = mv88e6xxx_get_switch_id(&soho_dev);

	soho_dev_handle = &soho_dev;
	dev = soho_dev_handle;

	if (soho_dev.id < 0) {
		printf("Switch Device not found\n");
		return -ENODEV;
	}

	switch (sw_resolve_options(argv[2])) {
	case SW_INFO:
		mv88e6xxx_display_switch_info(dev);
		break;
	case SW_READ:
		if (argc < 5) {
			printf("Syntax Error: switch read <port> <reg>\n");
			return 1;
		}
		port = (int)simple_strtoul(argv[3], NULL, 16);
		reg  = (int)simple_strtoul(argv[4], NULL, 16);
		ret = mv88e6xxx_read_register(dev, REG_PORT(port), reg);
		if (ret < 0) {
			printf("Failed: Read  - switch port: 0x%X, ", port);
			printf("reg: 0x%X, ret: %d\n", reg, ret);
		} else {
			printf("Read - switch port: 0x%X, ", port);
			printf("reg: 0x%X, val: 0x%X\n", reg, ret);
		}
		break;

	case SW_WRITE:
		if (argc < 6) {
			printf("Syntax Error: ");
			printf("switch write <port> <reg> <val>\n");
			return 1;
		}
		port = (int)simple_strtoul(argv[3], NULL, 16);
		reg  = (int)simple_strtoul(argv[4], NULL, 16);
		val  = (int)simple_strtoul(argv[5], NULL, 16);
		ret = mv88e6xxx_write_register(dev, REG_PORT(port), reg,
					       (unsigned short)val);
		if (ret < 0) {
			printf("Failed: Write - switch port: 0x%X, ", port);
			printf("reg: 0x%X, val: 0x%X, ret: %d\n",
			       reg, val, ret);
		} else {
			printf("Read  - switch port: 0x%X, ", port);
			printf("reg: 0x%X, val: 0x%X\n",
			       reg, val);
		}
		break;

	case SW_PHY_READ:
		if (argc < 6) {
			printf("Syntax Error: ");
			printf("switch phy_read <port> <page> <reg>\n");
			return 1;
		}
		port = (int)simple_strtoul(argv[3], NULL, 16);
		page = (int)simple_strtoul(argv[4], NULL, 16);
		reg  = (int)simple_strtoul(argv[5], NULL, 16);
		ret = mv88e6xxx_read_phy_register(dev, REG_PORT(port),
						  page, reg);
		if (ret < 0) {
			printf("Failed: Read - switch port: 0x%X, ", port);
			printf("page: 0x%X, reg: 0x%X\n, ret: %d",
			       page, reg, ret);
		} else {
			printf("Read - switch port: 0x%X, ", port);
			printf("page: 0x%X, reg: 0x%X, val: 0x%X\n",
			       page, reg, ret);
		}
		break;

	case SW_PHY_WRITE:
		if (argc < 7) {
			printf("Syntax Error: ");
			printf("switch phy_write <port> <page> <reg> <val>\n");
			return 1;
		}
		port = (int)simple_strtoul(argv[3], NULL, 16);
		page = (int)simple_strtoul(argv[4], NULL, 16);
		reg  = (int)simple_strtoul(argv[5], NULL, 16);
		val  = (int)simple_strtoul(argv[6], NULL, 16);
		ret = mv88e6xxx_write_phy_register(dev, REG_PORT(port),
						   page, reg,
						   (unsigned short)val);
		if (ret < 0) {
			printf("Failed: Write - switch port: 0x%X, ", port);
			printf("page: 0x%X, reg: 0x%X, val: 0x%X, ret: %d\n",
			       page, reg, val, ret);
		} else {
			printf("Read - switch port: 0x%X, ", port);
			printf("page: 0x%X, reg: 0x%X, val: 0x%X\n",
			       page, reg, val);
		}
		break;

	case SW_DEV_READ:
		if (argc < 6) {
			printf("Syntax Error: ");
			printf("mvswitch <mii_add> dev_read  ");
			printf("<port> <dev> <reg>\n");
			return 1;
		}
		port = (int)strtoul(argv[3], NULL, 16);
		page = (int)strtoul(argv[4], NULL, 16);
		reg  = (int)strtoul(argv[5], NULL, 16);
		ret = mv88e6xxx_read_dev_register(dev, REG_PORT(port),
						  page, reg);
		if (ret < 0) {
			printf("Failed: Read - switch port: 0x%X, ", port);
			printf("dev: 0x%X, reg: 0x%X\n, ret: %d",
			       page, reg, ret);
		} else {
			printf("Read - switch port: 0x%X, ", port);
			printf("dev: 0x%X, reg: 0x%X, val: 0x%X\n",
			       page, reg, ret);
		}
		break;

	case SW_DEV_WRITE:
		if (argc < 7) {
			printf("Syntax Error: ");
			printf("mvswitch <mii_add> dev_write ");
			printf("<port> <dev> <reg> <val>\n");
			return 1;
		}
		port = (int)strtoul(argv[3], NULL, 16);
		page = (int)strtoul(argv[4], NULL, 16);
		reg  = (int)strtoul(argv[5], NULL, 16);
		val  = (int)strtoul(argv[6], NULL, 16);
		ret = mv88e6xxx_write_dev_register(dev, REG_PORT(port),
						   page, reg,
						   (unsigned short)val);
		if (ret < 0) {
			printf("Failed: Write - switch port: 0x%X, ", port);
			printf("dev: 0x%X, reg: 0x%X, val: 0x%X, ret: %d\n",
			       page, reg, val, ret);
		} else {
			printf("Read - switch port: 0x%X, ", port);
			printf("dev: 0x%X, reg: 0x%X, val: 0x%X\n",
			       page, reg, val);
		}
		break;

	case SW_LINK:
		if (argc < 4) {
			printf("Error: Too few arguments\n");
			return 1;
		}
		port = (int)simple_strtoul(argv[3], NULL, 16);
		ret = mv88e6xxx_get_link_status(dev, port);
		break;

	case SW_NA:
		printf("\"switch %s\" - Wrong command. Try \"help switch\"\n",
		       argv[1]);

	default:
		return CMD_RET_USAGE;
	}
	return 0;
}

/***************************************************/
U_BOOT_CMD(
	mvswitch,	7,	1,	do_sw,
	"MV88e6xxx Switch Access commands",
	"mvswitch <mii_add> info  - Display switch information\n"
	"mvswitch <mii_add> read   <port> <reg>       - read switch register <reg> of a <port>\n"
	"mvswitch <mii_add> write  <port> <reg> <val> - write <val> to switch register <reg> of a <port>\n"
	"mvswitch <mii_add> phy_read  <port> <page> <reg>         - read internal switch phy register <reg> at <page> of a switch <port>\n"
	"mvswitch <mii_add> phy_write <port> <page> <reg> <val>   - write <val> to internal phy register at <page> of a <port>\n"
	"mvswitch <mii_add> dev_read  <port> <device> <reg>       - read internal switch dev register <reg> at <dev> of a switch <port>\n"
	"mvswitch <mii_add> dev_write <port> <device> <reg> <val> - write <val> to internal dev register at <dev> of a <port>\n"
	"mvswitch <mii_add> link  <port> - Display link state and speed of a <port>\n"
);
