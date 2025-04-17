// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for B&R brcp150, brcp170, brcp1, brsmarc2 Board
 *
 * B&R Industrial Automation GmbH - http://www.br-automation.com
 *
 */

#include <fdtdec.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <init.h>
#include <i2c.h>
#include <dm/uclass.h>
#include <command.h>
#include <binman.h>
#include "../../common/br_resetc.h"
#include "../../common/bur_common.h"

#include <fdt_support.h>
#include <spi_flash.h>
#include <fpga.h>
#include <zynqpl.h>

#define RSTCTRL_CTRLSPEC_nPCIRST		0x1

__weak int br_board_late_init(void)
{
	return 0;
}

#if defined(CONFIG_SPL_BUILD) && CONFIG_IS_ENABLED(FPGA)
const char *fpga_paths[2] = { "/binman/blob-ext@4",
			      "/binman/blob-ext@1"};

static int start_fpga(unsigned int bank)
{
	struct spi_flash *flash_dev;
	ofnode fpga_node;
	void *buf;

	u32 flash_offset, flash_size;
	int rc;

	fpga_node = ofnode_path(fpga_paths[bank]);

	if (!ofnode_valid(fpga_node)) {
		printf("WARN:  binman node not found %s\n", fpga_paths[bank]);
		return -ENOENT;
	}

	flash_offset = ofnode_read_u32_default(fpga_node, "offset", ~0UL);
	flash_size = ofnode_read_u32_default(fpga_node, "size", ~0UL);

	if (flash_offset == ~0UL || flash_size == ~0UL) {
		printf("WARN:  invalid fpga 'offset, size' in fdt (0x%x, 0x%x)",
		       flash_offset, flash_size);
		return -EINVAL;
	}

	printf("loading bitstream from bank #%d (0x%08x / 0x%08x)\n", bank,
	       flash_offset, flash_size);

	flash_dev = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				    CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);

	if (rc)  {
		printf("WARN:  cannot probe SPI-flash for bitstream!\n");
		return -ENODEV;
	}

	buf = kmalloc(flash_size, 0);
	if (!buf) {
		spi_flash_free(flash_dev);
		return -ENOMEM;
	}

	debug("using buf @ %p, flashbase: 0x%08x, len: 0x%08x\n",
	      buf, flash_offset, flash_size);

	rc = spi_flash_read(flash_dev, flash_offset, flash_size, buf);

	spi_flash_free(flash_dev);

	if (rc) {
		printf("WARN:  cannot read bitstream from spi-flash!\n");
		kfree(buf);

		return -EIO;
	}

	rc = fpga_loadbitstream(0, buf, flash_size, BIT_FULL);
	if (rc) {
		printf("WARN:  FPGA configuration from bank #%d failed!\n", bank);
		kfree(buf);

		return -EIO;
	}

	kfree(buf);

	return 0;
}
#endif

#if defined(CONFIG_SPL_BUILD)
const char *boot_gpios[] = { "br,rs232-en",
			     "br,board-reset",
			     NULL};

/* spl stage */
int board_init(void)
{
	struct gpio_desc gpio;
	int node;
	int rc;

	/* peripheral RESET on PSOC reset-controller */
	rc = br_resetc_regset(RSTCTRL_SPECGPIO_O, RSTCTRL_CTRLSPEC_nPCIRST);
	if (rc != 0)
		printf("ERROR: cannot write to resetc (nPCIRST)!\n");

	for (int i = 0; boot_gpios[i]; i++) {
		node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, boot_gpios[i]);

		if (node < 0) {
			printf("INFO:  %s not found!\n", boot_gpios[i]);
		} else {
			rc = gpio_request_by_name_nodev(offset_to_ofnode(node), "pin",
							0, &gpio, GPIOD_IS_OUT);

			if (!rc)
				dm_gpio_set_value(&gpio, 1);
			else
				printf("ERROR: failed to setup %s!\n", boot_gpios[i]);
		}
	}

#if CONFIG_IS_ENABLED(FPGA)
	unsigned int bmode;
	unsigned int bank;

	rc = br_resetc_bmode_get(&bmode);
	if (rc) {
		printf("WARN:  can't get Boot Mode!\n");
		return -ENODEV;
	}

	/* use golden FPGA image in case of special boot flow (PME, BootAR, USB, Net ...) */
	bank = ((bmode == 0) || (bmode == 12)) ? 1 : 0;

	/* bring up FPGA */
	if (start_fpga(bank) != 0) {
		printf("WARN:  cannot start fpga from bank %d, trying bank %d!\n", bank, bank ^ 1);
		bank ^= 1;
		start_fpga(bank);
	}
#endif
	return 0;
}
#else
int board_init(void)
{
	return 0;
}

/*
 * PMIC buckboost regulator workaround:
 * The DA9062 PMIC can switch its buckboost regulator output
 * between PFM and PWM mode for eco-purpose.
 * In very rare situations this transition leads into a non-
 * functional buckboost regulator with zero output.
 * With this workaround we prevent this with turning this
 * feature off by forcing PWM-mode if auto-mode is selected.
 */
static void pmic_fixup(int addr)
{
	u8 regs[] = { 0x9E, 0x9D, 0xA0, 0x9F };
	struct udevice *i2cdev = NULL;
	unsigned int i;
	u8 val;
	int rc;

	i2c_get_chip_for_busnum(0, addr, 1, &i2cdev);
	if (!i2cdev)
		return;

	printf("PMIC:  fixup buckboost at i2c device 0x%x\n", addr);

	for (i = 0; i < sizeof(regs); i++) {
		rc = dm_i2c_read(i2cdev, regs[i], &val, 1);
		if (rc == 0 && val == 0xC0) {
			val = 0x80;
			dm_i2c_write(i2cdev, regs[i], &val, 1);
		}
	}
}

int board_late_init(void)
{
	ofnode node;
	u32 addr;

	br_resetc_bmode();
	br_board_late_init();

	node = ofnode_by_compatible(ofnode_null(), "dlg,da9062");

	if (!ofnode_valid(node))
		return 0;

	if (!ofnode_read_u32(node, "reg", &addr))
		pmic_fixup(addr);
	else
		printf("WARN:  cannot read PMIC address!");

	return 0;
}
#endif

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	zynq_ddrc_init();

	return 0;
}
