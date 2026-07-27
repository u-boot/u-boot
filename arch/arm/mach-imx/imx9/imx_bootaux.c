// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <command.h>
#include <log.h>
#include <imx_sip.h>
#include <vsprintf.h>
#include <linux/arm-smccc.h>
#include <linux/errno.h>
#include <asm/mach-imx/ahab.h>
#include <asm/arch/imx-regs.h>
#include <cpu_func.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_auxiliary_core_check_up(u32 core_id)
{
	struct arm_smccc_res res;

	arm_smccc_smc(IMX_SIP_SRC, IMX_SIP_SRC_MCU_STARTED, 0, 0,
		      0, 0, 0, 0, &res);

	return res.a0;
}

int arch_auxiliary_core_down(u32 core_id)
{
	struct arm_smccc_res res;

	printf("## Stopping auxiliary core\n");

	arm_smccc_smc(IMX_SIP_SRC, IMX_SIP_SRC_MCU_STOP, 0, 0,
		      0, 0, 0, 0, &res);

	return 0;
}

int arch_auxiliary_core_up(u32 core_id, ulong addr)
{
	struct arm_smccc_res res;

	if (!addr)
		return -EINVAL;

	printf("## Starting auxiliary core addr = 0x%08lX...\n", addr);

	arm_smccc_smc(IMX_SIP_SRC, IMX_SIP_SRC_MCU_START, addr, 0,
		      0, 0, 0, 0, &res);

	return 0;
}

static inline bool check_in_ddr(ulong addr)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; ++i) {
		if (gd->dram[i].size) {
			if (addr >= gd->dram[i].start &&
			    addr < (gd->dram[i].start + gd->dram[i].size))
				return true;
		}
	}

	return false;
}

static inline bool check_in_tcm(ulong addr, bool mcore_view)
{
	if (mcore_view) {
		if ((addr >= TCML_BASE_MCORE_SEC_ADDR &&
		     addr < TCML_BASE_MCORE_SEC_ADDR + TCML_SIZE) ||
		    (addr >= TCMU_BASE_MCORE_SEC_ADDR &&
		     addr < TCMU_BASE_MCORE_SEC_ADDR + TCMU_SIZE))
			return true;

		if ((addr >= TCML_BASE_MCORE_NSEC_ADDR &&
		     addr < TCML_BASE_MCORE_NSEC_ADDR + TCML_SIZE) ||
		    (addr >= TCMU_BASE_MCORE_NSEC_ADDR &&
		     addr < TCMU_BASE_MCORE_NSEC_ADDR + TCMU_SIZE))
			return true;
	} else {
		if ((addr >= TCML_BASE_ADDR &&
		     addr < TCML_BASE_ADDR + TCML_SIZE) ||
		    (addr >= TCMU_BASE_ADDR &&
		     addr < TCMU_BASE_ADDR + TCMU_SIZE))
			return true;
	}
	return false;
}

static inline bool check_in_flexspi(ulong addr)
{
	if (addr >= FLEXSPI_AHB_ADDR && addr < FLEXSPI_AHB_ADDR + FLEXSPI_AHB_SIZE)
		return true;

	return false;
}

#if IS_ENABLED(CONFIG_AHAB_BOOT)
static int authenticate_auxcore_container(ulong addr, ulong *entry)
{
	struct container_hdr *phdr;
	int i, ret = 0;
	u16 length;
	struct boot_img_t *img;
	unsigned long s, e;

	if (addr % 4) {
		printf("Error: Image's address is not 4 byte aligned\n");
		return -EINVAL;
	}

	if (!check_in_ddr(addr) && !check_in_tcm(addr, false) && !check_in_flexspi(addr)) {
		printf("Error: Container's address is invalid\n");
		return -EINVAL;
	}

	phdr = (struct container_hdr *)addr;
	if (!valid_container_hdr(phdr)) {
		printf("Error: Wrong container header\n");
		return -EFAULT;
	}

	if (!phdr->num_images) {
		printf("Error: Wrong container, no image found\n");
		return -EFAULT;
	}

	length = phdr->length_lsb + (phdr->length_msb << 8);

	debug("container length %u\n", length);

	phdr = ahab_auth_cntr_hdr(phdr, length);
	if (!phdr) {
		ret = -EIO;
		goto exit;
	}

	/* Copy images to dest address */
	for (i = 0; i < phdr->num_images; i++) {
		img = (struct boot_img_t *)((ulong)phdr +
					    sizeof(struct container_hdr) +
					    i * sizeof(struct boot_img_t));

		/* Check Core ID of M core */
		if ((img->meta & 0xff) != 0) {
			printf("Error: Wrong Image core ID, meta = 0x%x\n", img->meta);
			ret = -EFAULT;
			break;
		}

		debug("img %d, dst 0x%x, src 0x%lx, size 0x%x\n",
		      i, (uint32_t)img->dst, img->offset + addr, img->size);

		if (check_in_flexspi(img->dst)) {
			if (img->dst != img->offset + addr) {
				printf("Error: Wrong Image[%u] load address 0x%llx\n", i, img->dst);
				ret = -EFAULT;
				break;
			}
		} else {
			if (!check_in_ddr(img->dst) && !check_in_tcm(img->dst, false)) {
				printf("Error: Invalid Image[%u] load address 0x%llx\n",
				       i, img->dst);
				ret = -EFAULT;
				break;
			}

			memcpy((void *)img->dst, (const void *)(img->offset + addr), img->size);

			s = img->dst & ~(CONFIG_SYS_CACHELINE_SIZE - 1);
			e = ALIGN(img->dst + img->size, CONFIG_SYS_CACHELINE_SIZE) - 1;

			flush_dcache_range(s, e);
		}

		ret = ahab_verify_cntr_image(img, i);
		if (ret)
			goto exit;

		/*  If the image is type of  executable, set entry */
		if (entry && (img->hab_flags & 0xf) == 0x3)
			*entry = img->entry;
	}

exit:
	debug("ahab_auth_release, 0x%x\n", ret);
	ahab_auth_release();

	return ret;
}

static int do_bootaux_cntr(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	ulong addr, entry;
	int ret, up;
	u32 core = 0;
	u32 stop = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (argc > 2)
		core = simple_strtoul(argv[2], NULL, 10);

	if (argc > 3)
		stop = simple_strtoul(argv[3], NULL, 10);

	up = arch_auxiliary_core_check_up(core);
	if (up) {
		printf("## Auxiliary core is already up\n");
		return CMD_RET_SUCCESS;
	}

	addr = simple_strtoul(argv[1], NULL, 16);

	if (!addr)
		return CMD_RET_FAILURE;

	printf("Authenticate auxcore container at 0x%lx\n", addr);

	ret = authenticate_auxcore_container(addr, &entry);
	if (ret) {
		printf("Authenticate container failed %d\n", ret);
		return CMD_RET_FAILURE;
	}

	if (!check_in_ddr(entry) && !check_in_tcm(entry, true) &&
	    !check_in_flexspi(entry)) {
		printf("Error: Image's entry 0x%lx is invalid\n", entry);
		return CMD_RET_FAILURE;
	}

	ret = arch_auxiliary_core_up(core, entry);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}
#else

/*
 * To i.MX6SX and i.MX7D, the image supported by bootaux needs
 * the reset vector at the head for the image, with SP and PC
 * as the first two words.
 *
 * Per the cortex-M reference manual, the reset vector of M4/M7 needs
 * to exist at 0x0 (TCMUL/IDTCM). The PC and SP are the first two addresses
 * of that vector.  So to boot M4/M7, the A core must build the M4/M7's reset
 * vector with getting the PC and SP from image and filling them to
 * TCMUL/IDTCM. When M4/M7 is kicked, it will load the PC and SP by itself.
 * The TCMUL/IDTCM is mapped to (MCU_BOOTROM_BASE_ADDR) at A core side for
 * accessing the M4/M7 TCMUL/IDTCM.
 */
static int do_bootaux(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	ulong addr;
	int ret, up;
	u32 core = 0;
	u32 stop = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (argc > 2)
		core = simple_strtoul(argv[2], NULL, 10);

	if (argc > 3)
		stop = simple_strtoul(argv[3], NULL, 10);

	up = arch_auxiliary_core_check_up(core);
	if (up) {
		printf("## Auxiliary core is already up\n");
		return CMD_RET_SUCCESS;
	}

	addr = simple_strtoul(argv[1], NULL, 16);

	if (!addr)
		return CMD_RET_FAILURE;

	if (!check_in_ddr(addr) && !check_in_tcm(addr, true) && !check_in_flexspi(addr)) {
		printf("Error: Image's address 0x%lx is invalid\n", addr);
		printf("     Address should be memory from M core view,\n"
			   "     For example: 0x1ffe0000 for TCML in secure\n");
		return CMD_RET_FAILURE;
	}

	ret = arch_auxiliary_core_up(core, addr);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}
#endif

static int do_stopaux(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	int ret, up;

	up = arch_auxiliary_core_check_up(0);
	if (!up) {
		printf("## Auxiliary core is already down\n");
		return CMD_RET_SUCCESS;
	}

	ret = arch_auxiliary_core_down(0);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	stopaux, CONFIG_SYS_MAXARGS, 1,	do_stopaux,
	"Stop auxiliary core",
	"<address> [<core>]\n"
	"   - start auxiliary core [<core>] (default 0),\n"
	"     at address <address>\n"
);

#if IS_ENABLED(CONFIG_AHAB_BOOT)
U_BOOT_CMD(
	bootaux_cntr, CONFIG_SYS_MAXARGS, 1,	do_bootaux_cntr,
	"Start auxiliary core",
	"<container_address> [<core>]\n"
	"   - start auxiliary core [<core>] (default 0),\n"
	"     with signed container image at address <address> in A core view\n"
);
#else
U_BOOT_CMD(
	bootaux, CONFIG_SYS_MAXARGS, 1,	do_bootaux,
	"Start auxiliary core",
	"<address> [<core>]\n"
	"   - start auxiliary core [<core>] (default 0),\n"
	"     at address <address> of auxiliary core view\n"
);
#endif
