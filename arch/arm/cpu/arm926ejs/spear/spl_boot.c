/*
 * (C) Copyright 2000-2009
 * Vipin Kumar, ST Microelectronics, vipin.kumar@st.com
 *
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <image.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include <asm/arch/spr_defs.h>
#include <linux/mtd/st_smi.h>

static const char kernel_name[] = "Linux";
static const char loader_name[] = "U-Boot";

int image_check_header(image_header_t *hdr, const char *name)
{
	if (image_check_magic(hdr) &&
	    (!strncmp(image_get_name(hdr), name, strlen(name))) &&
	    image_check_hcrc(hdr)) {
		return 1;
	}
	return 0;
}

int image_check_data(image_header_t *hdr)
{
	if (image_check_dcrc(hdr))
		return 1;

	return 0;
}

/*
 * SNOR (Serial NOR flash) related functions
 */
void snor_init(void)
{
	struct smi_regs *const smicntl =
		(struct smi_regs * const)CONFIG_SYS_SMI_BASE;

	/* Setting the fast mode values. SMI working at 166/4 = 41.5 MHz */
	writel(HOLD1 | FAST_MODE | BANK_EN | DSEL_TIME | PRESCAL4,
	       &smicntl->smi_cr1);
}

static int snor_image_load(u8 *load_addr, void (**image_p)(void),
			   const char *image_name)
{
	image_header_t *header;

	/*
	 * Since calculating the crc in the SNOR flash does not
	 * work, we copy the image to the destination address
	 * minus the header size. And point the header to this
	 * new destination. This will not work for address 0
	 * of course.
	 */
	header = (image_header_t *)load_addr;
	memcpy((ulong *)(image_get_load(header) - sizeof(image_header_t)),
	       (const ulong *)load_addr,
	       image_get_data_size(header) + sizeof(image_header_t));
	header = (image_header_t *)(image_get_load(header) -
				    sizeof(image_header_t));

	if (image_check_header(header, image_name)) {
		if (image_check_data(header)) {
			/* Jump to boot image */
			*image_p = (void *)image_get_load(header);
			return 1;
		}
	}

	return 0;
}

static void boot_image(void (*image)(void))
{
	void (*funcp)(void) __noreturn = (void *)image;

	(*funcp)();
}

/*
 * spl_boot:
 *
 * All supported booting types of all supported SoCs are listed here.
 * Generic readback APIs are provided for each supported booting type
 * eg. nand_read_skip_bad
 */
u32 spl_boot(void)
{
	void (*image)(void);

#ifdef CONFIG_SPEAR_USBTTY
	plat_late_init();
	return 1;
#endif

	/*
	 * All the supported booting devices are listed here. Each of
	 * the booting type supported by the platform would define the
	 * macro xxx_BOOT_SUPPORTED to TRUE.
	 */

	if (SNOR_BOOT_SUPPORTED && snor_boot_selected()) {
		/* SNOR-SMI initialization */
		snor_init();

		serial_puts("Booting via SNOR\n");
		/* Serial NOR booting */
		if (1 == snor_image_load((u8 *)CONFIG_SYS_UBOOT_BASE,
					    &image, loader_name)) {
			/* Platform related late initialasations */
			plat_late_init();

			/* Jump to boot image */
			serial_puts("Jumping to U-Boot\n");
			boot_image(image);
			return 1;
		}
	}

	if (NAND_BOOT_SUPPORTED && nand_boot_selected()) {
		/* NAND booting */
		/* Not ported from XLoader to SPL yet */
		return 0;
	}

	if (PNOR_BOOT_SUPPORTED && pnor_boot_selected()) {
		/* PNOR booting */
		/* Not ported from XLoader to SPL yet */
		return 0;
	}

	if (MMC_BOOT_SUPPORTED && mmc_boot_selected()) {
		/* MMC booting */
		/* Not ported from XLoader to SPL yet */
		return 0;
	}

	if (SPI_BOOT_SUPPORTED && spi_boot_selected()) {
		/* SPI booting */
		/* Not supported for any platform as of now */
		return 0;
	}

	if (I2C_BOOT_SUPPORTED && i2c_boot_selected()) {
		/* I2C booting */
		/* Not supported for any platform as of now */
		return 0;
	}

	/*
	 * All booting types without memory are listed as below
	 * Control has to be returned to BootROM in case of all
	 * the following booting scenarios
	 */

	if (USB_BOOT_SUPPORTED && usb_boot_selected()) {
		plat_late_init();
		return 1;
	}

	if (TFTP_BOOT_SUPPORTED && tftp_boot_selected()) {
		plat_late_init();
		return 1;
	}

	if (UART_BOOT_SUPPORTED && uart_boot_selected()) {
		plat_late_init();
		return 1;
	}

	/* Ideally, the control should not reach here. */
	hang();
}
