/*
 * (C) Copyright 2007 Michal Simek
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Michal  SIMEK <monstr@monstr.eu>
 * Yasushi SHOJI <yashi@atmark-techno.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <u-boot/zlib.h>
#include <asm/byteorder.h>

#if defined(CONFIG_CMD_BOOTB)
#include <asm/icap.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

int do_bootm_linux(int flag, int argc, char * const argv[],
		   bootm_headers_t *images)
{
	/* First parameter is mapped to $r5 for kernel boot args */
	void	(*thekernel) (char *, ulong, ulong);
	char	*commandline = getenv("bootargs");
	ulong	rd_data_start, rd_data_end;

	/*
	 * allow the PREP bootm subcommand, it is required for bootm to work
	 */
	if (flag & BOOTM_STATE_OS_PREP)
		return 0;

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	int	ret;

	char	*of_flat_tree = NULL;
#if defined(CONFIG_OF_LIBFDT)
	/* did generic code already find a device tree? */
	if (images->ft_len)
		of_flat_tree = images->ft_addr;
#endif

	thekernel = (void (*)(char *, ulong, ulong))images->ep;

	/* find ramdisk */
	ret = boot_get_ramdisk(argc, argv, images, IH_ARCH_MICROBLAZE,
			&rd_data_start, &rd_data_end);
	if (ret)
		return 1;

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	if (!of_flat_tree && argc > 1)
		of_flat_tree = (char *)simple_strtoul(argv[1], NULL, 16);

	/* fixup the initrd now that we know where it should be */
	if (images->rd_start && images->rd_end && of_flat_tree)
		ret = fdt_initrd(of_flat_tree, images->rd_start,
				 images->rd_end);
		if (ret)
			return 1;

#ifdef DEBUG
	printf("## Transferring control to Linux (at address 0x%08lx) ",
	       (ulong)thekernel);
	printf("ramdisk 0x%08lx, FDT 0x%08lx...\n",
	       rd_data_start, (ulong) of_flat_tree);
#endif

#ifdef XILINX_USE_DCACHE
	flush_cache(0, XILINX_DCACHE_BYTE_SIZE);
#endif
	/*
	 * Linux Kernel Parameters (passing device tree):
	 * r5: pointer to command line
	 * r6: pointer to ramdisk
	 * r7: pointer to the fdt, followed by the board info data
	 */
	thekernel(commandline, rd_data_start, (ulong)of_flat_tree);
	/* does not return */

	return 1;
}

#if defined(CONFIG_CMD_BOOTB)
int do_bootb_kintex7(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 framebuffer[8];
	u32 bootaddress = simple_strtoul(argv[1], NULL, 16);
	u32 index = 0;
	u32 count;

	if (argc < 2)
		return -1;

	if ((bootaddress < CONFIG_SYS_FLASH_BASE) ||
	    (bootaddress > (CONFIG_SYS_FLASH_BASE + CONFIG_SYS_FLASH_SIZE)))
		return -1;

	/*
	 * Create the data to be written to the ICAP.
	 */
	framebuffer[index++] = XHI_DUMMY_PACKET;
	framebuffer[index++] = XHI_SYNC_PACKET;
	framebuffer[index++] = XHI_NOOP_PACKET;
	framebuffer[index++] = 0x30020001; /* Type 1 write to WBSTAR */
	framebuffer[index++] = bootaddress;
	framebuffer[index++] = 0x30008001; /* Type 1 Write to CMD */
	framebuffer[index++] = XHI_CMD_IPROG;
	framebuffer[index++] = XHI_NOOP_PACKET;

	/*
	 * Fill the FIFO with as many words as it will take
	 * (or as many as we have to send).
	 */
	while (index > XHwIcap_GetWrFifoVacancy(HWICAP_BASEADDR))
		;
	for (count = 0; count < index; count++)
		XHwIcap_FifoWrite(HWICAP_BASEADDR, framebuffer[count]);


	/*
	 * Start the transfer of the data from the FIFO to the ICAP device.
	 */
	XHwIcap_StartConfig(HWICAP_BASEADDR);

	while ((XHwIcap_ReadReg(HWICAP_BASEADDR, XHI_CR_OFFSET)) &
	       XHI_CR_WRITE_MASK)
		;

	while (XHwIcap_IsDeviceBusy(HWICAP_BASEADDR) != 0)
		;
	while (XHwIcap_ReadReg(HWICAP_BASEADDR, XHI_CR_OFFSET) &
	       XHI_CR_WRITE_MASK)
		;

	/* The code should never get here sice the FPGA should reset */
	return -1;
}

U_BOOT_CMD(
	bootb, 2, 1,	do_bootb_kintex7,
	"reprogram the fpga with a new image",
	"<address> - Program the FPGA with the data starting at the given address"
);

#endif
