/*
 * Copyright (C) 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <errno.h>
#include <asm/io.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_private.h>
#include <fsl-mc/fsl_dpmng.h>
#include <fsl_debug_server.h>
#include <fsl-mc/fsl_dprc.h>
#include <fsl-mc/fsl_dpio.h>
#include <fsl-mc/fsl_qbman_portal.h>

#define MC_RAM_BASE_ADDR_ALIGNMENT  (512UL * 1024 * 1024)
#define MC_RAM_BASE_ADDR_ALIGNMENT_MASK	(~(MC_RAM_BASE_ADDR_ALIGNMENT - 1))
#define MC_RAM_SIZE_ALIGNMENT	    (256UL * 1024 * 1024)

#define MC_MEM_SIZE_ENV_VAR	"mcmemsize"
#define MC_BOOT_TIMEOUT_ENV_VAR	"mcboottimeout"

DECLARE_GLOBAL_DATA_PTR;
static int mc_boot_status;
struct fsl_mc_io *dflt_mc_io = NULL;
uint16_t dflt_dprc_handle = 0;
struct fsl_dpbp_obj *dflt_dpbp = NULL;
struct fsl_dpio_obj *dflt_dpio = NULL;
uint16_t dflt_dpio_handle = 0;

#ifdef DEBUG
void dump_ram_words(const char *title, void *addr)
{
	int i;
	uint32_t *words = addr;

	printf("Dumping beginning of %s (%p):\n", title, addr);
	for (i = 0; i < 16; i++)
		printf("%#x ", words[i]);

	printf("\n");
}

void dump_mc_ccsr_regs(struct mc_ccsr_registers __iomem *mc_ccsr_regs)
{
	printf("MC CCSR registers:\n"
		"reg_gcr1 %#x\n"
		"reg_gsr %#x\n"
		"reg_sicbalr %#x\n"
		"reg_sicbahr %#x\n"
		"reg_sicapr %#x\n"
		"reg_mcfbalr %#x\n"
		"reg_mcfbahr %#x\n"
		"reg_mcfapr %#x\n"
		"reg_psr %#x\n",
		mc_ccsr_regs->reg_gcr1,
		mc_ccsr_regs->reg_gsr,
		mc_ccsr_regs->reg_sicbalr,
		mc_ccsr_regs->reg_sicbahr,
		mc_ccsr_regs->reg_sicapr,
		mc_ccsr_regs->reg_mcfbalr,
		mc_ccsr_regs->reg_mcfbahr,
		mc_ccsr_regs->reg_mcfapr,
		mc_ccsr_regs->reg_psr);
}
#else

#define dump_ram_words(title, addr)
#define dump_mc_ccsr_regs(mc_ccsr_regs)

#endif /* DEBUG */

#ifndef CONFIG_SYS_LS_MC_FW_IN_DDR
/**
 * Copying MC firmware or DPL image to DDR
 */
static int mc_copy_image(const char *title,
			 u64 image_addr, u32 image_size, u64 mc_ram_addr)
{
	debug("%s copied to address %p\n", title, (void *)mc_ram_addr);
	memcpy((void *)mc_ram_addr, (void *)image_addr, image_size);
	flush_dcache_range(mc_ram_addr, mc_ram_addr + image_size);
	return 0;
}

/**
 * MC firmware FIT image parser checks if the image is in FIT
 * format, verifies integrity of the image and calculates
 * raw image address and size values.
 * Returns 0 on success and a negative errno on error.
 * task fail.
 **/
int parse_mc_firmware_fit_image(const void **raw_image_addr,
				size_t *raw_image_size)
{
	int format;
	void *fit_hdr;
	int node_offset;
	const void *data;
	size_t size;
	const char *uname = "firmware";

	/* Check if the image is in NOR flash */
#ifdef CONFIG_SYS_LS_MC_FW_IN_NOR
	fit_hdr = (void *)CONFIG_SYS_LS_MC_FW_ADDR;
#else
#error "No CONFIG_SYS_LS_MC_FW_IN_xxx defined"
#endif

	/* Check if Image is in FIT format */
	format = genimg_get_format(fit_hdr);

	if (format != IMAGE_FORMAT_FIT) {
		printf("fsl-mc: ERROR: Bad firmware image (not a FIT image)\n");
		return -EINVAL;
	}

	if (!fit_check_format(fit_hdr)) {
		printf("fsl-mc: ERROR: Bad firmware image (bad FIT header)\n");
		return -EINVAL;
	}

	node_offset = fit_image_get_node(fit_hdr, uname);

	if (node_offset < 0) {
		printf("fsl-mc: ERROR: Bad firmware image (missing subimage)\n");
		return -ENOENT;
	}

	/* Verify MC firmware image */
	if (!(fit_image_verify(fit_hdr, node_offset))) {
		printf("fsl-mc: ERROR: Bad firmware image (bad CRC)\n");
		return -EINVAL;
	}

	/* Get address and size of raw image */
	fit_image_get_data(fit_hdr, node_offset, &data, &size);

	*raw_image_addr = data;
	*raw_image_size = size;

	return 0;
}
#endif

/*
 * Calculates the values to be used to specify the address range
 * for the MC private DRAM block, in the MCFBALR/MCFBAHR registers.
 * It returns the highest 512MB-aligned address within the given
 * address range, in '*aligned_base_addr', and the number of 256 MiB
 * blocks in it, in 'num_256mb_blocks'.
 */
static int calculate_mc_private_ram_params(u64 mc_private_ram_start_addr,
					   size_t mc_ram_size,
					   u64 *aligned_base_addr,
					   u8 *num_256mb_blocks)
{
	u64 addr;
	u16 num_blocks;

	if (mc_ram_size % MC_RAM_SIZE_ALIGNMENT != 0) {
		printf("fsl-mc: ERROR: invalid MC private RAM size (%lu)\n",
		       mc_ram_size);
		return -EINVAL;
	}

	num_blocks = mc_ram_size / MC_RAM_SIZE_ALIGNMENT;
	if (num_blocks < 1 || num_blocks > 0xff) {
		printf("fsl-mc: ERROR: invalid MC private RAM size (%lu)\n",
		       mc_ram_size);
		return -EINVAL;
	}

	addr = (mc_private_ram_start_addr + mc_ram_size - 1) &
		MC_RAM_BASE_ADDR_ALIGNMENT_MASK;

	if (addr < mc_private_ram_start_addr) {
		printf("fsl-mc: ERROR: bad start address %#llx\n",
		       mc_private_ram_start_addr);
		return -EFAULT;
	}

	*aligned_base_addr = addr;
	*num_256mb_blocks = num_blocks;
	return 0;
}

static int load_mc_dpc(u64 mc_ram_addr, size_t mc_ram_size)
{
	u64 mc_dpc_offset;
#ifndef CONFIG_SYS_LS_MC_DPC_IN_DDR
	int error;
	void *dpc_fdt_hdr;
	int dpc_size;
#endif

#ifdef CONFIG_SYS_LS_MC_DRAM_DPC_OFFSET
	BUILD_BUG_ON((CONFIG_SYS_LS_MC_DRAM_DPC_OFFSET & 0x3) != 0 ||
		     CONFIG_SYS_LS_MC_DRAM_DPC_OFFSET > 0xffffffff);

	mc_dpc_offset = CONFIG_SYS_LS_MC_DRAM_DPC_OFFSET;
#else
#error "CONFIG_SYS_LS_MC_DRAM_DPC_OFFSET not defined"
#endif

	/*
	 * Load the MC DPC blob in the MC private DRAM block:
	 */
#ifdef CONFIG_SYS_LS_MC_DPC_IN_DDR
	printf("MC DPC is preloaded to %#llx\n", mc_ram_addr + mc_dpc_offset);
#else
	/*
	 * Get address and size of the DPC blob stored in flash:
	 */
#ifdef CONFIG_SYS_LS_MC_DPC_IN_NOR
	dpc_fdt_hdr = (void *)CONFIG_SYS_LS_MC_DPC_ADDR;
#else
#error "No CONFIG_SYS_LS_MC_DPC_IN_xxx defined"
#endif

	error = fdt_check_header(dpc_fdt_hdr);
	if (error != 0) {
		/*
		 * Don't return with error here, since the MC firmware can
		 * still boot without a DPC
		 */
		printf("fsl-mc: WARNING: No DPC image found\n");
		return 0;
	}

	dpc_size = fdt_totalsize(dpc_fdt_hdr);
	if (dpc_size > CONFIG_SYS_LS_MC_DPC_MAX_LENGTH) {
		printf("fsl-mc: ERROR: Bad DPC image (too large: %d)\n",
		       dpc_size);
		return -EINVAL;
	}

	mc_copy_image("MC DPC blob",
		      (u64)dpc_fdt_hdr, dpc_size, mc_ram_addr + mc_dpc_offset);
#endif /* not defined CONFIG_SYS_LS_MC_DPC_IN_DDR */

	dump_ram_words("DPC", (void *)(mc_ram_addr + mc_dpc_offset));
	return 0;
}

static int load_mc_dpl(u64 mc_ram_addr, size_t mc_ram_size)
{
	u64 mc_dpl_offset;
#ifndef CONFIG_SYS_LS_MC_DPL_IN_DDR
	int error;
	void *dpl_fdt_hdr;
	int dpl_size;
#endif

#ifdef CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET
	BUILD_BUG_ON((CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET & 0x3) != 0 ||
		     CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET > 0xffffffff);

	mc_dpl_offset = CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET;
#else
#error "CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET not defined"
#endif

	/*
	 * Load the MC DPL blob in the MC private DRAM block:
	 */
#ifdef CONFIG_SYS_LS_MC_DPL_IN_DDR
	printf("MC DPL is preloaded to %#llx\n", mc_ram_addr + mc_dpl_offset);
#else
	/*
	 * Get address and size of the DPL blob stored in flash:
	 */
#ifdef CONFIG_SYS_LS_MC_DPL_IN_NOR
	dpl_fdt_hdr = (void *)CONFIG_SYS_LS_MC_DPL_ADDR;
#else
#error "No CONFIG_SYS_LS_MC_DPL_IN_xxx defined"
#endif

	error = fdt_check_header(dpl_fdt_hdr);
	if (error != 0) {
		printf("fsl-mc: ERROR: Bad DPL image (bad header)\n");
		return error;
	}

	dpl_size = fdt_totalsize(dpl_fdt_hdr);
	if (dpl_size > CONFIG_SYS_LS_MC_DPL_MAX_LENGTH) {
		printf("fsl-mc: ERROR: Bad DPL image (too large: %d)\n",
		       dpl_size);
		return -EINVAL;
	}

	mc_copy_image("MC DPL blob",
		      (u64)dpl_fdt_hdr, dpl_size, mc_ram_addr + mc_dpl_offset);
#endif /* not defined CONFIG_SYS_LS_MC_DPL_IN_DDR */

	dump_ram_words("DPL", (void *)(mc_ram_addr + mc_dpl_offset));
	return 0;
}

/**
 * Return the MC boot timeout value in milliseconds
 */
static unsigned long get_mc_boot_timeout_ms(void)
{
	unsigned long timeout_ms = CONFIG_SYS_LS_MC_BOOT_TIMEOUT_MS;

	char *timeout_ms_env_var = getenv(MC_BOOT_TIMEOUT_ENV_VAR);

	if (timeout_ms_env_var) {
		timeout_ms = simple_strtoul(timeout_ms_env_var, NULL, 10);
		if (timeout_ms == 0) {
			printf("fsl-mc: WARNING: Invalid value for \'"
			       MC_BOOT_TIMEOUT_ENV_VAR
			       "\' environment variable: %lu\n",
			       timeout_ms);

			timeout_ms = CONFIG_SYS_LS_MC_BOOT_TIMEOUT_MS;
		}
	}

	return timeout_ms;
}

static int wait_for_mc(bool booting_mc, u32 *final_reg_gsr)
{
	u32 reg_gsr;
	u32 mc_fw_boot_status;
	unsigned long timeout_ms = get_mc_boot_timeout_ms();
	struct mc_ccsr_registers __iomem *mc_ccsr_regs = MC_CCSR_BASE_ADDR;

	dmb();
	debug("Polling mc_ccsr_regs->reg_gsr ...\n");
	assert(timeout_ms > 0);
	for (;;) {
		udelay(1000);	/* throttle polling */
		reg_gsr = in_le32(&mc_ccsr_regs->reg_gsr);
		mc_fw_boot_status = (reg_gsr & GSR_FS_MASK);
		if (mc_fw_boot_status & 0x1)
			break;

		timeout_ms--;
		if (timeout_ms == 0)
			break;
	}

	if (timeout_ms == 0) {
		if (booting_mc)
			printf("fsl-mc: timeout booting management complex firmware\n");
		else
			printf("fsl-mc: timeout deploying data path layout\n");

		/* TODO: Get an error status from an MC CCSR register */
		return -ETIMEDOUT;
	}

	if (mc_fw_boot_status != 0x1) {
		/*
		 * TODO: Identify critical errors from the GSR register's FS
		 * field and for those errors, set error to -ENODEV or other
		 * appropriate errno, so that the status property is set to
		 * failure in the fsl,dprc device tree node.
		 */
		if (booting_mc) {
			printf("fsl-mc: WARNING: Firmware booted with error (GSR: %#x)\n",
			       reg_gsr);
		} else {
			printf("fsl-mc: WARNING: Data path layout deployed with error (GSR: %#x)\n",
			       reg_gsr);
		}
	}

	*final_reg_gsr = reg_gsr;
	return 0;
}

int mc_init(void)
{
	int error = 0;
	int portal_id = 0;
	struct mc_ccsr_registers __iomem *mc_ccsr_regs = MC_CCSR_BASE_ADDR;
	u64 mc_ram_addr;
	u32 reg_gsr;
	u32 reg_mcfbalr;
#ifndef CONFIG_SYS_LS_MC_FW_IN_DDR
	const void *raw_image_addr;
	size_t raw_image_size = 0;
#endif
	struct mc_version mc_ver_info;
	u64 mc_ram_aligned_base_addr;
	u8 mc_ram_num_256mb_blocks;
	size_t mc_ram_size = mc_get_dram_block_size();

	/*
	 * The MC private DRAM block was already carved at the end of DRAM
	 * by board_init_f() using CONFIG_SYS_MEM_TOP_HIDE:
	 */
	if (gd->bd->bi_dram[1].start) {
		mc_ram_addr =
			gd->bd->bi_dram[1].start + gd->bd->bi_dram[1].size;
	} else {
		mc_ram_addr =
			gd->bd->bi_dram[0].start + gd->bd->bi_dram[0].size;
	}

#ifdef CONFIG_FSL_DEBUG_SERVER
	/*
	 * FIXME: I don't think this is right. See get_dram_size_to_hide()
	 */
		mc_ram_addr -= debug_server_get_dram_block_size();
#endif

	error = calculate_mc_private_ram_params(mc_ram_addr,
						mc_ram_size,
						&mc_ram_aligned_base_addr,
						&mc_ram_num_256mb_blocks);
	if (error != 0)
		goto out;

	/*
	 * Management Complex cores should be held at reset out of POR.
	 * U-boot should be the first software to touch MC. To be safe,
	 * we reset all cores again by setting GCR1 to 0. It doesn't do
	 * anything if they are held at reset. After we setup the firmware
	 * we kick off MC by deasserting the reset bit for core 0, and
	 * deasserting the reset bits for Command Portal Managers.
	 * The stop bits are not touched here. They are used to stop the
	 * cores when they are active. Setting stop bits doesn't stop the
	 * cores from fetching instructions when they are released from
	 * reset.
	 */
	out_le32(&mc_ccsr_regs->reg_gcr1, 0);
	dmb();

#ifdef CONFIG_SYS_LS_MC_FW_IN_DDR
	printf("MC firmware is preloaded to %#llx\n", mc_ram_addr);
#else
	error = parse_mc_firmware_fit_image(&raw_image_addr, &raw_image_size);
	if (error != 0)
		goto out;
	/*
	 * Load the MC FW at the beginning of the MC private DRAM block:
	 */
	mc_copy_image("MC Firmware",
		      (u64)raw_image_addr, raw_image_size, mc_ram_addr);
#endif
	dump_ram_words("firmware", (void *)mc_ram_addr);

	error = load_mc_dpc(mc_ram_addr, mc_ram_size);
	if (error != 0)
		goto out;

	error = load_mc_dpl(mc_ram_addr, mc_ram_size);
	if (error != 0)
		goto out;

	debug("mc_ccsr_regs %p\n", mc_ccsr_regs);
	dump_mc_ccsr_regs(mc_ccsr_regs);

	/*
	 * Tell MC what is the address range of the DRAM block assigned to it:
	 */
	reg_mcfbalr = (u32)mc_ram_aligned_base_addr |
		      (mc_ram_num_256mb_blocks - 1);
	out_le32(&mc_ccsr_regs->reg_mcfbalr, reg_mcfbalr);
	out_le32(&mc_ccsr_regs->reg_mcfbahr,
		 (u32)(mc_ram_aligned_base_addr >> 32));
	out_le32(&mc_ccsr_regs->reg_mcfapr, MCFAPR_BYPASS_ICID_MASK);

	/*
	 * Tell the MC that we want delayed DPL deployment.
	 */
	out_le32(&mc_ccsr_regs->reg_gsr, 0xDD00);

	printf("\nfsl-mc: Booting Management Complex ...\n");

	/*
	 * Deassert reset and release MC core 0 to run
	 */
	out_le32(&mc_ccsr_regs->reg_gcr1, GCR1_P1_DE_RST | GCR1_M_ALL_DE_RST);
	error = wait_for_mc(true, &reg_gsr);
	if (error != 0)
		goto out;

	/*
	 * TODO: need to obtain the portal_id for the root container from the
	 * DPL
	 */
	portal_id = 0;

	/*
	 * Initialize the global default MC portal
	 * And check that the MC firmware is responding portal commands:
	 */
	dflt_mc_io = (struct fsl_mc_io *)malloc(sizeof(struct fsl_mc_io));
	if (!dflt_mc_io) {
		printf(" No memory: malloc() failed\n");
		return -ENOMEM;
	}

	dflt_mc_io->mmio_regs = SOC_MC_PORTAL_ADDR(portal_id);
	debug("Checking access to MC portal of root DPRC container (portal_id %d, portal physical addr %p)\n",
	      portal_id, dflt_mc_io->mmio_regs);

	error = mc_get_version(dflt_mc_io, &mc_ver_info);
	if (error != 0) {
		printf("fsl-mc: ERROR: Firmware version check failed (error: %d)\n",
		       error);
		goto out;
	}

	if (MC_VER_MAJOR != mc_ver_info.major)
		printf("fsl-mc: ERROR: Firmware major version mismatch (found: %d, expected: %d)\n",
		       mc_ver_info.major, MC_VER_MAJOR);

	if (MC_VER_MINOR != mc_ver_info.minor)
		printf("fsl-mc: WARNING: Firmware minor version mismatch (found: %d, expected: %d)\n",
		       mc_ver_info.minor, MC_VER_MINOR);

	printf("fsl-mc: Management Complex booted (version: %d.%d.%d, boot status: %#x)\n",
	       mc_ver_info.major, mc_ver_info.minor, mc_ver_info.revision,
	       reg_gsr & GSR_FS_MASK);

	/*
	 * Tell the MC to deploy the DPL:
	 */
	out_le32(&mc_ccsr_regs->reg_gsr, 0x0);
	printf("\nfsl-mc: Deploying data path layout ...\n");
	error = wait_for_mc(false, &reg_gsr);
	if (error != 0)
		goto out;
out:
	if (error != 0)
		mc_boot_status = -error;
	else
		mc_boot_status = 0;

	return error;
}

int get_mc_boot_status(void)
{
	return mc_boot_status;
}

/**
 * Return the actual size of the MC private DRAM block.
 */
unsigned long mc_get_dram_block_size(void)
{
	unsigned long dram_block_size = CONFIG_SYS_LS_MC_DRAM_BLOCK_MIN_SIZE;

	char *dram_block_size_env_var = getenv(MC_MEM_SIZE_ENV_VAR);

	if (dram_block_size_env_var) {
		dram_block_size = simple_strtoul(dram_block_size_env_var, NULL,
						 10);

		if (dram_block_size < CONFIG_SYS_LS_MC_DRAM_BLOCK_MIN_SIZE) {
			printf("fsl-mc: WARNING: Invalid value for \'"
			       MC_MEM_SIZE_ENV_VAR
			       "\' environment variable: %lu\n",
			       dram_block_size);

			dram_block_size = CONFIG_SYS_LS_MC_DRAM_BLOCK_MIN_SIZE;
		}
	}

	return dram_block_size;
}

int dpio_init(struct dprc_obj_desc obj_desc)
{
	struct qbman_swp_desc p_des;
	struct dpio_attr attr;
	int err = 0;

	dflt_dpio = (struct fsl_dpio_obj *)malloc(sizeof(struct fsl_dpio_obj));
	if (!dflt_dpio) {
		printf(" No memory: malloc() failed\n");
		return -ENOMEM;
	}

	dflt_dpio->dpio_id = obj_desc.id;

	err = dpio_open(dflt_mc_io, obj_desc.id, &dflt_dpio_handle);
	if (err) {
		printf("dpio_open() failed\n");
		goto err_open;
	}

	err = dpio_get_attributes(dflt_mc_io, dflt_dpio_handle, &attr);
	if (err) {
		printf("dpio_get_attributes() failed %d\n", err);
		goto err_get_attr;
	}

	err = dpio_enable(dflt_mc_io, dflt_dpio_handle);
	if (err) {
		printf("dpio_enable() failed %d\n", err);
		goto err_get_enable;
	}
	debug("ce_paddr=0x%llx, ci_paddr=0x%llx, portalid=%d, prios=%d\n",
	      attr.qbman_portal_ce_paddr,
	      attr.qbman_portal_ci_paddr,
	      attr.qbman_portal_id,
	      attr.num_priorities);

	p_des.cena_bar = (void *)attr.qbman_portal_ce_paddr;
	p_des.cinh_bar = (void *)attr.qbman_portal_ci_paddr;

	dflt_dpio->sw_portal = qbman_swp_init(&p_des);
	if (dflt_dpio->sw_portal == NULL) {
		printf("qbman_swp_init() failed\n");
		goto err_get_swp_init;
	}
	return 0;

err_get_swp_init:
err_get_enable:
	dpio_disable(dflt_mc_io, dflt_dpio_handle);
err_get_attr:
	dpio_close(dflt_mc_io, dflt_dpio_handle);
err_open:
	free(dflt_dpio);
	return err;
}

int dpbp_init(struct dprc_obj_desc obj_desc)
{
	dflt_dpbp = (struct fsl_dpbp_obj *)malloc(sizeof(struct fsl_dpbp_obj));
	if (!dflt_dpbp) {
		printf(" No memory: malloc() failed\n");
		return -ENOMEM;
	}
	dflt_dpbp->dpbp_attr.id = obj_desc.id;

	return 0;
}

int dprc_init_container_obj(struct dprc_obj_desc obj_desc, uint16_t dprc_handle)
{
	int error = 0, state = 0;
	struct dprc_endpoint dpni_endpoint, dpmac_endpoint;
	if (!strcmp(obj_desc.type, "dpbp")) {
		if (!dflt_dpbp) {
			error = dpbp_init(obj_desc);
			if (error < 0)
				printf("dpbp_init failed\n");
		}
	} else if (!strcmp(obj_desc.type, "dpio")) {
		if (!dflt_dpio) {
			error = dpio_init(obj_desc);
			if (error < 0)
				printf("dpio_init failed\n");
		}
	} else if (!strcmp(obj_desc.type, "dpni")) {
		strcpy(dpni_endpoint.type, obj_desc.type);
		dpni_endpoint.id = obj_desc.id;
		error = dprc_get_connection(dflt_mc_io, dprc_handle,
				     &dpni_endpoint, &dpmac_endpoint, &state);
		if (!strcmp(dpmac_endpoint.type, "dpmac"))
			error = ldpaa_eth_init(obj_desc);
		if (error < 0)
			printf("ldpaa_eth_init failed\n");
	}

	return error;
}

int dprc_scan_container_obj(uint16_t dprc_handle, char *obj_type, int i)
{
	int error = 0;
	struct dprc_obj_desc obj_desc;

	memset((void *)&obj_desc, 0x00, sizeof(struct dprc_obj_desc));

	error = dprc_get_obj(dflt_mc_io, dprc_handle,
			     i, &obj_desc);
	if (error < 0) {
		printf("dprc_get_obj(i=%d) failed: %d\n",
		       i, error);
		return error;
	}

	if (!strcmp(obj_desc.type, obj_type)) {
		debug("Discovered object: type %s, id %d, req %s\n",
		      obj_desc.type, obj_desc.id, obj_type);

		error = dprc_init_container_obj(obj_desc, dprc_handle);
		if (error < 0) {
			printf("dprc_init_container_obj(i=%d) failed: %d\n",
			       i, error);
			return error;
		}
	}

	return error;
}

int fsl_mc_ldpaa_init(bd_t *bis)
{
	int i, error = 0;
	int dprc_opened = 0, container_id;
	int num_child_objects = 0;

	error = mc_init();
	if (error < 0)
		goto error;

	error = dprc_get_container_id(dflt_mc_io, &container_id);
	if (error < 0) {
		printf("dprc_get_container_id() failed: %d\n", error);
		goto error;
	}

	debug("fsl-mc: Container id=0x%x\n", container_id);

	error = dprc_open(dflt_mc_io, container_id, &dflt_dprc_handle);
	if (error < 0) {
		printf("dprc_open() failed: %d\n", error);
		goto error;
	}
	dprc_opened = true;

	error = dprc_get_obj_count(dflt_mc_io,
				   dflt_dprc_handle,
				   &num_child_objects);
	if (error < 0) {
		printf("dprc_get_obj_count() failed: %d\n", error);
		goto error;
	}
	debug("Total child in container %d = %d\n", container_id,
	      num_child_objects);

	if (num_child_objects != 0) {
		/*
		 * Discover objects currently in the DPRC container in the MC:
		 */
		for (i = 0; i < num_child_objects; i++)
			error = dprc_scan_container_obj(dflt_dprc_handle,
							"dpbp", i);

		for (i = 0; i < num_child_objects; i++)
			error = dprc_scan_container_obj(dflt_dprc_handle,
							"dpio", i);

		for (i = 0; i < num_child_objects; i++)
			error = dprc_scan_container_obj(dflt_dprc_handle,
							"dpni", i);
	}
error:
	if (dprc_opened)
		dprc_close(dflt_mc_io, dflt_dprc_handle);

	return error;
}

void fsl_mc_ldpaa_exit(bd_t *bis)
{
	int err;

	if (get_mc_boot_status() == 0) {
		err = dpio_disable(dflt_mc_io, dflt_dpio_handle);
		if (err < 0) {
			printf("dpio_disable() failed: %d\n", err);
			return;
		}
		err = dpio_reset(dflt_mc_io, dflt_dpio_handle);
		if (err < 0) {
			printf("dpio_reset() failed: %d\n", err);
			return;
		}
		err = dpio_close(dflt_mc_io, dflt_dpio_handle);
		if (err < 0) {
			printf("dpio_close() failed: %d\n", err);
			return;
		}

		free(dflt_dpio);
		free(dflt_dpbp);
	}

	if (dflt_mc_io)
		free(dflt_mc_io);
}
