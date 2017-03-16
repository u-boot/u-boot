/*
 * Copyright (C) 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <errno.h>
#include <linux/bug.h>
#include <asm/io.h>
#include <libfdt.h>
#include <net.h>
#include <fdt_support.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_private.h>
#include <fsl-mc/fsl_dpmng.h>
#include <fsl-mc/fsl_dprc.h>
#include <fsl-mc/fsl_dpio.h>
#include <fsl-mc/fsl_dpni.h>
#include <fsl-mc/fsl_qbman_portal.h>
#include <fsl-mc/ldpaa_wriop.h>

#define MC_RAM_BASE_ADDR_ALIGNMENT  (512UL * 1024 * 1024)
#define MC_RAM_BASE_ADDR_ALIGNMENT_MASK	(~(MC_RAM_BASE_ADDR_ALIGNMENT - 1))
#define MC_RAM_SIZE_ALIGNMENT	    (256UL * 1024 * 1024)

#define MC_MEM_SIZE_ENV_VAR	"mcmemsize"
#define MC_BOOT_TIMEOUT_ENV_VAR	"mcboottimeout"

DECLARE_GLOBAL_DATA_PTR;
static int mc_boot_status = -1;
static int mc_dpl_applied = -1;
#ifdef CONFIG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET
static int mc_aiop_applied = -1;
#endif
struct fsl_mc_io *root_mc_io = NULL;
struct fsl_mc_io *dflt_mc_io = NULL; /* child container */
uint16_t root_dprc_handle = 0;
uint16_t dflt_dprc_handle = 0;
int child_dprc_id;
struct fsl_dpbp_obj *dflt_dpbp = NULL;
struct fsl_dpio_obj *dflt_dpio = NULL;
struct fsl_dpni_obj *dflt_dpni = NULL;
static u64 mc_lazy_dpl_addr;

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
int parse_mc_firmware_fit_image(u64 mc_fw_addr,
				const void **raw_image_addr,
				size_t *raw_image_size)
{
	int format;
	void *fit_hdr;
	int node_offset;
	const void *data;
	size_t size;
	const char *uname = "firmware";

	fit_hdr = (void *)mc_fw_addr;

	/* Check if Image is in FIT format */
	format = genimg_get_format(fit_hdr);

	if (format != IMAGE_FORMAT_FIT) {
		printf("fsl-mc: ERR: Bad firmware image (not a FIT image)\n");
		return -EINVAL;
	}

	if (!fit_check_format(fit_hdr)) {
		printf("fsl-mc: ERR: Bad firmware image (bad FIT header)\n");
		return -EINVAL;
	}

	node_offset = fit_image_get_node(fit_hdr, uname);

	if (node_offset < 0) {
		printf("fsl-mc: ERR: Bad firmware image (missing subimage)\n");
		return -ENOENT;
	}

	/* Verify MC firmware image */
	if (!(fit_image_verify(fit_hdr, node_offset))) {
		printf("fsl-mc: ERR: Bad firmware image (bad CRC)\n");
		return -EINVAL;
	}

	/* Get address and size of raw image */
	fit_image_get_data(fit_hdr, node_offset, &data, &size);

	*raw_image_addr = data;
	*raw_image_size = size;

	return 0;
}
#endif

static int mc_fixup_dpc_mac_addr(void *blob, int noff, int dpmac_id,
		struct eth_device *eth_dev)
{
	int nodeoffset, err = 0;
	char mac_name[10];
	const char link_type_mode[] = "FIXED_LINK";
	unsigned char env_enetaddr[6];

	sprintf(mac_name, "mac@%d", dpmac_id);

	/* node not found - create it */
	nodeoffset = fdt_subnode_offset(blob, noff, (const char *) mac_name);
	if (nodeoffset < 0) {
		err = fdt_increase_size(blob, 200);
		if (err) {
			printf("fdt_increase_size: err=%s\n",
				fdt_strerror(err));
			return err;
		}

		nodeoffset = fdt_add_subnode(blob, noff, mac_name);

		/* add default property of fixed link */
		err = fdt_appendprop_string(blob, nodeoffset,
					    "link_type", link_type_mode);
		if (err) {
			printf("fdt_appendprop_string: err=%s\n",
				fdt_strerror(err));
			return err;
		}
	}

	/* port_mac_address property present in DPC */
	if (fdt_get_property(blob, nodeoffset, "port_mac_address", NULL)) {
		/* MAC addr randomly assigned - leave the one in DPC */
		eth_getenv_enetaddr_by_index("eth", eth_dev->index,
						env_enetaddr);
		if (is_zero_ethaddr(env_enetaddr))
			return err;

		/* replace DPC MAC address with u-boot env one */
		err = fdt_setprop(blob, nodeoffset, "port_mac_address",
				  eth_dev->enetaddr, 6);
		if (err) {
			printf("fdt_setprop mac: err=%s\n", fdt_strerror(err));
			return err;
		}

		return 0;
	}

	/* append port_mac_address property to mac node in DPC */
	err = fdt_increase_size(blob, 80);
	if (err) {
		printf("fdt_increase_size: err=%s\n", fdt_strerror(err));
		return err;
	}

	err = fdt_appendprop(blob, nodeoffset,
			     "port_mac_address", eth_dev->enetaddr, 6);
	if (err) {
		printf("fdt_appendprop: err=%s\n", fdt_strerror(err));
		return err;
	}

	return err;
}

static int mc_fixup_dpc(u64 dpc_addr)
{
	void *blob = (void *)dpc_addr;
	int nodeoffset, err = 0;
	char ethname[10];
	struct eth_device *eth_dev;
	int i;

	/* delete any existing ICID pools */
	nodeoffset = fdt_path_offset(blob, "/resources/icid_pools");
	if (fdt_del_node(blob, nodeoffset) < 0)
		printf("\nfsl-mc: WARNING: could not delete ICID pool\n");

	/* add a new pool */
	nodeoffset = fdt_path_offset(blob, "/resources");
	if (nodeoffset < 0) {
		printf("\nfsl-mc: ERROR: DPC is missing /resources\n");
		return -EINVAL;
	}
	nodeoffset = fdt_add_subnode(blob, nodeoffset, "icid_pools");
	nodeoffset = fdt_add_subnode(blob, nodeoffset, "icid_pool@0");
	do_fixup_by_path_u32(blob, "/resources/icid_pools/icid_pool@0",
			     "base_icid", FSL_DPAA2_STREAM_ID_START, 1);
	do_fixup_by_path_u32(blob, "/resources/icid_pools/icid_pool@0",
			     "num",
			     FSL_DPAA2_STREAM_ID_END -
			     FSL_DPAA2_STREAM_ID_START + 1, 1);

	/* fixup MAC addresses for dpmac ports */
	nodeoffset = fdt_path_offset(blob, "/board_info/ports");
	if (nodeoffset < 0)
		goto out;

	for (i = WRIOP1_DPMAC1; i < NUM_WRIOP_PORTS; i++) {
		/* port not enabled */
		if ((wriop_is_enabled_dpmac(i) != 1) ||
		    (wriop_get_phy_address(i) == -1))
			continue;

		sprintf(ethname, "DPMAC%d@%s", i,
			phy_interface_strings[wriop_get_enet_if(i)]);

		eth_dev = eth_get_dev_by_name(ethname);
		if (eth_dev == NULL)
			continue;

		err = mc_fixup_dpc_mac_addr(blob, nodeoffset, i, eth_dev);
		if (err) {
			printf("mc_fixup_dpc_mac_addr failed: err=%s\n",
			fdt_strerror(err));
			goto out;
		}
	}

out:
	flush_dcache_range(dpc_addr, dpc_addr + fdt_totalsize(blob));

	return err;
}

static int load_mc_dpc(u64 mc_ram_addr, size_t mc_ram_size, u64 mc_dpc_addr)
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
	dpc_fdt_hdr = (void *)mc_dpc_addr;

	error = fdt_check_header(dpc_fdt_hdr);
	if (error != 0) {
		/*
		 * Don't return with error here, since the MC firmware can
		 * still boot without a DPC
		 */
		printf("\nfsl-mc: WARNING: No DPC image found");
		return 0;
	}

	dpc_size = fdt_totalsize(dpc_fdt_hdr);
	if (dpc_size > CONFIG_SYS_LS_MC_DPC_MAX_LENGTH) {
		printf("\nfsl-mc: ERROR: Bad DPC image (too large: %d)\n",
		       dpc_size);
		return -EINVAL;
	}

	mc_copy_image("MC DPC blob",
		      (u64)dpc_fdt_hdr, dpc_size, mc_ram_addr + mc_dpc_offset);
#endif /* not defined CONFIG_SYS_LS_MC_DPC_IN_DDR */

	if (mc_fixup_dpc(mc_ram_addr + mc_dpc_offset))
		return -EINVAL;

	dump_ram_words("DPC", (void *)(mc_ram_addr + mc_dpc_offset));
	return 0;
}

static int load_mc_dpl(u64 mc_ram_addr, size_t mc_ram_size, u64 mc_dpl_addr)
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
	dpl_fdt_hdr = (void *)mc_dpl_addr;

	error = fdt_check_header(dpl_fdt_hdr);
	if (error != 0) {
		printf("\nfsl-mc: ERROR: Bad DPL image (bad header)\n");
		return error;
	}

	dpl_size = fdt_totalsize(dpl_fdt_hdr);
	if (dpl_size > CONFIG_SYS_LS_MC_DPL_MAX_LENGTH) {
		printf("\nfsl-mc: ERROR: Bad DPL image (too large: %d)\n",
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

#ifdef CONFIG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET

__weak bool soc_has_aiop(void)
{
	return false;
}

static int load_mc_aiop_img(u64 aiop_fw_addr)
{
	u64 mc_ram_addr = mc_get_dram_addr();
#ifndef CONFIG_SYS_LS_MC_DPC_IN_DDR
	void *aiop_img;
#endif

	/* Check if AIOP is available */
	if (!soc_has_aiop())
		return -ENODEV;
	/*
	 * Load the MC AIOP image in the MC private DRAM block:
	 */

#ifdef CONFIG_SYS_LS_MC_DPC_IN_DDR
	printf("MC AIOP is preloaded to %#llx\n", mc_ram_addr +
	       CONFIG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET);
#else
	aiop_img = (void *)aiop_fw_addr;
	mc_copy_image("MC AIOP image",
		      (u64)aiop_img, CONFIG_SYS_LS_MC_AIOP_IMG_MAX_LENGTH,
		      mc_ram_addr + CONFIG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET);
#endif
	mc_aiop_applied = 0;

	return 0;
}
#endif

static int wait_for_mc(bool booting_mc, u32 *final_reg_gsr)
{
	u32 reg_gsr;
	u32 mc_fw_boot_status;
	unsigned long timeout_ms = get_mc_boot_timeout_ms();
	struct mc_ccsr_registers __iomem *mc_ccsr_regs = MC_CCSR_BASE_ADDR;

	dmb();
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
		printf("ERROR: timeout\n");

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
		printf("WARNING: Firmware returned an error (GSR: %#x)\n",
		       reg_gsr);
	} else {
		printf("SUCCESS\n");
	}


	*final_reg_gsr = reg_gsr;
	return 0;
}

int mc_init(u64 mc_fw_addr, u64 mc_dpc_addr)
{
	int error = 0;
	int portal_id = 0;
	struct mc_ccsr_registers __iomem *mc_ccsr_regs = MC_CCSR_BASE_ADDR;
	u64 mc_ram_addr = mc_get_dram_addr();
	u32 reg_gsr;
	u32 reg_mcfbalr;
#ifndef CONFIG_SYS_LS_MC_FW_IN_DDR
	const void *raw_image_addr;
	size_t raw_image_size = 0;
#endif
	struct mc_version mc_ver_info;
	u8 mc_ram_num_256mb_blocks;
	size_t mc_ram_size = mc_get_dram_block_size();

	mc_ram_num_256mb_blocks = mc_ram_size / MC_RAM_SIZE_ALIGNMENT;
	if (mc_ram_num_256mb_blocks < 1 || mc_ram_num_256mb_blocks > 0xff) {
		error = -EINVAL;
		printf("fsl-mc: ERROR: invalid MC private RAM size (%lu)\n",
		       mc_ram_size);
		goto out;
	}

	/*
	 * Management Complex cores should be held at reset out of POR.
	 * U-Boot should be the first software to touch MC. To be safe,
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
	error = parse_mc_firmware_fit_image(mc_fw_addr, &raw_image_addr,
					    &raw_image_size);
	if (error != 0)
		goto out;
	/*
	 * Load the MC FW at the beginning of the MC private DRAM block:
	 */
	mc_copy_image("MC Firmware",
		      (u64)raw_image_addr, raw_image_size, mc_ram_addr);
#endif
	dump_ram_words("firmware", (void *)mc_ram_addr);

	error = load_mc_dpc(mc_ram_addr, mc_ram_size, mc_dpc_addr);
	if (error != 0)
		goto out;

	debug("mc_ccsr_regs %p\n", mc_ccsr_regs);
	dump_mc_ccsr_regs(mc_ccsr_regs);

	/*
	 * Tell MC what is the address range of the DRAM block assigned to it:
	 */
	reg_mcfbalr = (u32)mc_ram_addr |
		      (mc_ram_num_256mb_blocks - 1);
	out_le32(&mc_ccsr_regs->reg_mcfbalr, reg_mcfbalr);
	out_le32(&mc_ccsr_regs->reg_mcfbahr,
		 (u32)(mc_ram_addr >> 32));
	out_le32(&mc_ccsr_regs->reg_mcfapr, FSL_BYPASS_AMQ);

	/*
	 * Tell the MC that we want delayed DPL deployment.
	 */
	out_le32(&mc_ccsr_regs->reg_gsr, 0xDD00);

	printf("\nfsl-mc: Booting Management Complex ... ");

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
	root_mc_io = (struct fsl_mc_io *)malloc(sizeof(struct fsl_mc_io));
	if (!root_mc_io) {
		printf(" No memory: malloc() failed\n");
		return -ENOMEM;
	}

	root_mc_io->mmio_regs = SOC_MC_PORTAL_ADDR(portal_id);
	debug("Checking access to MC portal of root DPRC container (portal_id %d, portal physical addr %p)\n",
	      portal_id, root_mc_io->mmio_regs);

	error = mc_get_version(root_mc_io, MC_CMD_NO_FLAGS, &mc_ver_info);
	if (error != 0) {
		printf("fsl-mc: ERROR: Firmware version check failed (error: %d)\n",
		       error);
		goto out;
	}

	printf("fsl-mc: Management Complex booted (version: %d.%d.%d, boot status: %#x)\n",
	       mc_ver_info.major, mc_ver_info.minor, mc_ver_info.revision,
	       reg_gsr & GSR_FS_MASK);

out:
	if (error != 0)
		mc_boot_status = error;
	else
		mc_boot_status = 0;

	return error;
}

int mc_apply_dpl(u64 mc_dpl_addr)
{
	struct mc_ccsr_registers __iomem *mc_ccsr_regs = MC_CCSR_BASE_ADDR;
	int error = 0;
	u32 reg_gsr;
	u64 mc_ram_addr = mc_get_dram_addr();
	size_t mc_ram_size = mc_get_dram_block_size();

	if (!mc_dpl_addr)
		return -1;

	error = load_mc_dpl(mc_ram_addr, mc_ram_size, mc_dpl_addr);
	if (error != 0)
		return error;

	/*
	 * Tell the MC to deploy the DPL:
	 */
	out_le32(&mc_ccsr_regs->reg_gsr, 0x0);
	printf("fsl-mc: Deploying data path layout ... ");
	error = wait_for_mc(false, &reg_gsr);

	if (!error)
		mc_dpl_applied = 0;

	return error;
}

int get_mc_boot_status(void)
{
	return mc_boot_status;
}

#ifdef CONFIG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET
int get_aiop_apply_status(void)
{
	return mc_aiop_applied;
}
#endif

int get_dpl_apply_status(void)
{
	return mc_dpl_applied;
}

/**
 * Return the MC address of private DRAM block.
 */
u64 mc_get_dram_addr(void)
{
	return gd->arch.resv_ram;
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

int fsl_mc_ldpaa_init(bd_t *bis)
{
	int i;

	for (i = WRIOP1_DPMAC1; i < NUM_WRIOP_PORTS; i++)
		if ((wriop_is_enabled_dpmac(i) == 1) &&
		    (wriop_get_phy_address(i) != -1))
			ldpaa_eth_init(i, wriop_get_enet_if(i));
	return 0;
}

static int dprc_version_check(struct fsl_mc_io *mc_io, uint16_t handle)
{
	struct dprc_attributes attr;
	int error;

	memset(&attr, 0, sizeof(struct dprc_attributes));
	error = dprc_get_attributes(mc_io, MC_CMD_NO_FLAGS, handle, &attr);
	if (error == 0) {
		if ((attr.version.major != DPRC_VER_MAJOR) ||
		    (attr.version.minor != DPRC_VER_MINOR)) {
			printf("DPRC version mismatch found %u.%u,",
			       attr.version.major,
			       attr.version.minor);
			printf("supported version is %u.%u\n",
			       DPRC_VER_MAJOR, DPRC_VER_MINOR);
		}
	}
	return error;
}

static int dpio_init(void)
{
	struct qbman_swp_desc p_des;
	struct dpio_attr attr;
	struct dpio_cfg dpio_cfg;
	int err = 0;

	dflt_dpio = (struct fsl_dpio_obj *)malloc(sizeof(struct fsl_dpio_obj));
	if (!dflt_dpio) {
		printf("No memory: malloc() failed\n");
		err = -ENOMEM;
		goto err_malloc;
	}

	dpio_cfg.channel_mode = DPIO_LOCAL_CHANNEL;
	dpio_cfg.num_priorities = 8;

	err = dpio_create(dflt_mc_io, MC_CMD_NO_FLAGS, &dpio_cfg,
			  &dflt_dpio->dpio_handle);
	if (err < 0) {
		printf("dpio_create() failed: %d\n", err);
		err = -ENODEV;
		goto err_create;
	}

	memset(&attr, 0, sizeof(struct dpio_attr));
	err = dpio_get_attributes(dflt_mc_io, MC_CMD_NO_FLAGS,
				  dflt_dpio->dpio_handle, &attr);
	if (err < 0) {
		printf("dpio_get_attributes() failed: %d\n", err);
		goto err_get_attr;
	}

	if ((attr.version.major != DPIO_VER_MAJOR) ||
	    (attr.version.minor != DPIO_VER_MINOR)) {
		printf("DPIO version mismatch found %u.%u,",
		       attr.version.major, attr.version.minor);
		printf("supported version is %u.%u\n",
		       DPIO_VER_MAJOR, DPIO_VER_MINOR);
	}

	dflt_dpio->dpio_id = attr.id;
#ifdef DEBUG
	printf("Init: DPIO id=0x%d\n", dflt_dpio->dpio_id);
#endif
	err = dpio_enable(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpio->dpio_handle);
	if (err < 0) {
		printf("dpio_enable() failed %d\n", err);
		goto err_get_enable;
	}
	debug("ce_offset=0x%llx, ci_offset=0x%llx, portalid=%d, prios=%d\n",
	      attr.qbman_portal_ce_offset,
	      attr.qbman_portal_ci_offset,
	      attr.qbman_portal_id,
	      attr.num_priorities);

	p_des.cena_bar = (void *)(SOC_QBMAN_PORTALS_BASE_ADDR
					+ attr.qbman_portal_ce_offset);
	p_des.cinh_bar = (void *)(SOC_QBMAN_PORTALS_BASE_ADDR
					+ attr.qbman_portal_ci_offset);

	dflt_dpio->sw_portal = qbman_swp_init(&p_des);
	if (dflt_dpio->sw_portal == NULL) {
		printf("qbman_swp_init() failed\n");
		goto err_get_swp_init;
	}
	return 0;

err_get_swp_init:
	dpio_disable(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpio->dpio_handle);
err_get_enable:
err_get_attr:
	dpio_close(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpio->dpio_handle);
	dpio_destroy(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpio->dpio_handle);
err_create:
	free(dflt_dpio);
err_malloc:
	return err;
}

static int dpio_exit(void)
{
	int err;

	err = dpio_disable(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpio->dpio_handle);
	if (err < 0) {
		printf("dpio_disable() failed: %d\n", err);
		goto err;
	}

	err = dpio_destroy(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpio->dpio_handle);
	if (err < 0) {
		printf("dpio_destroy() failed: %d\n", err);
		goto err;
	}

#ifdef DEBUG
	printf("Exit: DPIO id=0x%d\n", dflt_dpio->dpio_id);
#endif

	if (dflt_dpio)
		free(dflt_dpio);

	return 0;
err:
	return err;
}

static int dprc_init(void)
{
	int err, child_portal_id, container_id;
	struct dprc_cfg cfg;
	uint64_t mc_portal_offset;

	/* Open root container */
	err = dprc_get_container_id(root_mc_io, MC_CMD_NO_FLAGS, &container_id);
	if (err < 0) {
		printf("dprc_get_container_id(): Root failed: %d\n", err);
		goto err_root_container_id;
	}

#ifdef DEBUG
	printf("Root container id = %d\n", container_id);
#endif
	err = dprc_open(root_mc_io, MC_CMD_NO_FLAGS, container_id,
			&root_dprc_handle);
	if (err < 0) {
		printf("dprc_open(): Root Container failed: %d\n", err);
		goto err_root_open;
	}

	if (!root_dprc_handle) {
		printf("dprc_open(): Root Container Handle is not valid\n");
		goto err_root_open;
	}

	err = dprc_version_check(root_mc_io, root_dprc_handle);
	if (err < 0) {
		printf("dprc_version_check() failed: %d\n", err);
		goto err_root_open;
	}

	memset(&cfg, 0, sizeof(struct dprc_cfg));
	cfg.options = DPRC_CFG_OPT_TOPOLOGY_CHANGES_ALLOWED |
		      DPRC_CFG_OPT_OBJ_CREATE_ALLOWED |
		      DPRC_CFG_OPT_ALLOC_ALLOWED;
	cfg.icid = DPRC_GET_ICID_FROM_POOL;
	cfg.portal_id = DPRC_GET_PORTAL_ID_FROM_POOL;
	err = dprc_create_container(root_mc_io, MC_CMD_NO_FLAGS,
			root_dprc_handle,
			&cfg,
			&child_dprc_id,
			&mc_portal_offset);
	if (err < 0) {
		printf("dprc_create_container() failed: %d\n", err);
		goto err_create;
	}

	dflt_mc_io = (struct fsl_mc_io *)malloc(sizeof(struct fsl_mc_io));
	if (!dflt_mc_io) {
		err  = -ENOMEM;
		printf(" No memory: malloc() failed\n");
		goto err_malloc;
	}

	child_portal_id = MC_PORTAL_OFFSET_TO_PORTAL_ID(mc_portal_offset);
	dflt_mc_io->mmio_regs = SOC_MC_PORTAL_ADDR(child_portal_id);
#ifdef DEBUG
	printf("MC portal of child DPRC container: %d, physical addr %p)\n",
	       child_dprc_id, dflt_mc_io->mmio_regs);
#endif

	err = dprc_open(dflt_mc_io, MC_CMD_NO_FLAGS, child_dprc_id,
			&dflt_dprc_handle);
	if (err < 0) {
		printf("dprc_open(): Child container failed: %d\n", err);
		goto err_child_open;
	}

	if (!dflt_dprc_handle) {
		printf("dprc_open(): Child container Handle is not valid\n");
		goto err_child_open;
	}

	return 0;
err_child_open:
	free(dflt_mc_io);
err_malloc:
	dprc_destroy_container(root_mc_io, MC_CMD_NO_FLAGS,
			       root_dprc_handle, child_dprc_id);
err_create:
	dprc_close(root_mc_io, MC_CMD_NO_FLAGS, root_dprc_handle);
err_root_open:
err_root_container_id:
	return err;
}

static int dprc_exit(void)
{
	int err;

	err = dprc_close(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dprc_handle);
	if (err < 0) {
		printf("dprc_close(): Child failed: %d\n", err);
		goto err;
	}

	err = dprc_destroy_container(root_mc_io, MC_CMD_NO_FLAGS,
				     root_dprc_handle, child_dprc_id);
	if (err < 0) {
		printf("dprc_destroy_container() failed: %d\n", err);
		goto err;
	}

	err = dprc_close(root_mc_io, MC_CMD_NO_FLAGS, root_dprc_handle);
	if (err < 0) {
		printf("dprc_close(): Root failed: %d\n", err);
		goto err;
	}

	if (dflt_mc_io)
		free(dflt_mc_io);

	if (root_mc_io)
		free(root_mc_io);

	return 0;

err:
	return err;
}

static int dpbp_init(void)
{
	int err;
	struct dpbp_attr dpbp_attr;
	struct dpbp_cfg dpbp_cfg;

	dflt_dpbp = (struct fsl_dpbp_obj *)malloc(sizeof(struct fsl_dpbp_obj));
	if (!dflt_dpbp) {
		printf("No memory: malloc() failed\n");
		err = -ENOMEM;
		goto err_malloc;
	}

	dpbp_cfg.options = 512;

	err = dpbp_create(dflt_mc_io, MC_CMD_NO_FLAGS, &dpbp_cfg,
			  &dflt_dpbp->dpbp_handle);

	if (err < 0) {
		err = -ENODEV;
		printf("dpbp_create() failed: %d\n", err);
		goto err_create;
	}

	memset(&dpbp_attr, 0, sizeof(struct dpbp_attr));
	err = dpbp_get_attributes(dflt_mc_io, MC_CMD_NO_FLAGS,
				  dflt_dpbp->dpbp_handle,
				  &dpbp_attr);
	if (err < 0) {
		printf("dpbp_get_attributes() failed: %d\n", err);
		goto err_get_attr;
	}

	if ((dpbp_attr.version.major != DPBP_VER_MAJOR) ||
	    (dpbp_attr.version.minor != DPBP_VER_MINOR)) {
		printf("DPBP version mismatch found %u.%u,",
		       dpbp_attr.version.major, dpbp_attr.version.minor);
		printf("supported version is %u.%u\n",
		       DPBP_VER_MAJOR, DPBP_VER_MINOR);
	}

	dflt_dpbp->dpbp_attr.id = dpbp_attr.id;
#ifdef DEBUG
	printf("Init: DPBP id=0x%d\n", dflt_dpbp->dpbp_attr.id);
#endif

	err = dpbp_close(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpbp->dpbp_handle);
	if (err < 0) {
		printf("dpbp_close() failed: %d\n", err);
		goto err_close;
	}

	return 0;

err_close:
	free(dflt_dpbp);
err_get_attr:
	dpbp_close(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpbp->dpbp_handle);
	dpbp_destroy(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpbp->dpbp_handle);
err_create:
err_malloc:
	return err;
}

static int dpbp_exit(void)
{
	int err;

	err = dpbp_open(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpbp->dpbp_attr.id,
			&dflt_dpbp->dpbp_handle);
	if (err < 0) {
		printf("dpbp_open() failed: %d\n", err);
		goto err;
	}

	err = dpbp_destroy(dflt_mc_io, MC_CMD_NO_FLAGS,
			   dflt_dpbp->dpbp_handle);
	if (err < 0) {
		printf("dpbp_destroy() failed: %d\n", err);
		goto err;
	}

#ifdef DEBUG
	printf("Exit: DPBP id=0x%d\n", dflt_dpbp->dpbp_attr.id);
#endif

	if (dflt_dpbp)
		free(dflt_dpbp);
	return 0;

err:
	return err;
}

static int dpni_init(void)
{
	int err;
	struct dpni_attr dpni_attr;
	uint8_t	ext_cfg_buf[256] = {0};
	struct dpni_extended_cfg dpni_extended_cfg;
	struct dpni_cfg dpni_cfg;

	dflt_dpni = (struct fsl_dpni_obj *)malloc(sizeof(struct fsl_dpni_obj));
	if (!dflt_dpni) {
		printf("No memory: malloc() failed\n");
		err = -ENOMEM;
		goto err_malloc;
	}

	memset(&dpni_extended_cfg, 0, sizeof(dpni_extended_cfg));
	err = dpni_prepare_extended_cfg(&dpni_extended_cfg, &ext_cfg_buf[0]);
	if (err < 0) {
		err = -ENODEV;
		printf("dpni_prepare_extended_cfg() failed: %d\n", err);
		goto err_prepare_extended_cfg;
	}

	memset(&dpni_cfg, 0, sizeof(dpni_cfg));
	dpni_cfg.adv.options = DPNI_OPT_UNICAST_FILTER |
			       DPNI_OPT_MULTICAST_FILTER;

	dpni_cfg.adv.ext_cfg_iova = (uint64_t)&ext_cfg_buf[0];
	err = dpni_create(dflt_mc_io, MC_CMD_NO_FLAGS, &dpni_cfg,
			  &dflt_dpni->dpni_handle);

	if (err < 0) {
		err = -ENODEV;
		printf("dpni_create() failed: %d\n", err);
		goto err_create;
	}

	memset(&dpni_attr, 0, sizeof(struct dpni_attr));
	err = dpni_get_attributes(dflt_mc_io, MC_CMD_NO_FLAGS,
				  dflt_dpni->dpni_handle,
				  &dpni_attr);
	if (err < 0) {
		printf("dpni_get_attributes() failed: %d\n", err);
		goto err_get_attr;
	}

	if ((dpni_attr.version.major != DPNI_VER_MAJOR) ||
	    (dpni_attr.version.minor != DPNI_VER_MINOR)) {
		printf("DPNI version mismatch found %u.%u,",
		       dpni_attr.version.major, dpni_attr.version.minor);
		printf("supported version is %u.%u\n",
		       DPNI_VER_MAJOR, DPNI_VER_MINOR);
	}

	dflt_dpni->dpni_id = dpni_attr.id;
#ifdef DEBUG
	printf("Init: DPNI id=0x%d\n", dflt_dpni->dpni_id);
#endif

	err = dpni_close(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_handle);
	if (err < 0) {
		printf("dpni_close() failed: %d\n", err);
		goto err_close;
	}

	return 0;

err_close:
err_get_attr:
	dpni_close(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_handle);
	dpni_destroy(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_handle);
err_create:
err_prepare_extended_cfg:
	free(dflt_dpni);
err_malloc:
	return err;
}

static int dpni_exit(void)
{
	int err;

	err = dpni_open(dflt_mc_io, MC_CMD_NO_FLAGS, dflt_dpni->dpni_id,
			&dflt_dpni->dpni_handle);
	if (err < 0) {
		printf("dpni_open() failed: %d\n", err);
		goto err;
	}

	err = dpni_destroy(dflt_mc_io, MC_CMD_NO_FLAGS,
			   dflt_dpni->dpni_handle);
	if (err < 0) {
		printf("dpni_destroy() failed: %d\n", err);
		goto err;
	}

#ifdef DEBUG
	printf("Exit: DPNI id=0x%d\n", dflt_dpni->dpni_id);
#endif

	if (dflt_dpni)
		free(dflt_dpni);
	return 0;

err:
	return err;
}

static int mc_init_object(void)
{
	int err = 0;

	err = dprc_init();
	if (err < 0) {
		printf("dprc_init() failed: %d\n", err);
		goto err;
	}

	err = dpbp_init();
	if (err < 0) {
		printf("dpbp_init() failed: %d\n", err);
		goto err;
	}

	err = dpio_init();
	if (err < 0) {
		printf("dpio_init() failed: %d\n", err);
		goto err;
	}

	err = dpni_init();
	if (err < 0) {
		printf("dpni_init() failed: %d\n", err);
		goto err;
	}

	return 0;
err:
	return err;
}

int fsl_mc_ldpaa_exit(bd_t *bd)
{
	int err = 0;

	if (bd && mc_lazy_dpl_addr && !fsl_mc_ldpaa_exit(NULL)) {
		mc_apply_dpl(mc_lazy_dpl_addr);
		mc_lazy_dpl_addr = 0;
	}

	/* MC is not loaded intentionally, So return success. */
	if (bd && get_mc_boot_status() != 0)
		return 0;

	if (bd && !get_mc_boot_status() && get_dpl_apply_status() == -1) {
		printf("ERROR: fsl-mc: DPL is not applied\n");
		err = -ENODEV;
		return err;
	}

	if (bd && !get_mc_boot_status() && !get_dpl_apply_status())
		return err;

	err = dpbp_exit();
	if (err < 0) {
		printf("dpbp_exit() failed: %d\n", err);
		goto err;
	}

	err = dpio_exit();
	if (err < 0) {
		printf("dpio_exit() failed: %d\n", err);
		goto err;
	}

	err = dpni_exit();
	if (err < 0) {
		printf("dpni_exit() failed: %d\n", err);
		goto err;
	}

	err = dprc_exit();
	if (err < 0) {
		printf("dprc_exit() failed: %d\n", err);
		goto err;
	}

	return 0;
err:
	return err;
}

static int do_fsl_mc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int err = 0;
	if (argc < 3)
		goto usage;

	switch (argv[1][0]) {
	case 's': {
			char sub_cmd;
			u64 mc_fw_addr, mc_dpc_addr;
#ifdef CONFIG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET
			u64 aiop_fw_addr;
#endif

			sub_cmd = argv[2][0];
			switch (sub_cmd) {
			case 'm':
				if (argc < 5)
					goto usage;

				if (get_mc_boot_status() == 0) {
					printf("fsl-mc: MC is already booted");
					printf("\n");
					return err;
				}
				mc_fw_addr = simple_strtoull(argv[3], NULL, 16);
				mc_dpc_addr = simple_strtoull(argv[4], NULL,
							      16);

				if (!mc_init(mc_fw_addr, mc_dpc_addr))
					err = mc_init_object();
				break;

#ifdef CONFIG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET
			case 'a':
				if (argc < 4)
					goto usage;
				if (get_aiop_apply_status() == 0) {
					printf("fsl-mc: AIOP FW is already");
					printf(" applied\n");
					return err;
				}

				aiop_fw_addr = simple_strtoull(argv[3], NULL,
							       16);

				/* if SoC doesn't have AIOP, err = -ENODEV */
				err = load_mc_aiop_img(aiop_fw_addr);
				if (!err)
					printf("fsl-mc: AIOP FW applied\n");
				break;
#endif
			default:
				printf("Invalid option: %s\n", argv[2]);
				goto usage;

				break;
			}
		}
		break;

	case 'l':
	case 'a': {
			u64 mc_dpl_addr;

			if (argc < 4)
				goto usage;

			if (get_dpl_apply_status() == 0) {
				printf("fsl-mc: DPL already applied\n");
				return err;
			}

			mc_dpl_addr = simple_strtoull(argv[3], NULL,
							      16);

			if (get_mc_boot_status() != 0) {
				printf("fsl-mc: Deploying data path layout ..");
				printf("ERROR (MC is not booted)\n");
				return -ENODEV;
			}

			if (argv[1][0] == 'l') {
				/*
				 * We will do the actual dpaa exit and dpl apply
				 * later from announce_and_cleanup().
				 */
				mc_lazy_dpl_addr = mc_dpl_addr;
			} else {
				/* The user wants it applied now */
				if (!fsl_mc_ldpaa_exit(NULL))
					err = mc_apply_dpl(mc_dpl_addr);
			}
			break;
		}
	default:
		printf("Invalid option: %s\n", argv[1]);
		goto usage;
		break;
	}
	return err;
 usage:
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	fsl_mc,  CONFIG_SYS_MAXARGS,  1,   do_fsl_mc,
	"DPAA2 command to manage Management Complex (MC)",
	"start mc [FW_addr] [DPC_addr] - Start Management Complex\n"
	"fsl_mc apply DPL [DPL_addr] - Apply DPL file\n"
	"fsl_mc lazyapply DPL [DPL_addr] - Apply DPL file on exit\n"
	"fsl_mc start aiop [FW_addr] - Start AIOP\n"
);
