// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2026 Renesas Electronics Corp.
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <errno.h>
#include <hang.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>
#include <malloc.h>
#include <remoteproc.h>
#include <reset.h>

/* R-Car X5H contains 1 SCP core, 6 lockstep Cortex-R52 and 32 Cortex-A720AE cores. */
#define RCAR5_SCP_CORES			1
#define RCAR5_CR52_CORES		12
#define RCAR5_CA720_CORES		32

#define SCP_BASE			0xc1340000
#define SCP_CFGVECTABLE			(SCP_BASE + 0x0)
#define SCP_CFGNSSTCALIB		(SCP_BASE + 0x10)
#define SCP_CFGNSSTCALIB_13_3MHZ	0x010040f0
#define SCP_CPUWAIT			(SCP_BASE + 0x30)
#define SCP_CPUWAIT_WAIT		BIT(0)
#define SCP_STCM			0xc1000000

struct scp_scmi_shmem {
	u32	reserved0;
	u32	status;
	u64	reserved1;
	u32	flags;
	u32	length;
	u32	message_header;
	u8	payload[];
};

struct scp_scmi_pd_power_state_set_a2p {
	u32	flags;
	u32	domain_id;
	u32	power_state;
	u32	boot_addr;
};

/* The addresses in range 0x08000000..0x1fffffff are incremented by 0xa0000000 */
#define MFIS_COMMON_BASE	0xb89e1000
#define MFIS_SCP_WACNTR		(MFIS_COMMON_BASE + 0x904)

/* The addresses in range 0x08000000..0x1fffffff are incremented by 0xa0000000 */
#define MFIS_SCP_BASE		0xb8840000
#define MFIS_SCP_REICR8		(MFIS_SCP_BASE + 0x28004)
#define MFIS_SCP_CODEVALUE	0xacc00000
#define MFIS_SCP_REG_MASK	GENMASK(19, 0)

/**
 * mfis_scp_unlock() - Release MFIS-SCP lock
 * @lck: MFIS lock register
 */
static void mfis_scp_unlock(const u32 lck)
{
	writel(MFIS_SCP_CODEVALUE | (lck & MFIS_SCP_REG_MASK), MFIS_SCP_WACNTR);
}

#define SCP_SCMI_SHMEM_AREA09		0xc1060800
#define SCP_SCMI_STATUS_MASK		0x3
#define SCP_SCMI_STATUS_FREE		0x1

/**
 * scp_wait_fw_free() - Wait for SCP channel to be free for communication
 */
static void scp_wait_fw_free(void)
{
	struct scp_scmi_shmem *shmem = (struct scp_scmi_shmem *)SCP_SCMI_SHMEM_AREA09;

	while ((shmem->status & SCP_SCMI_STATUS_MASK) != SCP_SCMI_STATUS_FREE)
		mdelay(1);
}

/**
 * scp_send_interrupt() - Raise interrupt on SCP side
 */
static void scp_send_interrupt(void)
{
	mfis_scp_unlock(MFIS_SCP_REICR8);
	/* Send SCP IRQ */
	writel(1, MFIS_SCP_REICR8);
}

/* SCMI power domain IDs */
#define SCMI_PD_CORE_RT_CORE00			117
#define SCMI_PD_CORE_AP_CORE00			129

/*
 * FIXME: This is custom extension to the SCMI PD protocol:
 * - Protocol 0x11 (PD)
 * - Command 0x11 (POWER_STATE_SET_BOOTADDR - custom)
 * This must be removed when proper upstream SCP port exists
 */
#define SCMI_PD_POWER_STATE_SET_BOOTADDR	0x4411
#define SCMI_PD_POWER_STATE_SET_STATE_ON	0
#define SCMI_PD_POWER_STATE_SET_STATE_OFF	BIT(30)

/**
 * scp_cpu_core_id_to_domain() - Convert core ID to SCMI domain ID
 * @core: CPU core
 * @domain: SCMI domain
 */
static int scp_cpu_core_id_to_domain(const u32 core, u32 *domain)
{
	if (core >= RCAR5_SCP_CORES && core < RCAR5_SCP_CORES + RCAR5_CR52_CORES) {
		/* CR52 */
		*domain = core - RCAR5_SCP_CORES + SCMI_PD_CORE_RT_CORE00;
		return 0;
	}

	if (core >= RCAR5_SCP_CORES + RCAR5_CR52_CORES &&
	    core < RCAR5_SCP_CORES + RCAR5_CR52_CORES + RCAR5_CA720_CORES) {
		/* CA720 */
		*domain = core - RCAR5_SCP_CORES - RCAR5_CR52_CORES + SCMI_PD_CORE_AP_CORE00;
		return 0;
	}

	return -EINVAL;
}

/**
 * scp_cpu_core_set() - Start or stop CPU core by invoking SCP via SCMI
 * @core: CPU core to boot
 * @ep: Entry point
 * @state: Power state, 0: Power on, BIT(30): Power off
 */
static int scp_cpu_core_set(const u32 core, const u32 ep, const u32 state)
{
	struct scp_scmi_shmem *shmem = (struct scp_scmi_shmem *)SCP_SCMI_SHMEM_AREA09;
	struct scp_scmi_pd_power_state_set_a2p scmi_parameter = {
		.flags = 1,	/* Asynchronous power transition using APMU */
		.domain_id = 0,		/* Core ID */
		.power_state = state,	/* 0: Power on, BIT(30): Power off */
		.boot_addr = ep,
	};
	u32 status;
	int ret;

	ret = scp_cpu_core_id_to_domain(core, &scmi_parameter.domain_id);
	if (ret)
		return ret;

	/* Wait for SCP to be free, then set it busy */
	scp_wait_fw_free();
	shmem->status &= ~SCP_SCMI_STATUS_FREE;
	shmem->flags = 0;

	/* Set up the message and copy it to SHMEM */
	shmem->message_header = SCMI_PD_POWER_STATE_SET_BOOTADDR;
	memcpy(shmem->payload, &scmi_parameter, sizeof(scmi_parameter));
	shmem->length = sizeof(shmem->message_header) + sizeof(scmi_parameter);

	/* Send message to SCP and wait for completion */
	scp_send_interrupt();
	scp_wait_fw_free();

	/* Read back the result */
	status = readl((uintptr_t)shmem->payload);
	if (status) {
		printf("SCP POWER_STATE_SET domain %d failed, status=0x%x (%d)\n",
		       scmi_parameter.domain_id, status, status);
		return -EINVAL;
	}

	return 0;
}

/**
 * struct renesas_rsip_rproc_privdata - remote processor private data
 * @core_id:		CPU core id
 * @ep:			Entry point
 */
struct renesas_rsip_rproc_privdata {
	ulong		core_id;
	ulong		ep;
};

/**
 * renesas_rsip_rproc_load() - Load the remote processor
 * @dev:	corresponding remote processor device
 * @addr:	Address in memory where image is stored
 * @size:	Size in bytes of the image
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_rsip_rproc_load(struct udevice *dev, ulong addr, ulong size)
{
	struct renesas_rsip_rproc_privdata *priv = dev_get_priv(dev);

	priv->ep = addr;

	if (priv->core_id == 0)		/* SCP */
		memcpy((void *)SCP_STCM, (void *)addr, size);

	return 0;
}

/**
 * renesas_rsip_rproc_init() - Initialize the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_rsip_rproc_init(struct udevice *dev)
{
	struct renesas_rsip_rproc_privdata *priv = dev_get_priv(dev);
	struct reset_ctl *rst = dev_get_priv(dev->parent);
	u32 addr;

	if (priv->core_id != 0)	/* No init for non-SCP cores */
		return 0;

	/*
	 * Put SCP into reset, configure SCP entry point address and systick
	 * timer, release SCP from reset, and zero out SCP STCM regions.
	 */
	reset_assert(rst);

	writel(SCP_STCM, SCP_CFGVECTABLE);
	writel(SCP_CFGNSSTCALIB_13_3MHZ, SCP_CFGNSSTCALIB);
	setbits_le32(SCP_CPUWAIT, SCP_CPUWAIT_WAIT);

	reset_deassert(rst);

	/* Fill zero to SCP STCM regions 0 ... 27 */
	for (addr = SCP_STCM; addr < 0xc1061b00; addr += 8)
		writeq(0, addr);

	asm volatile("dsb sy");

	return 0;
}

/**
 * renesas_rsip_rproc_start() - Start the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_rsip_rproc_start(struct udevice *dev)
{
	struct renesas_rsip_rproc_privdata *priv = dev_get_priv(dev);

	if (priv->core_id == 0) {
		/* SCP */
		clrbits_le32(SCP_CPUWAIT, SCP_CPUWAIT_WAIT);
		return 0;
	} else {
		/* CR52 or CA720 */
		return scp_cpu_core_set(priv->core_id, priv->ep,
					SCMI_PD_POWER_STATE_SET_STATE_ON);
	}
}

/**
 * renesas_rsip_rproc_stop() - Stop the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_rsip_rproc_stop(struct udevice *dev)
{
	struct renesas_rsip_rproc_privdata *priv = dev_get_priv(dev);

	if (priv->core_id == 0) {
		/* SCP */
		return renesas_rsip_rproc_init(dev);
	} else {
		/* CR52 or CA720 */
		return scp_cpu_core_set(priv->core_id, 0,
					SCMI_PD_POWER_STATE_SET_STATE_OFF);
	}
}

/**
 * renesas_rsip_rproc_reset() - Reset the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_rsip_rproc_reset(struct udevice *dev)
{
	renesas_rsip_rproc_stop(dev);
	renesas_rsip_rproc_start(dev);
	return 0;
}

/**
 * renesas_rsip_rproc_is_running() - Is the remote processor running
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if the remote processor is running, 1 otherwise
 */
static int renesas_rsip_rproc_is_running(struct udevice *dev)
{
	/* We assume the core is stopped. */
	return 1;
}

/**
 * renesas_rsip_rproc_device_to_virt() - Convert device address to virtual address
 * @dev:	corresponding remote processor device
 * @da:		device address
 * @size:	Size of the memory region @da is pointing to
 * @is_iomem:	optional pointer filled in to indicate if @da is iomapped memory
 *
 * Return: converted virtual address
 */
static void *renesas_rsip_rproc_device_to_virt(struct udevice *dev, ulong da,
					       ulong size, bool *is_iomem)
{
	return (void *)da;
}

static const struct dm_rproc_ops renesas_rsip_rproc_ops = {
	.init		= renesas_rsip_rproc_init,
	.load		= renesas_rsip_rproc_load,
	.start		= renesas_rsip_rproc_start,
	.stop		= renesas_rsip_rproc_stop,
	.reset		= renesas_rsip_rproc_reset,
	.is_running	= renesas_rsip_rproc_is_running,
	.device_to_virt	= renesas_rsip_rproc_device_to_virt,
};

/**
 * renesas_rsip_rproc_of_to_plat() - Convert OF data to platform data
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_rsip_rproc_of_to_plat(struct udevice *dev)
{
	struct renesas_rsip_rproc_privdata *priv = dev_get_priv(dev);

	priv->core_id = dev_get_driver_data(dev);

	return 0;
}

U_BOOT_DRIVER(renesas_rsip_core) = {
	.name		= "rcar-rsip-core",
	.id		= UCLASS_REMOTEPROC,
	.ops		= &renesas_rsip_rproc_ops,
	.of_to_plat	= renesas_rsip_rproc_of_to_plat,
	.priv_auto	= sizeof(struct renesas_rsip_rproc_privdata),
};

/**
 * renesas_rsip_rproc_probe() - Common rproc driver probe
 * @dev:	corresponding remote processor parent device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_rsip_rproc_probe(struct udevice *parent)
{
	struct reset_ctl *rst = dev_get_priv(parent);

	return reset_get_by_index(parent, 0, rst);
}

/**
 * renesas_rsip_rproc_bind() - Bind rproc driver to each core control
 * @dev:	corresponding remote processor parent device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_rsip_rproc_bind(struct udevice *parent)
{
	ofnode pnode = dev_ofnode(parent);
	struct udevice *cdev;
	struct driver *cdrv;
	char name[32];
	ulong i;
	int ret;

	cdrv = lists_driver_lookup_name("rcar-rsip-core");
	if (!cdrv)
		return -ENOENT;

	/* Singleton SCP core is core 0 */
	ret = device_bind_with_driver_data(parent, cdrv,
					   strdup("rcar-rsip-scp"),
					   0, pnode, &cdev);
	if (ret)
		return ret;

	/* Cores 1..13 are Cortex-R52 */
	for (i = 0; i < RCAR5_CR52_CORES; i++) {
		snprintf(name, sizeof(name), "rcar-rsip-cr.%ld", i);
		ret = device_bind_with_driver_data(parent, cdrv, strdup(name),
						   i + RCAR5_SCP_CORES, pnode,
						   &cdev);
		if (ret)
			return ret;
	}

	/* Cores 14..46 are Cortex-A720 */
	for (i = 0; i < RCAR5_CA720_CORES; i++) {
		snprintf(name, sizeof(name), "rcar-rsip-ca.%ld", i);
		ret = device_bind_with_driver_data(parent, cdrv, strdup(name),
						   i + RCAR5_SCP_CORES + RCAR5_CR52_CORES,
						   pnode, &cdev);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct udevice_id renesas_rsip_rproc_ids[] = {
	{ .compatible = "renesas,r8a78000-rproc" },
	{ }
};

U_BOOT_DRIVER(renesas_rsip_rproc) = {
	.name		= "rcar-rsip-rproc",
	.of_match	= renesas_rsip_rproc_ids,
	.id		= UCLASS_NOP,
	.bind		= renesas_rsip_rproc_bind,
	.probe		= renesas_rsip_rproc_probe,
	.priv_auto	= sizeof(struct reset_ctl),
};
