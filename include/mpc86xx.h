/*
 * Copyright 2006 Freescale Semiconductor.
 * Jeffrey Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
 */

#ifndef	__MPC86xx_H__
#define __MPC86xx_H__

#define EXC_OFF_SYS_RESET	0x0100	/* System reset	offset */

/*
 * l2cr values.  Look in config_<BOARD>.h for the actual setup
 */
#define l2cr		 1017

#define L2CR_L2E         0x80000000 /* bit 0 - enable */
#define L2CR_L2PE        0x40000000 /* bit 1 - data parity */
#define L2CR_L2I         0x00200000 /* bit 10 - global invalidate bit */
#define L2CR_L2CTL       0x00100000 /* bit 11 - l2 ram control */
#define L2CR_L2DO        0x00010000 /* bit 15 - data-only mode */
#define L2CR_REP         0x00001000 /* bit 19 - l2 replacement alg */
#define L2CR_HWF         0x00000800 /* bit 20 - hardware flush */
#define L2CR_L2IP        0x00000001 /* global invalidate in progress */

/*
 * BAT settings.  Look in config_<BOARD>.h for the actual setup
 */

#define BATU_BL_128K            0x00000000
#define BATU_BL_256K            0x00000004
#define BATU_BL_512K            0x0000000c
#define BATU_BL_1M              0x0000001c
#define BATU_BL_2M              0x0000003c
#define BATU_BL_4M              0x0000007c
#define BATU_BL_8M              0x000000fc
#define BATU_BL_16M             0x000001fc
#define BATU_BL_32M             0x000003fc
#define BATU_BL_64M             0x000007fc
#define BATU_BL_128M            0x00000ffc
#define BATU_BL_256M            0x00001ffc
#define BATU_BL_512M            0x00003ffc
#define BATU_BL_1G              0x00007ffc
#define BATU_BL_2G              0x0000fffc
#define BATU_BL_4G              0x0001fffc

#define BATU_VS                 0x00000002
#define BATU_VP                 0x00000001
#define BATU_INVALID            0x00000000

#define BATL_WRITETHROUGH       0x00000040
#define BATL_CACHEINHIBIT       0x00000020
#define BATL_MEMCOHERENCE	0x00000010
#define BATL_GUARDEDSTORAGE     0x00000008
#define BATL_NO_ACCESS		0x00000000

#define BATL_PP_MSK		0x00000003
#define BATL_PP_00		0x00000000 /* No access */
#define BATL_PP_01		0x00000001 /* Read-only */
#define BATL_PP_10		0x00000002 /* Read-write */
#define BATL_PP_11		0x00000003

#define BATL_PP_NO_ACCESS	BATL_PP_00
#define BATL_PP_RO		BATL_PP_01
#define BATL_PP_RW		BATL_PP_10

#define HID0_XBSEN              0x00000100
#define HID0_HIGH_BAT_EN        0x00800000
#define HID0_XAEN               0x00020000

#ifndef __ASSEMBLY__

typedef struct {
	unsigned long freqProcessor;
	unsigned long freqSystemBus;
} MPC86xx_SYS_INFO;

#define l1icache_enable	icache_enable

void l2cache_enable(void);
void l1dcache_enable(void);

static __inline__ unsigned long get_hid0 (void)
{
	unsigned long hid0;
	asm volatile("mfspr %0, 1008" : "=r" (hid0) :);
	return hid0;
}

static __inline__ unsigned long get_hid1 (void)
{
	unsigned long hid1;
	asm volatile("mfspr %0, 1009" : "=r" (hid1) :);
	return hid1;
}

static __inline__ void set_hid0 (unsigned long hid0)
{
	asm volatile("mtspr 1008, %0" : : "r" (hid0));
}

static __inline__ void set_hid1 (unsigned long hid1)
{
	asm volatile("mtspr 1009, %0" : : "r" (hid1));
}


static __inline__ unsigned long get_l2cr (void)
{
   unsigned long l2cr_val;
   asm volatile("mfspr %0, 1017" : "=r" (l2cr_val) :);
   return l2cr_val;
}

#endif  /* _ASMLANGUAGE */
#endif	/* __MPC86xx_H__ */
