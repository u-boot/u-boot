/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <config.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_srio.h>

#define SRIO_PORT_ACCEPT_ALL 0x10000001
#define SRIO_IB_ATMU_AR 0x80f55000
#define SRIO_OB_ATMU_AR_MAINT 0x80077000
#define SRIO_OB_ATMU_AR_RW 0x80045000
#define SRIO_LCSBA1CSR_OFFSET 0x5c
#define SRIO_MAINT_WIN_SIZE 0x1000000 /* 16M */
#define SRIO_RW_WIN_SIZE 0x100000 /* 1M */
#define SRIO_LCSBA1CSR 0x60000000

#if defined(CONFIG_FSL_CORENET)
	#define _DEVDISR_SRIO1 FSL_CORENET_DEVDISR_SRIO1
	#define _DEVDISR_SRIO2 FSL_CORENET_DEVDISR_SRIO2
	#define _DEVDISR_RMU   FSL_CORENET_DEVDISR_RMU
	#define CONFIG_SYS_MPC8xxx_GUTS_ADDR CONFIG_SYS_MPC85xx_GUTS_ADDR
#elif defined(CONFIG_MPC85xx)
	#define _DEVDISR_SRIO1 MPC85xx_DEVDISR_SRIO
	#define _DEVDISR_SRIO2 MPC85xx_DEVDISR_SRIO
	#define _DEVDISR_RMU   MPC85xx_DEVDISR_RMSG
	#define CONFIG_SYS_MPC8xxx_GUTS_ADDR CONFIG_SYS_MPC85xx_GUTS_ADDR
#elif defined(CONFIG_MPC86xx)
	#define _DEVDISR_SRIO1 MPC86xx_DEVDISR_SRIO
	#define _DEVDISR_SRIO2 MPC86xx_DEVDISR_SRIO
	#define _DEVDISR_RMU   MPC86xx_DEVDISR_RMSG
	#define CONFIG_SYS_MPC8xxx_GUTS_ADDR \
		(&((immap_t *)CONFIG_SYS_IMMR)->im_gur)
#else
#error "No defines for DEVDISR_SRIO"
#endif

void srio_init(void)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC8xxx_GUTS_ADDR;
	int srio1_used = 0, srio2_used = 0;

	if (is_serdes_configured(SRIO1)) {
		set_next_law(CONFIG_SYS_SRIO1_MEM_PHYS,
				law_size_bits(CONFIG_SYS_SRIO1_MEM_SIZE),
				LAW_TRGT_IF_RIO_1);
		srio1_used = 1;
		printf("SRIO1: enabled\n");
	} else {
		printf("SRIO1: disabled\n");
	}

#ifdef CONFIG_SRIO2
	if (is_serdes_configured(SRIO2)) {
		set_next_law(CONFIG_SYS_SRIO2_MEM_PHYS,
				law_size_bits(CONFIG_SYS_SRIO2_MEM_SIZE),
				LAW_TRGT_IF_RIO_2);
		srio2_used = 1;
		printf("SRIO2: enabled\n");
	} else {
		printf("SRIO2: disabled\n");
	}
#endif

#ifdef CONFIG_FSL_CORENET
	/* On FSL_CORENET devices we can disable individual ports */
	if (!srio1_used)
		setbits_be32(&gur->devdisr, FSL_CORENET_DEVDISR_SRIO1);
	if (!srio2_used)
		setbits_be32(&gur->devdisr, FSL_CORENET_DEVDISR_SRIO2);
#endif

	/* neither port is used - disable everything */
	if (!srio1_used && !srio2_used) {
		setbits_be32(&gur->devdisr, _DEVDISR_SRIO1);
		setbits_be32(&gur->devdisr, _DEVDISR_SRIO2);
		setbits_be32(&gur->devdisr, _DEVDISR_RMU);
	}
}

#ifdef CONFIG_SRIOBOOT_MASTER
void srio_boot_master(void)
{
	struct ccsr_rio *srio = (void *)CONFIG_SYS_FSL_SRIO_ADDR;

	/* set port accept-all */
	out_be32((void *)&srio->impl.port[CONFIG_SRIOBOOT_MASTER_PORT].ptaacr,
				SRIO_PORT_ACCEPT_ALL);

	debug("SRIOBOOT - MASTER: Master port [ %d ] for srio boot.\n",
			CONFIG_SRIOBOOT_MASTER_PORT);
	/* configure inbound window for slave's u-boot image */
	debug("SRIOBOOT - MASTER: Inbound window for slave's image; "
			"Local = 0x%llx, Srio = 0x%llx, Size = 0x%x\n",
			(u64)CONFIG_SRIOBOOT_SLAVE_IMAGE_LAW_PHYS1,
			(u64)CONFIG_SRIOBOOT_SLAVE_IMAGE_SRIO_PHYS1,
			CONFIG_SRIOBOOT_SLAVE_IMAGE_SIZE);
	out_be32((void *)&srio->atmu
			.port[CONFIG_SRIOBOOT_MASTER_PORT].inbw[0].riwtar,
			CONFIG_SRIOBOOT_SLAVE_IMAGE_LAW_PHYS1 >> 12);
	out_be32((void *)&srio->atmu
			.port[CONFIG_SRIOBOOT_MASTER_PORT].inbw[0].riwbar,
			CONFIG_SRIOBOOT_SLAVE_IMAGE_SRIO_PHYS1 >> 12);
	out_be32((void *)&srio->atmu
			.port[CONFIG_SRIOBOOT_MASTER_PORT].inbw[0].riwar,
			SRIO_IB_ATMU_AR
			| atmu_size_mask(CONFIG_SRIOBOOT_SLAVE_IMAGE_SIZE));

	/* configure inbound window for slave's u-boot image */
	debug("SRIOBOOT - MASTER: Inbound window for slave's image; "
			"Local = 0x%llx, Srio = 0x%llx, Size = 0x%x\n",
			(u64)CONFIG_SRIOBOOT_SLAVE_IMAGE_LAW_PHYS2,
			(u64)CONFIG_SRIOBOOT_SLAVE_IMAGE_SRIO_PHYS2,
			CONFIG_SRIOBOOT_SLAVE_IMAGE_SIZE);
	out_be32((void *)&srio->atmu
			.port[CONFIG_SRIOBOOT_MASTER_PORT].inbw[1].riwtar,
			CONFIG_SRIOBOOT_SLAVE_IMAGE_LAW_PHYS2 >> 12);
	out_be32((void *)&srio->atmu
			.port[CONFIG_SRIOBOOT_MASTER_PORT].inbw[1].riwbar,
			CONFIG_SRIOBOOT_SLAVE_IMAGE_SRIO_PHYS2 >> 12);
	out_be32((void *)&srio->atmu
			.port[CONFIG_SRIOBOOT_MASTER_PORT].inbw[1].riwar,
			SRIO_IB_ATMU_AR
			| atmu_size_mask(CONFIG_SRIOBOOT_SLAVE_IMAGE_SIZE));

	/* configure inbound window for slave's ucode */
	debug("SRIOBOOT - MASTER: Inbound window for slave's ucode; "
			"Local = 0x%llx, Srio = 0x%llx, Size = 0x%x\n",
			(u64)CONFIG_SRIOBOOT_SLAVE_UCODE_LAW_PHYS,
			(u64)CONFIG_SRIOBOOT_SLAVE_UCODE_SRIO_PHYS,
			CONFIG_SRIOBOOT_SLAVE_UCODE_SIZE);
	out_be32((void *)&srio->atmu
			.port[CONFIG_SRIOBOOT_MASTER_PORT].inbw[2].riwtar,
			CONFIG_SRIOBOOT_SLAVE_UCODE_LAW_PHYS >> 12);
	out_be32((void *)&srio->atmu
			.port[CONFIG_SRIOBOOT_MASTER_PORT].inbw[2].riwbar,
			CONFIG_SRIOBOOT_SLAVE_UCODE_SRIO_PHYS >> 12);
	out_be32((void *)&srio->atmu
			.port[CONFIG_SRIOBOOT_MASTER_PORT].inbw[2].riwar,
			SRIO_IB_ATMU_AR
			| atmu_size_mask(CONFIG_SRIOBOOT_SLAVE_UCODE_SIZE));

	/* configure inbound window for slave's ENV */
	debug("SRIOBOOT - MASTER: Inbound window for slave's ENV; "
			"Local = 0x%llx, Siro = 0x%llx, Size = 0x%x\n",
			CONFIG_SRIOBOOT_SLAVE_ENV_LAW_PHYS,
			CONFIG_SRIOBOOT_SLAVE_ENV_SRIO_PHYS,
			CONFIG_SRIOBOOT_SLAVE_ENV_SIZE);
	out_be32((void *)&srio->atmu
			.port[CONFIG_SRIOBOOT_MASTER_PORT].inbw[3].riwtar,
			CONFIG_SRIOBOOT_SLAVE_ENV_LAW_PHYS >> 12);
	out_be32((void *)&srio->atmu
			.port[CONFIG_SRIOBOOT_MASTER_PORT].inbw[3].riwbar,
			CONFIG_SRIOBOOT_SLAVE_ENV_SRIO_PHYS >> 12);
	out_be32((void *)&srio->atmu
			.port[CONFIG_SRIOBOOT_MASTER_PORT].inbw[3].riwar,
			SRIO_IB_ATMU_AR
			| atmu_size_mask(CONFIG_SRIOBOOT_SLAVE_ENV_SIZE));
}

#ifdef CONFIG_SRIOBOOT_SLAVE_HOLDOFF
void srio_boot_master_release_slave(void)
{
	struct ccsr_rio *srio = (void *)CONFIG_SYS_FSL_SRIO_ADDR;
	u32 escsr;
	debug("SRIOBOOT - MASTER: "
			"Check the port status and release slave core ...\n");

	escsr = in_be32((void *)&srio->lp_serial
			.port[CONFIG_SRIOBOOT_MASTER_PORT].pescsr);
	if (escsr & 0x2) {
		if (escsr & 0x10100) {
			debug("SRIOBOOT - MASTER: Port [ %d ] is error.\n",
					CONFIG_SRIOBOOT_MASTER_PORT);
		} else {
			debug("SRIOBOOT - MASTER: "
					"Port [ %d ] is ready, now release slave's core ...\n",
					CONFIG_SRIOBOOT_MASTER_PORT);
			/*
			 * configure outbound window
			 * with maintenance attribute to set slave's LCSBA1CSR
			 */
			out_be32((void *)&srio->atmu
				.port[CONFIG_SRIOBOOT_MASTER_PORT]
				.outbw[1].rowtar, 0);
			out_be32((void *)&srio->atmu
				.port[CONFIG_SRIOBOOT_MASTER_PORT]
				.outbw[1].rowtear, 0);
			if (CONFIG_SRIOBOOT_MASTER_PORT)
				out_be32((void *)&srio->atmu
					.port[CONFIG_SRIOBOOT_MASTER_PORT]
					.outbw[1].rowbar,
					CONFIG_SYS_SRIO2_MEM_PHYS >> 12);
			else
				out_be32((void *)&srio->atmu
					.port[CONFIG_SRIOBOOT_MASTER_PORT]
					.outbw[1].rowbar,
					CONFIG_SYS_SRIO1_MEM_PHYS >> 12);
			out_be32((void *)&srio->atmu
					.port[CONFIG_SRIOBOOT_MASTER_PORT]
					.outbw[1].rowar,
					SRIO_OB_ATMU_AR_MAINT
					| atmu_size_mask(SRIO_MAINT_WIN_SIZE));

			/*
			 * configure outbound window
			 * with R/W attribute to set slave's BRR
			 */
			out_be32((void *)&srio->atmu
				.port[CONFIG_SRIOBOOT_MASTER_PORT]
				.outbw[2].rowtar,
				SRIO_LCSBA1CSR >> 9);
			out_be32((void *)&srio->atmu
				.port[CONFIG_SRIOBOOT_MASTER_PORT]
				.outbw[2].rowtear, 0);
			if (CONFIG_SRIOBOOT_MASTER_PORT)
				out_be32((void *)&srio->atmu
					.port[CONFIG_SRIOBOOT_MASTER_PORT]
					.outbw[2].rowbar,
					(CONFIG_SYS_SRIO2_MEM_PHYS
					+ SRIO_MAINT_WIN_SIZE) >> 12);
			else
				out_be32((void *)&srio->atmu
					.port[CONFIG_SRIOBOOT_MASTER_PORT]
					.outbw[2].rowbar,
					(CONFIG_SYS_SRIO1_MEM_PHYS
					+ SRIO_MAINT_WIN_SIZE) >> 12);
			out_be32((void *)&srio->atmu
				.port[CONFIG_SRIOBOOT_MASTER_PORT]
				.outbw[2].rowar,
				SRIO_OB_ATMU_AR_RW
				| atmu_size_mask(SRIO_RW_WIN_SIZE));

			/*
			 * Set the LCSBA1CSR register in slave
			 * by the maint-outbound window
			 */
			if (CONFIG_SRIOBOOT_MASTER_PORT) {
				out_be32((void *)CONFIG_SYS_SRIO2_MEM_VIRT
					+ SRIO_LCSBA1CSR_OFFSET,
					SRIO_LCSBA1CSR);
				while (in_be32((void *)CONFIG_SYS_SRIO2_MEM_VIRT
					+ SRIO_LCSBA1CSR_OFFSET)
					!= SRIO_LCSBA1CSR)
					;
				/*
				 * And then set the BRR register
				 * to release slave core
				 */
				out_be32((void *)CONFIG_SYS_SRIO2_MEM_VIRT
					+ SRIO_MAINT_WIN_SIZE
					+ CONFIG_SRIOBOOT_SLAVE_BRR_OFFSET,
					CONFIG_SRIOBOOT_SLAVE_RELEASE_MASK);
			} else {
				out_be32((void *)CONFIG_SYS_SRIO1_MEM_VIRT
					+ SRIO_LCSBA1CSR_OFFSET,
					SRIO_LCSBA1CSR);
				while (in_be32((void *)CONFIG_SYS_SRIO1_MEM_VIRT
					+ SRIO_LCSBA1CSR_OFFSET)
					!= SRIO_LCSBA1CSR)
					;
				/*
				 * And then set the BRR register
				 * to release slave core
				 */
				out_be32((void *)CONFIG_SYS_SRIO1_MEM_VIRT
					+ SRIO_MAINT_WIN_SIZE
					+ CONFIG_SRIOBOOT_SLAVE_BRR_OFFSET,
					CONFIG_SRIOBOOT_SLAVE_RELEASE_MASK);
			}
			debug("SRIOBOOT - MASTER: "
					"Release slave successfully! Now the slave should start up!\n");
		}
	} else
		debug("SRIOBOOT - MASTER: Port [ %d ] is not ready.\n",
				CONFIG_SRIOBOOT_MASTER_PORT);
}
#endif
#endif
