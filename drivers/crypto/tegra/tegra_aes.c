// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for NVIDIA Tegra AES hardware engine residing inside the Bit Stream Engine
 * for Video (BSEV) hardware block and Bit Stream Engine for Audio (BSEA).
 *
 * The programming sequence for this engine is with the help of commands which travel
 * via a command queue residing between the CPU and the BSEV/BSEA block.
 *
 * The hardware key table length is 64 bytes and each key slot divided as follows:
 * 1. Key - 32 bytes
 * 2. Original IV - 16 bytes
 * 3. Updated IV - 16 bytes
 *
 * The engine has 4 slots in T20/T30 in which 0th contains SBK loaded by bootrom,
 * vendor bootloaders tend to clear this slot so that anything booted after can't
 * use the SBK. This is relevant for U-Boot's chainloaded from these vendor bootloaders.
 *
 * Copyright (c) 2010-2011, NVIDIA Corporation
 * Copyright (c) 2025, Ion Agorria.
 */

#include <dm.h>
#include <asm/io.h>
#include <malloc.h>
#include <time.h>
#include <linux/delay.h>
#include <clk.h>
#include <reset.h>
#include <uboot_aes.h>

#include <asm/arch-tegra/crypto.h>
#include <asm/arch-tegra/fuse.h>

/* Make sure pointers will fit register size for AES engine */
static_assert(sizeof(void *) == sizeof(u32));

#define IRAM_BASE				0x40000000

#define TEGRA_AES_DMA_BUFFER_SIZE		(0x4000 / AES_BLOCK_LENGTH)
#define TEGRA_AES_HW_MAX_KEY_LENGTH		AES256_KEY_LENGTH
#define TEGRA_AES_HW_TABLE_LENGTH		(TEGRA_AES_HW_MAX_KEY_LENGTH + AES_BLOCK_LENGTH * 2)
#define TEGRA_AES_IRAM_MAX_ADDR			(IRAM_BASE | TEGRA_AES_KEYTABLEADDR_FIELD)

#define TEGRA_AES_BUSY_TIMEOUT_MS		1000

/* Registers */
#define TEGRA_AES_ICMDQUE_WR			0x000
#define TEGRA_AES_CMDQUE_CONTROL		0x008
#define TEGRA_AES_INTR_STATUS			0x018
#define TEGRA_AES_INT_ENB			0x040
#define TEGRA_AES_BSE_CFG			0x044
#define TEGRA_AES_IRAM_ACCESS_CFG		0x0a0
#define TEGRA_AES_SECURE_DEST_ADDR		0x100
#define TEGRA_AES_SECURE_INPUT_SELECT		0x104
#define TEGRA_AES_SECURE_CONFIG			0x108
#define TEGRA_AES_SECURE_CONFIG_EXT		0x10c

/* Register field macros */
#define TEGRA_AES_ENGINE_BUSY_FIELD		BIT(0)
#define TEGRA_AES_ICQ_EMPTY_FIELD		BIT(3)
#define TEGRA_AES_DMA_BUSY_FIELD		BIT(23)
#define TEGRA_AES_SECURE_KEY_SCH_DIS_FIELD	BIT(15)
#define TEGRA_AES_KEYTABLEADDR_FIELD		(BIT(17) - 1)
#define TEGRA_AES_SECURE_KEY_INDEX_SHIFT	20
#define TEGRA_AES_SECURE_KEY_INDEX_FIELD	(0x1f << TEGRA_AES_SECURE_KEY_INDEX_SHIFT)
#define TEGRA_AES_SECURE_CTR_CNT_SHIFT		16
#define TEGRA_AES_SECURE_CTR_CNT_FIELD		(0xffff << TEGRA_AES_SECURE_CTR_CNT_SHIFT)
#define TEGRA_AES_BSE_MODE_FIELD		0x1f
#define TEGRA_AES_BSE_LITTLE_ENDIAN_FIELD	BIT(10)
#define TEGRA_AES_CMDQ_OPCODE_SHIFT		26
#define TEGRA_AES_CMDQ_CTRL_ICMDQEN_FIELD	BIT(1)
#define TEGRA_AES_CMDQ_CTRL_SRC_STM_SEL_FIELD	BIT(4)
#define TEGRA_AES_CMDQ_CTRL_DST_STM_SEL_FIELD	BIT(5)
#define TEGRA_AES_SECURE_INPUT_ALG_SEL_SHIFT	28
#define TEGRA_AES_SECURE_INPUT_KEY_LEN_SHIFT	16
#define TEGRA_AES_SECURE_INPUT_IV_FIELD		BIT(10)
#define TEGRA_AES_SECURE_INPUT_HASH_ENB_FIELD	BIT(2)
#define TEGRA_AES_SECURE_CORE_SEL_SHIFT		9
#define TEGRA_AES_SECURE_VCTRAM_SEL_SHIFT	7
#define TEGRA_AES_SECURE_XOR_POS_SHIFT		3
#define TEGRA_AES_INT_ERROR_MASK		0x6ff000

/* Commands for BSEV/BSEA */
#define TEGRA_AES_CMD_BLKSTARTENGINE		0x0e
#define TEGRA_AES_CMD_DMASETUP			0x10
#define TEGRA_AES_CMD_DMACOMPLETE		0x11
#define TEGRA_AES_CMD_SETTABLE			0x15

/* Flags for mode */
#define TEGRA_AES_MODE_ENCRYPT			BIT(0)
#define TEGRA_AES_MODE_CBC			BIT(1)
#define TEGRA_AES_MODE_UPDATE_IV		BIT(2)
#define TEGRA_AES_MODE_HASH			BIT(3)

struct tegra_aes_priv {
	void *regs;
	void *iram_addr;

	struct reset_ctl reset_ctl;
	struct reset_ctl reset_ctl_vde;

	struct clk *clk;
	struct clk *clk_parent;

	u8 current_key_size;
	bool sbk_available;
};

static bool tegra_aes_is_busy(struct tegra_aes_priv *priv, bool dma_wait)
{
	u32 value = readl(priv->regs + TEGRA_AES_INTR_STATUS);
	bool engine_busy = value & TEGRA_AES_ENGINE_BUSY_FIELD;
	bool non_empty_queue = !(value & TEGRA_AES_ICQ_EMPTY_FIELD);
	bool dma_busy = dma_wait && (value & TEGRA_AES_DMA_BUSY_FIELD);

	log_debug("%s - e:%d q:%d dma:%d\n", __func__, engine_busy, non_empty_queue, dma_busy);

	return engine_busy || non_empty_queue || dma_busy;
}

static u32 tegra_aes_check_error(struct tegra_aes_priv *priv)
{
	u32 value = readl(priv->regs + TEGRA_AES_INTR_STATUS) & TEGRA_AES_INT_ERROR_MASK;

	if (value) {
		writel(TEGRA_AES_INT_ERROR_MASK, priv->regs + TEGRA_AES_INTR_STATUS);
		log_debug("%s 0x%x\n", __func__, value);
	}

	return value;
}

static int tegra_aes_wait_for_idle_dma(struct tegra_aes_priv *priv, bool dma_wait)
{
	ulong start = get_timer(0);

	while (tegra_aes_is_busy(priv, dma_wait)) {
		if (get_timer(start) > TEGRA_AES_BUSY_TIMEOUT_MS) {
			log_err("%s: TIMEOUT!!!\n", __func__);
			break;
		}
		mdelay(5);
	}

	if (tegra_aes_check_error(priv))
		return -1;

	return 0;
}

static int tegra_aes_wait_for_idle(struct tegra_aes_priv *priv)
{
	return tegra_aes_wait_for_idle_dma(priv, 1);
}

static int tegra_aes_configure(struct tegra_aes_priv *priv)
{
	u32 value;

	if (tegra_aes_wait_for_idle(priv))
		return -1;

	/* IRAM config */
	writel(0, priv->regs + TEGRA_AES_IRAM_ACCESS_CFG);

	/* Reset interrupts bits, or engine will hang on next operation */
	writel(0xFFFFFFFF, priv->regs + TEGRA_AES_INTR_STATUS);

	/* Set interrupts */
	writel(0, priv->regs + TEGRA_AES_INT_ENB);

	/* Configure CMDQUE */
	value = readl(priv->regs + TEGRA_AES_CMDQUE_CONTROL);
	value |= TEGRA_AES_CMDQ_CTRL_SRC_STM_SEL_FIELD |
		 TEGRA_AES_CMDQ_CTRL_DST_STM_SEL_FIELD |
		 TEGRA_AES_CMDQ_CTRL_ICMDQEN_FIELD;
	writel(value, priv->regs + TEGRA_AES_CMDQUE_CONTROL);

	value = readl(priv->regs + TEGRA_AES_SECURE_CONFIG_EXT);
	value &= ~TEGRA_AES_SECURE_CTR_CNT_FIELD;
	writel(value, priv->regs + TEGRA_AES_SECURE_CONFIG_EXT);

	/* Configure BSE */
	value = readl(priv->regs + TEGRA_AES_BSE_CFG);
	value &= ~TEGRA_AES_BSE_MODE_FIELD;
	value |= TEGRA_AES_BSE_LITTLE_ENDIAN_FIELD;
	writel(value, priv->regs + TEGRA_AES_BSE_CFG);

	return 0;
}

static int tegra_aes_select_key_slot(struct tegra_aes_priv *priv, u32 key_size, u8 slot)
{
	if (tegra_aes_wait_for_idle(priv))
		return -1;

	if (key_size < (AES128_KEY_LENGTH * 8) ||
	    key_size > (TEGRA_AES_HW_MAX_KEY_LENGTH * 8))
		return -EINVAL;

	priv->current_key_size = key_size;

	/* Select the key slot */
	u32 value = readl(priv->regs + TEGRA_AES_SECURE_CONFIG);

	value &= ~TEGRA_AES_SECURE_KEY_INDEX_FIELD;
	value |= (slot << TEGRA_AES_SECURE_KEY_INDEX_SHIFT);
	writel(value, priv->regs + TEGRA_AES_SECURE_CONFIG);

	return 0;
}

static int tegra_aes_call_engine(struct tegra_aes_priv *priv, u8 *src, u8 *dst,
				 u32 nblocks, u32 mode)
{
	u32 value;
	const u32 ICMDQ_LENGTH = 4;
	u32 cmdq[ICMDQ_LENGTH];

	log_debug("%s: 0x%p -> 0x%p blocks %d mode 0x%x\n", __func__,
		  src, dst, nblocks, mode);

	if (!nblocks) {
		log_warning("%s: called with 0 blocks!\n", __func__);
		return -1;
	}

	if (tegra_aes_configure(priv))
		return -1;

	/* Configure Secure Input */
	value = 1 << TEGRA_AES_SECURE_INPUT_ALG_SEL_SHIFT |
		priv->current_key_size << TEGRA_AES_SECURE_INPUT_KEY_LEN_SHIFT;

	if (mode & TEGRA_AES_MODE_UPDATE_IV)
		value |= TEGRA_AES_SECURE_INPUT_IV_FIELD;
	if (mode & TEGRA_AES_MODE_HASH)
		value |= TEGRA_AES_SECURE_INPUT_HASH_ENB_FIELD;
	if (mode & TEGRA_AES_MODE_CBC) {
		value |= ((mode & TEGRA_AES_MODE_ENCRYPT) ? 2 : 3) <<
			 TEGRA_AES_SECURE_XOR_POS_SHIFT;
		value |= ((mode & TEGRA_AES_MODE_ENCRYPT) ? 2 : 3) <<
			 TEGRA_AES_SECURE_VCTRAM_SEL_SHIFT;
		value |= ((mode & TEGRA_AES_MODE_ENCRYPT) ? 1 : 0) <<
			 TEGRA_AES_SECURE_CORE_SEL_SHIFT;
	} else {
		/* ECB */
		value |= ((mode & TEGRA_AES_MODE_ENCRYPT) ? 1 : 0) <<
			 TEGRA_AES_SECURE_CORE_SEL_SHIFT;
	}

	writel(value, priv->regs + TEGRA_AES_SECURE_INPUT_SELECT);

	/* Set destination address (doing in-place at IRAM) */
	writel((u32)priv->iram_addr, priv->regs + TEGRA_AES_SECURE_DEST_ADDR);

	/* Copy src data to IRAM */
	if (src != priv->iram_addr)
		memcpy(priv->iram_addr, src, nblocks * AES_BLOCK_LENGTH);

	/* Run ICMD commands */
	cmdq[0] = TEGRA_AES_CMD_DMASETUP << TEGRA_AES_CMDQ_OPCODE_SHIFT;
	cmdq[1] = (u32)priv->iram_addr;
	cmdq[2] = TEGRA_AES_CMD_BLKSTARTENGINE << TEGRA_AES_CMDQ_OPCODE_SHIFT | (nblocks - 1);
	cmdq[3] = TEGRA_AES_CMD_DMACOMPLETE << TEGRA_AES_CMDQ_OPCODE_SHIFT;

	for (int i = 0; i < ICMDQ_LENGTH; i++) {
		tegra_aes_wait_for_idle_dma(priv, (ICMDQ_LENGTH - 1) == i);
		writel(cmdq[i], priv->regs + TEGRA_AES_ICMDQUE_WR);
	}

	if (tegra_aes_wait_for_idle(priv))
		return -1;

	/* Put the result from IRAM to destination if not hashing */
	if (dst != priv->iram_addr && !(mode & TEGRA_AES_MODE_HASH))
		memcpy(dst, priv->iram_addr, nblocks * AES_BLOCK_LENGTH);

	return 0;
}

static int tegra_aes_process_blocks(struct udevice *dev, u8 *iv, u8 *src,
				    u8 *dst, u32 num_aes_blocks, u32 mode)
{
	struct tegra_aes_priv *priv = dev_get_priv(dev);

	log_debug("%s: 0x%p -> 0x%p blocks %d mode 0x%x\n",
		  __func__, src, dst, num_aes_blocks, mode);

	if (!num_aes_blocks) {
		log_warning("%s: called with 0 blocks!\n", __func__);
		return -1;
	}

	/* Load initial IV if CBC mode */
	if (mode & TEGRA_AES_MODE_CBC) {
		if (tegra_aes_call_engine(priv, iv, priv->iram_addr, 1, TEGRA_AES_MODE_CBC))
			return -1;

		/* Add update IV flag */
		mode |= TEGRA_AES_MODE_UPDATE_IV;
	}

	/* Process blocks by calling engine several times per dma buffer size */
	while (num_aes_blocks > 0) {
		u32 blocks = min(num_aes_blocks, (u32)TEGRA_AES_DMA_BUFFER_SIZE);

		if (tegra_aes_call_engine(priv, src, dst, blocks, mode))
			return -1;

		num_aes_blocks -= blocks;
		src += blocks * AES_BLOCK_LENGTH;
		dst += blocks * AES_BLOCK_LENGTH;
	}

	return 0;
}

static int tegra_aes_ops_available_key_slots(struct udevice *dev)
{
	return 4; /* 4 slots in Tegra20 and Tegra30 */
}

static int tegra_aes_ops_select_key_slot(struct udevice *dev, u32 key_size, u8 slot)
{
	struct tegra_aes_priv *priv = dev_get_priv(dev);

	if (slot == TEGRA_AES_SLOT_SBK && !priv->sbk_available) {
		log_warning("%s: SBK not available!\n", __func__);
		return -1;
	}

	return tegra_aes_select_key_slot(priv, key_size, slot);
}

static int tegra_aes_ops_set_key_for_key_slot(struct udevice *dev, u32 key_size,
					      u8 *key, u8 slot)
{
	struct tegra_aes_priv *priv = dev_get_priv(dev);
	const u8 SUBCMD_CRYPTO_TABLE_SEL = 0x3;
	const u8 SUBCMD_KEY_TABLE_SEL = 0x8;
	const u8 CMDQ_KEYTABLEADDR_SHIFT = 0;
	const u8 CMDQ_KEYTABLEID_SHIFT = 17;
	const u8 CMDQ_TABLESEL_SHIFT = 24;
	u32 value, addr;

	log_debug("%s: slot %d\n", __func__, slot);

	if (tegra_aes_configure(priv))
		return -1;

	if (key_size < (AES128_KEY_LENGTH * 8) ||
	    key_size > (TEGRA_AES_HW_MAX_KEY_LENGTH * 8))
		return -EINVAL;

	if (slot == TEGRA_AES_SLOT_SBK)
		log_debug("%s: SBK slot being set!\n", __func__);

	/* Clear and copy data to IRAM */
	memset(priv->iram_addr, 0, TEGRA_AES_HW_TABLE_LENGTH);
	memcpy(priv->iram_addr, key, key_size / 8);

	/* Mask the addr */
	addr = ((u32)priv->iram_addr) & TEGRA_AES_KEYTABLEADDR_FIELD;

	/* Command for engine to load AES key from IRAM */
	value = TEGRA_AES_CMD_SETTABLE << TEGRA_AES_CMDQ_OPCODE_SHIFT |
		SUBCMD_CRYPTO_TABLE_SEL << CMDQ_TABLESEL_SHIFT |
		(SUBCMD_KEY_TABLE_SEL | slot) << CMDQ_KEYTABLEID_SHIFT |
		addr << CMDQ_KEYTABLEADDR_SHIFT;
	writel(value, priv->regs + TEGRA_AES_ICMDQUE_WR);

	return tegra_aes_wait_for_idle(priv);
}

static int tegra_aes_ops_aes_ecb_encrypt(struct udevice *dev, u8 *src, u8 *dst,
					 u32 num_aes_blocks)
{
	return tegra_aes_process_blocks(dev, NULL, src, dst, num_aes_blocks,
					TEGRA_AES_MODE_ENCRYPT);
}

static int tegra_aes_ops_aes_ecb_decrypt(struct udevice *dev, u8 *src, u8 *dst,
					 u32 num_aes_blocks)
{
	return tegra_aes_process_blocks(dev, NULL, src, dst, num_aes_blocks, 0);
}

static int tegra_aes_ops_aes_cbc_encrypt(struct udevice *dev, u8 *iv, u8 *src,
					 u8 *dst, u32 num_aes_blocks)
{
	return tegra_aes_process_blocks(dev, iv, src, dst, num_aes_blocks,
					TEGRA_AES_MODE_CBC | TEGRA_AES_MODE_ENCRYPT);
}

static int tegra_aes_ops_aes_cbc_decrypt(struct udevice *dev, u8 *iv, u8 *src,
					 u8 *dst, u32 num_aes_blocks)
{
	return tegra_aes_process_blocks(dev, iv, src, dst, num_aes_blocks,
					TEGRA_AES_MODE_CBC);
}

static void tegra_aes_test_loaded_sbk(struct udevice *dev)
{
	struct tegra_aes_priv *priv = dev_get_priv(dev);
	enum fuse_operating_mode opmode = tegra_fuse_get_operation_mode();
	const u8 ZERO_KEY_CIPHERTEXT[AES_BLOCK_LENGTH] = {
		0x66, 0xe9, 0x4b, 0xd4, 0xef, 0x8a, 0x2c, 0x3b,
		0x88, 0x4c, 0xfa, 0x59, 0xca, 0x34, 0x2b, 0x2e
	};

	/* Encrypt a zero block, we use ECB so that we only care about SBK and not the IV */
	memset(priv->iram_addr, 0, AES_BLOCK_LENGTH);
	tegra_aes_select_key_slot(priv, 128, TEGRA_AES_SLOT_SBK);
	tegra_aes_call_engine(priv, priv->iram_addr, priv->iram_addr, 1, TEGRA_AES_MODE_ENCRYPT);

	/* Evaluate the result of engine operation */
	if (!memcmp(priv->iram_addr, AES_ZERO_BLOCK, AES_BLOCK_LENGTH)) {
		log_err("%s: engine is not operational! (opmode 0x%x)\n", __func__, opmode);
	} else if (!memcmp(priv->iram_addr, ZERO_KEY_CIPHERTEXT, AES_BLOCK_LENGTH)) {
		if (opmode == MODE_ODM_PRODUCTION_SECURE) {
			log_warning("%s: SBK is zero or is cleared from engine! (opmode 0x%x)\n",
				    __func__, opmode);
		} else {
			log_debug("%s - SBK is zero and available! (opmode 0x%x)\n",
				  __func__, opmode);
			priv->sbk_available = true;
		}
	} else {
		if (opmode == MODE_ODM_PRODUCTION_SECURE) {
			log_debug("%s: SBK is available! (opmode 0x%x)\n", __func__, opmode);
			priv->sbk_available = true;
		} else {
			log_warning("%s: SBK is not zero and should be! (opmode 0x%x)\n",
				    __func__, opmode);
		}
	}
}

static int tegra_aes_hw_init(struct udevice *dev)
{
	struct tegra_aes_priv *priv = dev_get_priv(dev);
	u32 value;
	int ret;

	if (priv->clk_parent) {
		ret = reset_assert(&priv->reset_ctl_vde);
		if (ret) {
			log_debug("%s: VDE reset assert failed: %d\n", __func__, ret);
			return ret;
		}
	}

	ret = reset_assert(&priv->reset_ctl);
	if (ret) {
		log_debug("%s: BSE reset assert failed: %d\n", __func__, ret);
		return ret;
	}

	if (priv->clk_parent) {
		ret = clk_enable(priv->clk_parent);
		if (ret) {
			log_err("%s: VDE clock enable failed: %d\n", __func__, ret);
			return ret;
		}

		ret = clk_set_rate(priv->clk_parent, 50 * 1000000);
		if (IS_ERR_VALUE(ret)) {
			log_err("%s: VDE clock set rate failed: %d\n", __func__, ret);
			return ret;
		}
	}

	ret = clk_enable(priv->clk);
	if (ret) {
		log_err("%s: BSE clock enable failed: %d\n", __func__, ret);
		return ret;
	}

	if (priv->clk_parent) {
		ret = reset_deassert(&priv->reset_ctl_vde);
		if (ret) {
			log_err("%s: VDE reset deassert failed: %d\n", __func__, ret);
			return ret;
		}
	}

	ret = reset_deassert(&priv->reset_ctl);
	if (ret) {
		log_err("%s: BSE reset deassert failed: %d\n", __func__, ret);
		return ret;
	}

	/* Enable key schedule generation in hardware */
	value = readl(priv->regs + TEGRA_AES_SECURE_CONFIG_EXT);
	value &= ~TEGRA_AES_SECURE_KEY_SCH_DIS_FIELD;
	writel(value, priv->regs + TEGRA_AES_SECURE_CONFIG_EXT);

	/* Check if SBK is loaded in SBK slot or was erased */
	priv->sbk_available = false;
	tegra_aes_test_loaded_sbk(dev);

	return 0;
}

static int tegra_aes_probe(struct udevice *dev)
{
	struct tegra_aes_priv *priv = dev_get_priv(dev);
	fdt_size_t iram_size = 0;
	u32 value;
	int ret;

	priv->current_key_size = AES128_KEY_LENGTH;

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs) {
		log_err("%s: Cannot find aes reg address, binding failed\n", __func__);
		return -EINVAL;
	}

	priv->iram_addr = devfdt_get_addr_size_name_ptr(dev, "iram-buffer", &iram_size);
	if (!priv->iram_addr) {
		log_debug("%s: Cannot find iram buffer address, binding failed\n", __func__);
		return -EINVAL;
	}

	if (iram_size < TEGRA_AES_DMA_BUFFER_SIZE * AES_BLOCK_LENGTH) {
		log_debug("%s: Unsupported iram buffer size: 0x%x required: 0x%x\n",
			  __func__, iram_size, TEGRA_AES_DMA_BUFFER_SIZE);
		return -EINVAL;
	}

	/* Make sure the IRAM address is kept block aligned and accessible for slot loading */
	value = (uint32_t)priv->iram_addr;
	if ((value & 0xFFF0000F) != IRAM_BASE || value > TEGRA_AES_IRAM_MAX_ADDR) {
		log_debug("%s: iram buffer must be located inside iram,", __func__);
		log_debug("AES block aligned and not above 0x%08x, current addr %p\n",
			  (u32)TEGRA_AES_IRAM_MAX_ADDR, priv->iram_addr);
		return -EINVAL;
	}

	ret = reset_get_by_name(dev, NULL, &priv->reset_ctl);
	if (ret) {
		log_debug("%s: failed to get BSE reset: %d\n", __func__, ret);
		return ret;
	}

	priv->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(priv->clk)) {
		log_err("%s: failed to get BSE clock: %d\n", __func__, ret);
		return ret;
	}

	/* VDE clock and reset required by BSEV */
	ret = reset_get_by_name(dev, "vde", &priv->reset_ctl_vde);
	if (ret)
		log_debug("%s: failed to get VDE reset: %d\n", __func__, ret);

	priv->clk_parent = devm_clk_get(dev, "vde");
	if (IS_ERR(priv->clk_parent))
		log_debug("%s: failed to get BSE clock: %d\n", __func__, ret);

	return tegra_aes_hw_init(dev);
}

static const struct aes_ops tegra_aes_ops = {
	.available_key_slots = tegra_aes_ops_available_key_slots,
	.select_key_slot = tegra_aes_ops_select_key_slot,
	.set_key_for_key_slot = tegra_aes_ops_set_key_for_key_slot,
	.aes_ecb_encrypt = tegra_aes_ops_aes_ecb_encrypt,
	.aes_ecb_decrypt = tegra_aes_ops_aes_ecb_decrypt,
	.aes_cbc_encrypt = tegra_aes_ops_aes_cbc_encrypt,
	.aes_cbc_decrypt = tegra_aes_ops_aes_cbc_decrypt,
};

static const struct udevice_id tegra_aes_ids[] = {
	{ .compatible = "nvidia,tegra20-bsea" },
	{ .compatible = "nvidia,tegra20-bsev" },
	{ .compatible = "nvidia,tegra30-bsea" },
	{ .compatible = "nvidia,tegra30-bsev" },
	{ }
};

U_BOOT_DRIVER(tegra_aes) = {
	.name = "tegra_aes",
	.id = UCLASS_AES,
	.of_match = tegra_aes_ids,
	.probe = tegra_aes_probe,
	.ops = &tegra_aes_ops,
	.priv_auto = sizeof(struct tegra_aes_priv),
};
