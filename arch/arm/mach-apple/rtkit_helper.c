// SPDX-License-Identifier: GPL-2.0+ OR MIT
/*
 * Copyright The Asahi Linux Contributors
 */

#include <dm.h>
#include <mailbox.h>
#include <mapmem.h>
#include <reset.h>

#include <asm/io.h>
#include <asm/arch/rtkit.h>
#include <linux/iopoll.h>

/* ASC registers */
#define REG_CPU_CTRL		0x0044
#define  REG_CPU_CTRL_RUN	BIT(4)

#define APPLE_RTKIT_EP_OSLOG 8

struct rtkit_helper_priv {
	void *asc;		/* ASC registers */
	struct mbox_chan chan;
	struct apple_rtkit *rtk;
	bool sram_stolen;
};

static int shmem_setup(void *cookie, struct apple_rtkit_buffer *buf) {
	struct udevice *dev = cookie;
	struct rtkit_helper_priv *priv = dev_get_priv(dev);

	if (!buf->is_mapped) {
		/*
		 * Special case: The OSLog buffer on MTP persists on Linux handoff.
		 * Steal some SRAM instead of putting this in DRAM, so we don't
		 * have to hand off DART/DAPF mappings.
		 */
		if (buf->endpoint == APPLE_RTKIT_EP_OSLOG) {
			if (priv->sram_stolen) {
				printf("%s: Tried to map more than one OSLog buffer out of SRAM\n",
				       __func__);
			} else {
				fdt_size_t size;
				fdt_addr_t addr;

				addr = dev_read_addr_size_name(dev, "sram", &size);

				if (addr != FDT_ADDR_T_NONE) {
					buf->dva = ALIGN_DOWN(addr + size - buf->size, SZ_16K);
					priv->sram_stolen = true;

					return 0;
				} else {
					printf("%s: No SRAM, falling back to DRAM\n", __func__);
				}
			}
		}

		buf->buffer = memalign(SZ_16K, ALIGN(buf->size, SZ_16K));
		if (!buf->buffer)
			return -ENOMEM;

		buf->dva = (u64)buf->buffer;
	}
	return 0;
}

static void shmem_destroy(void *cookie, struct apple_rtkit_buffer *buf) {
	if (buf->buffer)
		free(buf->buffer);
}

static int rtkit_helper_probe(struct udevice *dev)
{
	struct rtkit_helper_priv *priv = dev_get_priv(dev);
	u32 ctrl;
	int ret;

	priv->asc = dev_read_addr_ptr(dev);
	if (!priv->asc)
		return -EINVAL;

	ret = mbox_get_by_index(dev, 0, &priv->chan);
	if (ret < 0)
		return ret;

	ctrl = readl(priv->asc + REG_CPU_CTRL);
	writel(ctrl | REG_CPU_CTRL_RUN, priv->asc + REG_CPU_CTRL);

	priv->rtk = apple_rtkit_init(&priv->chan, dev, shmem_setup, shmem_destroy);
	if (!priv->rtk)
		return -ENOMEM;

	ret = apple_rtkit_boot(priv->rtk);
	if (ret < 0) {
		printf("%s: Helper apple_rtkit_boot returned: %d\n", __func__, ret);
		return ret;
	}

	ret = apple_rtkit_set_ap_power(priv->rtk, APPLE_RTKIT_PWR_STATE_ON);
	if (ret < 0) {
		printf("%s: Helper apple_rtkit_set_ap_power returned: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int rtkit_helper_remove(struct udevice *dev)
{
	struct rtkit_helper_priv *priv = dev_get_priv(dev);
	u32 ctrl;

	apple_rtkit_shutdown(priv->rtk, APPLE_RTKIT_PWR_STATE_QUIESCED);

	ctrl = readl(priv->asc + REG_CPU_CTRL);
	writel(ctrl & ~REG_CPU_CTRL_RUN, priv->asc + REG_CPU_CTRL);

	apple_rtkit_free(priv->rtk);
	priv->rtk = NULL;

	return 0;
}

int apple_rtkit_helper_poll(struct udevice *dev, ulong timeout)
{
	struct rtkit_helper_priv *priv = dev_get_priv(dev);

	return apple_rtkit_poll(priv->rtk, timeout);
}

static const struct udevice_id rtkit_helper_ids[] = {
	{ .compatible = "apple,rtk-helper-asc4" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(rtkit_helper) = {
	.name = "rtkit_helper",
	.id = UCLASS_MISC,
	.of_match = rtkit_helper_ids,
	.priv_auto = sizeof(struct rtkit_helper_priv),
	.probe = rtkit_helper_probe,
	.remove = rtkit_helper_remove,
	.flags = DM_FLAG_OS_PREPARE,
};
