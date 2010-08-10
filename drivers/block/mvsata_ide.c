/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.aribaud@free.fr>
 *
 * Written-by: Albert ARIBAUD <albert.aribaud@free.fr>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <asm/io.h>

#if defined(CONFIG_ORION5X)
#include <asm/arch/orion5x.h>
#elif defined(CONFIG_KIRKWOOD)
#include <asm/arch/kirkwood.h>
#endif

/* SATA port registers */
struct mvsata_port_registers {
	u32 reserved1[192];
	/* offset 0x300 : ATA Interface registers */
	u32 sstatus;
	u32 serror;
	u32 scontrol;
	u32 ltmode;
	u32 phymode3;
	u32 phymode4;
	u32 reserved2[5];
	u32 phymode1;
	u32 phymode2;
	u32 bist_cr;
	u32 bist_dw1;
	u32 bist_dw2;
	u32 serrorintrmask;
};

/*
 * Sanity checks:
 * - to compile at all, we need CONFIG_SYS_ATA_BASE_ADDR.
 * - for ide_preinit to make sense, we need at least one of
 *   CONFIG_SYS_ATA_IDE0_OFFSET or CONFIG_SYS_ATA_IDE0_OFFSET;
 * - for inde_preinit to be called, we need CONFIG_IDE_PREINIT.
 * Fail with an explanation message if these conditions are not met.
 * This is particularly important for CONFIG_IDE_PREINIT, because
 * its lack would not cause a build error.
 */

#if !defined(CONFIG_SYS_ATA_BASE_ADDR)
#error CONFIG_SYS_ATA_BASE_ADDR must be defined
#elif !defined(CONFIG_SYS_ATA_IDE0_OFFSET) \
   && !defined(CONFIG_SYS_ATA_IDE1_OFFSET)
#error CONFIG_SYS_ATA_IDE0_OFFSET or CONFIG_SYS_ATA_IDE1_OFFSET \
   must be defined
#elif !defined(CONFIG_IDE_PREINIT)
#error CONFIG_IDE_PREINIT must be defined
#endif

/*
 * Masks and values for SControl DETection and Interface Power Management,
 * and for SStatus DETection.
 */

#define MVSATA_SCONTROL_DET_MASK		0x0000000F
#define MVSATA_SCONTROL_DET_NONE		0x00000000
#define MVSATA_SCONTROL_DET_INIT		0x00000001
#define MVSATA_SCONTROL_IPM_MASK		0x00000F00
#define MVSATA_SCONTROL_IPM_NO_LP_ALLOWED	0x00000300
#define MVSATA_SCONTROL_MASK \
	(MVSATA_SCONTROL_DET_MASK|MVSATA_SCONTROL_IPM_MASK)
#define MVSATA_PORT_INIT \
	(MVSATA_SCONTROL_DET_INIT|MVSATA_SCONTROL_IPM_NO_LP_ALLOWED)
#define MVSATA_PORT_USE \
	(MVSATA_SCONTROL_DET_NONE|MVSATA_SCONTROL_IPM_NO_LP_ALLOWED)
#define MVSATA_SSTATUS_DET_MASK			0x0000000F
#define MVSATA_SSTATUS_DET_DEVCOMM		0x00000003

/*
 * Initialize one MVSATAHC port: set SControl's IPM to "always active"
 * and DET to "reset", then wait for SStatus's DET to become "device and
 * comm ok" (or time out after 50 us if no device), then set SControl's
 * DET back to "no action".
 */

static void mvsata_ide_initialize_port(struct mvsata_port_registers *port)
{
	u32 control;
	u32 status;
	u32 tout = 50; /* wait at most 50 us for SATA reset to complete */

	control = readl(&port->scontrol);
	control = (control & ~MVSATA_SCONTROL_MASK) | MVSATA_PORT_INIT;
	writel(control, &port->scontrol);
	while (--tout) {
		status = readl(&port->sstatus) & MVSATA_SSTATUS_DET_MASK;
		if (status == MVSATA_SSTATUS_DET_DEVCOMM)
			break;
		udelay(1);
	}
	control = (control & ~MVSATA_SCONTROL_MASK) | MVSATA_PORT_USE;
	writel(control, &port->scontrol);
}

/*
 * ide_preinit() will be called by ide_init in cmd_ide.c and will
 * reset the MVSTATHC ports needed by the board.
 */

int ide_preinit(void)
{
	/* Enable ATA port 0 (could be SATA port 0 or 1) if declared */
#if defined(CONFIG_SYS_ATA_IDE0_OFFSET)
	mvsata_ide_initialize_port(
		(struct mvsata_port_registers *)
		(CONFIG_SYS_ATA_BASE_ADDR + CONFIG_SYS_ATA_IDE0_OFFSET));
#endif
	/* Enable ATA port 1 (could be SATA port 0 or 1) if declared */
#if defined(CONFIG_SYS_ATA_IDE1_OFFSET)
	mvsata_ide_initialize_port(
		(struct mvsata_port_registers *)
		(CONFIG_SYS_ATA_BASE_ADDR + CONFIG_SYS_ATA_IDE1_OFFSET));
#endif
	/* return 0 as we always succeed */
	return 0;
}
