// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <dm.h>
#include <hang.h>
#include <log.h>
#include <ram.h>
#include <reset.h>
#include <wait_bit.h>
#include <wdt.h>
#include <asm/arch/system_manager.h>
#include <linux/bitfield.h>
#include "iossm_mailbox.h"
#include "sdram_soc64.h"
#include "uibssm_mailbox.h"

/* NOCPLL register */
#define SYSMGR_HMC_CLK		0xB4
#define SYSMGR_HMC_CLK_NOCPLL	BIT(8)

/* MPFE NOC registers */
#define F2SDRAM_SIDEBAND_FLAGOUTSET0	0x50
#define F2SDRAM_SIDEBAND_FLAGOUTSTATUS0	0x58
#define SIDEBANDMGR_FLAGOUTSET0_REG	SOCFPGA_F2SDRAM_MGR_ADDRESS +\
					F2SDRAM_SIDEBAND_FLAGOUTSET0
#define SIDEBANDMGR_FLAGOUTSTATUS0_REG	SOCFPGA_F2SDRAM_MGR_ADDRESS +\
					F2SDRAM_SIDEBAND_FLAGOUTSTATUS0

#define SIDEBANDMGR_FLAGOUTSET0_REG_MULTICHANNEL	BIT(4)
#define SIDEBANDMGR_FLAGOUTSET0_REG_INTERLEAVING	BIT(5)

/* Reset type */
enum reset_type {
	POR_RESET,
	WARM_RESET,
	COLD_RESET,
	NCONFIG,
	JTAG_CONFIG,
	RSU_RECONFIG
};

phys_addr_t io96b_csr_reg_addr[] = {
	0xf8400000, /* IO96B_0 CSR registers address */
	0xf8800000  /* IO96B_1 CSR registers address */
};

phys_addr_t uib_csr_reg_addr[] = {
	0xf8400000, /* UIB_0 CSR registers address */
	0xf8410000, /* UIB_1 CSR registers address */
	0xf8420000, /* UIB_2 CSR registers address */
	0xf8430000, /* UIB_3 CSR registers address */
	0xf8440000, /* UIB_4 CSR registers address */
	0xf8450000, /* UIB_5 CSR registers address */
	0xf8460000, /* UIB_6 CSR registers address */
	0xf8470000  /* UIB_7 CSR registers address */
};

static enum reset_type get_reset_type(u32 reg)
{
	return FIELD_GET(ALT_SYSMGR_SCRATCH_REG_0_DDR_RESET_TYPE_MASK, reg);
}

bool is_ddr_init_hang(void)
{
	u32 reg = readl(socfpga_get_sysmgr_addr() +
			SYSMGR_SOC64_BOOT_SCRATCH_COLD8);

	debug("%s: 0x%x\n", __func__, reg);

	if (reg & ALT_SYSMGR_SCRATCH_REG_8_DDR_PROGRESS_MASK)
		return true;

	return false;
}

void ddr_init_inprogress(bool start)
{
	if (start)
		setbits_le32(socfpga_get_sysmgr_addr() +
			     SYSMGR_SOC64_BOOT_SCRATCH_COLD8,
			     ALT_SYSMGR_SCRATCH_REG_8_DDR_PROGRESS_MASK);
	else
		clrbits_le32(socfpga_get_sysmgr_addr() +
			     SYSMGR_SOC64_BOOT_SCRATCH_COLD8,
			     ALT_SYSMGR_SCRATCH_REG_8_DDR_PROGRESS_MASK);
}

static const char *memory_type_in_use(struct udevice *dev)
{
	struct altera_sdram_plat *plat = dev_get_plat(dev);

	return (plat->mem_type == DDR_MEMORY ? "DDR" : "HBM");
}

static bool is_ddr_in_use(struct udevice *dev)
{
	struct altera_sdram_plat *plat = dev_get_plat(dev);

	return (plat->mem_type == DDR_MEMORY ? true : false);
}

int populate_ddr_handoff(struct udevice *dev, struct io96b_info *io96b_ctrl,
			 struct uib_info *uib_ctrl)
{
	struct altera_sdram_plat *plat = dev_get_plat(dev);
	int i;
	u8 count = 0;
	u32 len = SOC64_HANDOFF_DDR_LEN;
	u32 handoff_table[len];

	/* Read handoff for DDR configuration */
	socfpga_handoff_read((void *)SOC64_HANDOFF_DDR_BASE, handoff_table, len);

	/* Interleaving Mode */
	if (handoff_table[0] & SOC64_HANDOFF_DDR_INTERLEAVING_MODE_MASK)
		plat->multichannel_interleaving = true;
	else
		plat->multichannel_interleaving = false;

	debug("%s: MPFE-EMIF is in %s mode\n", __func__,
	      plat->multichannel_interleaving ? "interleaving" : "multichannel");

	/* Memory type */
	if (handoff_table[2] & SOC64_HANDOFF_DDR_MEMORY_TYPE_MASK)
		plat->mem_type = HBM_MEMORY;
	else
		plat->mem_type = DDR_MEMORY;

	debug("%s: Memory type is %s\n", __func__
			, plat->mem_type ? "HBM" : "DDR");

	if (plat->mem_type == HBM_MEMORY) {
		/* Assign UIB CSR base address if it is valid */
		for (i = 0; i < MAX_UIB_SUPPORTED; i++) {
			if (handoff_table[3] & BIT(i)) {
				uib_ctrl->uib[count].uib_csr_addr = uib_csr_reg_addr[i];
				debug("%s: UIB 0x%llx CSR enabled\n", __func__
					, uib_ctrl->uib[count].uib_csr_addr);
				count++;
			}
		}

		uib_ctrl->num_instance = count;
		debug("%s: returned num_instance 0x%x\n", __func__, uib_ctrl->num_instance);

		/*
		 * HBM memory size
		 * 1 UIB channel has 2 pseudo channels
		 * 1 pseudo channel is 1GB, hence 1 UIB channel is 2GB
		 */
		uib_ctrl->overall_size = (phys_size_t)uib_ctrl->num_instance * SZ_2G;

		/* UIB ECC status */
		uib_ctrl->ecc_status = handoff_table[4];

		debug("%s: ECC status 0x%x\n", __func__, uib_ctrl->ecc_status);
	} else {
		/* Assign IO96B CSR base address if it is valid */
		for (i = 0; i < MAX_IO96B_SUPPORTED; i++) {
			if (handoff_table[1] & BIT(i)) {
				io96b_ctrl->io96b[i].io96b_csr_addr = io96b_csr_reg_addr[i];

				debug("%s: IO96B 0x%llx CSR enabled\n", __func__,
				      io96b_ctrl->io96b[i].io96b_csr_addr);

				count++;
			}
		}

		io96b_ctrl->num_instance = count;
		debug("%s: returned num_instance 0x%x\n", __func__, io96b_ctrl->num_instance);
	}

	return 0;
}

int config_mpfe_sideband_mgr(struct udevice *dev)
{
	struct altera_sdram_plat *plat = dev_get_plat(dev);
	u32 reg, mask;
	int ret = 0;

	if (plat->multichannel_interleaving) {
		mask = SIDEBANDMGR_FLAGOUTSET0_REG_INTERLEAVING;
		setbits_le32(SIDEBANDMGR_FLAGOUTSET0_REG, mask);
	} else {
		mask = SIDEBANDMGR_FLAGOUTSET0_REG_MULTICHANNEL;
		setbits_le32(SIDEBANDMGR_FLAGOUTSET0_REG, mask);
	}

	reg = readl(SIDEBANDMGR_FLAGOUTSTATUS0_REG);

	debug("%s: F2SDRAM_SIDEBAND_FLAGOUTSTATUS0: 0x%x\n", __func__, reg);

	if ((reg & mask) == SIDEBANDMGR_FLAGOUTSET0_REG_INTERLEAVING)
		debug("%s: Interleaving bit is set\n", __func__);
	else if ((reg & mask) == SIDEBANDMGR_FLAGOUTSET0_REG_MULTICHANNEL)
		debug("%s: Multichannel bit is set\n", __func__);
	else
		ret = -EINVAL;

	return ret;
}

bool hps_ocram_dbe_status(void)
{
	u32 reg = readl(socfpga_get_sysmgr_addr() +
			SYSMGR_SOC64_BOOT_SCRATCH_COLD8);

	if (reg & ALT_SYSMGR_SCRATCH_REG_8_OCRAM_DBE_MASK)
		return true;

	return false;
}

bool ddr_ecc_dbe_status(void)
{
	u32 reg = readl(socfpga_get_sysmgr_addr() +
			SYSMGR_SOC64_BOOT_SCRATCH_COLD8);

	if (reg & ALT_SYSMGR_SCRATCH_REG_8_DDR_DBE_MASK)
		return true;

	return false;
}

int sdram_mmr_init_full(struct udevice *dev)
{
	int i, ret = 0;
	struct altera_sdram_plat *plat = dev_get_plat(dev);
	struct altera_sdram_priv *priv = dev_get_priv(dev);
	struct io96b_info *io96b_ctrl = malloc(sizeof(*io96b_ctrl));
	struct uib_info *uib_ctrl = malloc(sizeof(*uib_ctrl));
	struct bd_info bd = {0};
	bool full_mem_init = false, is_ddr_hang_be4_rst = false;
	phys_size_t hw_size;
	u32 reg = readl(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_BOOT_SCRATCH_COLD0);
	enum reset_type reset_t = get_reset_type(reg);

	/* Populating DDR handoff data */
	debug("DDR: Populating DDR handoff\n");

	ret = populate_ddr_handoff(dev, io96b_ctrl, uib_ctrl);
	if (ret) {
		printf("DDR: Failed to populate DDR handoff\n");

		goto err;
	}

	debug("%s: Address MPFE 0x%llx\n", memory_type_in_use(dev), plat->mpfe_base_addr);

	/* DDR initialization progress status tracking */
	is_ddr_hang_be4_rst = is_ddr_init_hang();

	printf("%s: SDRAM init in progress ...\n", memory_type_in_use(dev));

	ddr_init_inprogress(true);

	/* Configuring MPFE sideband manager registers - multichannel or interleaving*/
	debug("%s: MPFE configuration in progress ...\n", memory_type_in_use(dev));

	ret = config_mpfe_sideband_mgr(dev);
	if (ret) {
		printf("%s: Failed to configure multichannel/interleaving mode\n",
		       memory_type_in_use(dev));

		goto err;
	}

	debug("%s: MPFE configuration completed\n", memory_type_in_use(dev));

	debug("%s: Waiting for NOCPLL locked ...\n", memory_type_in_use(dev));

	/* Ensure NOCPLL locked */
	ret = wait_for_bit_le32((const void *)socfpga_get_sysmgr_addr() + SYSMGR_HMC_CLK
				, SYSMGR_HMC_CLK_NOCPLL, true, TIMEOUT, false);
	if (ret) {
		printf("%s: NOCPLL is not locked\n", memory_type_in_use(dev));

		goto err;
	}

	debug("%s: NOCPLL locked\n", memory_type_in_use(dev));

	debug("%s: Checking calibration...\n", memory_type_in_use(dev));

	if (is_ddr_in_use(dev)) {
		/* Configure if polling is needed for IO96B GEN PLL locked */
		io96b_ctrl->ckgen_lock = false;

		/* Ensure calibration status passing */
		init_mem_cal(io96b_ctrl);

		printf("DDR: Calibration success\n");

		/* Initiate IOSSM mailbox */
		io96b_mb_init(io96b_ctrl);

		/* DDR type */
		ret = get_mem_technology(io96b_ctrl);
		if (ret) {
			printf("DDR: Failed to get DDR type\n");

			goto err;
		}

		/* DDR size */
		ret = get_mem_width_info(io96b_ctrl);
		if (ret) {
			printf("DDR: Failed to get DDR size\n");

			goto err;
		}
	} else {
		/* Ensure calibration status passing */
		uib_init_mem_cal(uib_ctrl);

		/* Need to trigger re-calibration for HBM DBE */
		if (ddr_ecc_dbe_status()) {
			for (i = 0; i < uib_ctrl->num_instance; i++)
				uib_ctrl->uib[i].cal_status = false;

			uib_ctrl->overall_cal_status = false;
		}

		/* Trigger re-calibration if calibration failed */
		if (!(uib_ctrl->overall_cal_status)) {
			printf("HBM: Re-calibration in progress...\n");

			uib_trig_mem_cal(uib_ctrl);
		}

		if (!(uib_ctrl->overall_cal_status)) {
			printf("HBM: Retry calibration failed & not able to re-calibrate\n");

			ret = -EINVAL;
			goto err;
		}

		debug("HBM: Setting Error Mask Register\n");

		/* Responder Error Mask Register */
		for (i = 0; i < uib_ctrl->num_instance; i++) {
			clrsetbits_le32(uib_ctrl->uib[i].uib_csr_addr +
					UIB_R_ERRMSK_PSEUDO_CH0_OFFSET,
					UIB_DRAM_SBE_MSK | UIB_INTERNAL_CORR_ERR_MSK,
					UIB_DRAM_SBE(0x1) | UIB_INTERNAL_CORR_ERR(0x1));

			debug("HBM: Error Mask Pseudo CH0 addr: 0x%llx\n",
			      uib_ctrl->uib[i].uib_csr_addr +
			      UIB_R_ERRMSK_PSEUDO_CH0_OFFSET);

			debug("HBM: Error Mask Pseudo CH0 value: 0x%x\n",
			      readl(uib_ctrl->uib[i].uib_csr_addr +
			      UIB_R_ERRMSK_PSEUDO_CH0_OFFSET));

			clrsetbits_le32(uib_ctrl->uib[i].uib_csr_addr +
					UIB_R_ERRMSK_PSEUDO_CH1_OFFSET,
					UIB_DRAM_SBE_MSK | UIB_INTERNAL_CORR_ERR_MSK,
					UIB_DRAM_SBE(0x1) | UIB_INTERNAL_CORR_ERR(0x1));

			debug("HBM: Error Mask Pseudo CH1 addr: 0x%llx\n",
			      uib_ctrl->uib[i].uib_csr_addr +
			      UIB_R_ERRMSK_PSEUDO_CH1_OFFSET);

			debug("HBM: Error Mask Pseudo CH1 value: 0x%x\n\n",
			      readl(uib_ctrl->uib[i].uib_csr_addr +
			      UIB_R_ERRMSK_PSEUDO_CH1_OFFSET));
		}

		printf("HBM: Calibration success\n");
	}

	/* Get bank configuration from devicetree */
	ret = fdtdec_decode_ram_size(gd->fdt_blob, NULL, 0, NULL,
				     (phys_size_t *)&gd->ram_size, &bd);
	if (ret) {
		printf("%s: Failed to decode memory node\n", memory_type_in_use(dev));

		goto err;
	}

	if (!is_ddr_in_use(dev)) {
		hw_size = uib_ctrl->overall_size;
	} else {
		/* ECC status */
		ret = ecc_enable_status(io96b_ctrl);
		if (ret) {
			printf("DDR: Failed to get DDR ECC status\n");

			goto err;
		}

		hw_size = io96b_ctrl->overall_size;

		if (io96b_ctrl->inline_ecc)
			hw_size = CALC_INLINE_ECC_HW_SIZE(hw_size);
	}

	if (gd->ram_size != hw_size) {
		printf("%s: Warning: DRAM size from device tree (%lld MiB)\n",
		       memory_type_in_use(dev), gd->ram_size >> 20);
		printf(" mismatch with hardware (%lld MiB).\n",
		       hw_size >> 20);
	}

	if (gd->ram_size > hw_size) {
		printf("%s: Error: DRAM size from device tree is greater\n",
		       memory_type_in_use(dev));
		printf(" than hardware size.\n");

		hang();
	}

	printf("%s: %lld MiB\n", (is_ddr_in_use(dev) ? io96b_ctrl->ddr_type : "HBM"),
	       gd->ram_size >> 20);

	if (is_ddr_in_use(dev)) {
		/*
		 * Is HPS cold or warm reset? If yes, Skip full memory initialization if ECC
		 * enabled to preserve memory content
		 */
		if (io96b_ctrl->ecc_status) {
			if (ecc_interrupt_status(io96b_ctrl)) {
				if (CONFIG_IS_ENABLED(WDT)) {
					struct udevice *wdt;

					printf("DDR: ECC error recover start now\n");
					ret = uclass_first_device_err(UCLASS_WDT, &wdt);
					if (ret) {
						printf("DDR: Failed to trigger watchdog reset\n");
						hang();
					}

					wdt_expire_now(wdt, 0);
				}
				hang();
			}

			full_mem_init = hps_ocram_dbe_status() | is_ddr_hang_be4_rst;
			if (full_mem_init || !(reset_t == WARM_RESET || reset_t == COLD_RESET)) {
				debug("%s: Needed to fully initialize DDR memory\n",
				      io96b_ctrl->ddr_type);

				ret = bist_mem_init_start(io96b_ctrl);
				if (ret) {
					printf("%s: Failed to fully initialize DDR memory\n",
					       io96b_ctrl->ddr_type);

					goto err;
				}
			}
		}
	} else {
		debug("HBM: ECC enable status: %d\n", uib_ctrl->ecc_status);

		/*
		 * Is HPS cold or warm reset? If yes, Skip full memory initialization if ECC
		 * enabled to preserve memory content
		 */
		if (uib_ctrl->ecc_status) {
			full_mem_init = hps_ocram_dbe_status() | ddr_ecc_dbe_status() |
					is_ddr_hang_be4_rst;
			if (full_mem_init || !(reset_t == WARM_RESET || reset_t == COLD_RESET)) {
				debug("HBM: Needed to fully initialize HBM memory\n");

				ret = uib_bist_mem_init_start(uib_ctrl);
				if (ret) {
					printf("HBM: Failed to fully initialize HBM memory\n");

					goto err;
				}
			}
		}
	}

	/* Ensure sanity memory test passing */
	sdram_size_check(&bd);
	printf("%s: size check success\n", (is_ddr_in_use(dev) ? io96b_ctrl->ddr_type : "HBM"));

	sdram_set_firewall(&bd);

	printf("%s: firewall init success\n", (is_ddr_in_use(dev) ? io96b_ctrl->ddr_type : "HBM"));

	priv->info.base = bd.bi_dram[0].start;
	priv->info.size = gd->ram_size;

	/* Ending DDR driver initialization success tracking */
	ddr_init_inprogress(false);

	printf("%s init success\n", (is_ddr_in_use(dev) ? io96b_ctrl->ddr_type : "HBM"));

err:
	free(io96b_ctrl);
	free(uib_ctrl);

	return ret;
}
