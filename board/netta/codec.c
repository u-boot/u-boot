/*
 * CODEC
 */

#include <common.h>
#include <post.h>

#include "mpc8xx.h"

/***********************************************/

#define MAX_DUSLIC	4

#define NUM_CHANNELS	2
#define MAX_SLICS	(MAX_DUSLIC * NUM_CHANNELS)

/***********************************************/

#define SOP_READ_CH_0		0xC4  /* Read SOP Register for Channel A  */
#define SOP_READ_CH_1		0xCC  /* Read SOP Register for Channel B  */
#define SOP_WRITE_CH_0		0x44  /* Write SOP Register for Channel A */
#define SOP_WRITE_CH_1		0x4C  /* Write SOP Register for Channel B */

#define COP_READ_CH_0		0xC5
#define COP_READ_CH_1		0xCD
#define COP_WRITE_CH_0		0x45
#define COP_WRITE_CH_1		0x4D

#define POP_READ_CH_0		0xC6
#define POP_READ_CH_1		0xCE
#define POP_WRITE_CH_0		0x46
#define POP_WRITE_CH_1		0x4E

#define RST_CMD_DUSLIC_CHIP	0x40  /* OR 0x48 */
#define RST_CMD_DUSLIC_CH_A	0x41
#define RST_CMD_DUSLIC_CH_B	0x49

#define PCM_RESYNC_CMD_CH_A	0x42
#define PCM_RESYNC_CMD_CH_B	0x4A

#define ACTIVE_HOOK_LEV_4	0
#define ACTIVE_HOOK_LEV_12	1

#define SLIC_P_NORMAL		0x01

/************************************************/

#define CODSP_WR	0x00
#define CODSP_RD	0x80
#define CODSP_OP	0x40
#define CODSP_ADR(x)	(((unsigned char)(x) & 7) << 3)
#define CODSP_M(x)	((unsigned char)(x) & 7)
#define CODSP_CMD(x)	((unsigned char)(x) & 7)

/************************************************/

/* command indication ops */
#define CODSP_M_SLEEP_PWRDN	7
#define CODSP_M_PWRDN_HIZ	0
#define CODSP_M_ANY_ACT		2
#define CODSP_M_RING		5
#define CODSP_M_ACT_MET		6
#define CODSP_M_GND_START	4
#define CODSP_M_RING_PAUSE	1

/* single byte commands */
#define CODSP_CMD_SOFT_RESET	CODSP_CMD(0)
#define CODSP_CMD_RESET_CH	CODSP_CMD(1)
#define CODSP_CMD_RESYNC	CODSP_CMD(2)

/* two byte commands */
#define CODSP_CMD_SOP		CODSP_CMD(4)
#define CODSP_CMD_COP		CODSP_CMD(5)
#define CODSP_CMD_POP		CODSP_CMD(6)

/************************************************/

/* read as 4-bytes */
#define CODSP_INTREG_INT_CH	0x80000000
#define CODSP_INTREG_HOOK	0x40000000
#define CODSP_INTREG_GNDK	0x20000000
#define CODSP_INTREG_GNDP	0x10000000
#define CODSP_INTREG_ICON	0x08000000
#define CODSP_INTREG_VRTLIM	0x04000000
#define CODSP_INTREG_OTEMP	0x02000000
#define CODSP_INTREG_SYNC_FAIL	0x01000000
#define CODSP_INTREG_LM_THRES	0x00800000
#define CODSP_INTREG_READY	0x00400000
#define CODSP_INTREG_RSTAT	0x00200000
#define CODSP_INTREG_LM_OK	0x00100000
#define CODSP_INTREG_IO4_DU	0x00080000
#define CODSP_INTREG_IO3_DU	0x00040000
#define CODSP_INTREG_IO2_DU	0x00020000
#define CODSP_INTREG_IO1_DU	0x00010000
#define CODSP_INTREG_DTMF_OK	0x00008000
#define CODSP_INTREG_DTMF_KEY4	0x00004000
#define CODSP_INTREG_DTMF_KEY3	0x00002000
#define CODSP_INTREG_DTMF_KEY2	0x00001000
#define CODSP_INTREG_DTMF_KEY1	0x00000800
#define CODSP_INTREG_DTMF_KEY0	0x00000400
#define CODSP_INTREG_UTDR_OK	0x00000200
#define CODSP_INTREG_UTDX_OK	0x00000100
#define CODSP_INTREG_EDSP_FAIL	0x00000080
#define CODSP_INTREG_CIS_BOF	0x00000008
#define CODSP_INTREG_CIS_BUF	0x00000004
#define CODSP_INTREG_CIS_REQ	0x00000002
#define CODSP_INTREG_CIS_ACT	0x00000001

/************************************************/

/* ======== SOP REG ADDRESSES =======*/

#define REVISION_ADDR		0x00
#define PCMC1_ADDR		0x05
#define XCR_ADDR		0x06
#define INTREG1_ADDR		0x07
#define INTREG2_ADDR		0x08
#define INTREG3_ADDR		0x09
#define INTREG4_ADDR		0x0A
#define LMRES1_ADDR		0x0D
#define MASK_ADDR		0x11
#define IOCTL3_ADDR		0x14
#define BCR1_ADDR		0x15
#define BCR2_ADDR		0x16
#define BCR3_ADDR		0x17
#define BCR4_ADDR		0x18
#define BCR5_ADDR		0x19
#define DSCR_ADDR		0x1A
#define LMCR1_ADDR		0x1C
#define LMCR2_ADDR		0x1D
#define LMCR3_ADDR		0x1E
#define OFR1_ADDR		0x1F
#define PCMR1_ADDR		0x21
#define PCMX1_ADDR		0x25
#define TSTR3_ADDR		0x2B
#define TSTR4_ADDR		0x2C
#define TSTR5_ADDR		0x2D

/* ========= POP REG ADDRESSES ========*/

#define CIS_DAT_ADDR		0x00

#define LEC_LEN_ADDR		0x3A
#define LEC_POWR_ADDR		0x3B
#define LEC_DELP_ADDR		0x3C
#define LEC_DELQ_ADDR		0x3D
#define LEC_GAIN_XI_ADDR	0x3E
#define LEC_GAIN_RI_ADDR	0x3F
#define LEC_GAIN_XO_ADDR	0x40
#define LEC_RES_1_ADDR		0x41
#define LEC_RES_2_ADDR		0x42

#define NLP_POW_LPF_ADDR	0x30
#define NLP_POW_LPS_ADDR	0x31
#define NLP_BN_LEV_X_ADDR	0x32
#define NLP_BN_LEV_R_ADDR	0x33
#define NLP_BN_INC_ADDR		0x34
#define NLP_BN_DEC_ADDR		0x35
#define NLP_BN_MAX_ADDR		0x36
#define NLP_BN_ADJ_ADDR		0x37
#define NLP_RE_MIN_ERLL_ADDR	0x38
#define NLP_RE_EST_ERLL_ADDR	0x39
#define NLP_SD_LEV_X_ADDR	0x3A
#define NLP_SD_LEV_R_ADDR	0x3B
#define NLP_SD_LEV_BN_ADDR	0x3C
#define NLP_SD_LEV_RE_ADDR	0x3D
#define NLP_SD_OT_DT_ADDR	0x3E
#define NLP_ERL_LIN_LP_ADDR	0x3F
#define NLP_ERL_LEC_LP_ADDR	0x40
#define NLP_CT_LEV_RE_ADDR	0x41
#define NLP_CTRL_ADDR		0x42

#define UTD_CF_H_ADDR		0x4B
#define UTD_CF_L_ADDR		0x4C
#define UTD_BW_H_ADDR		0x4D
#define UTD_BW_L_ADDR		0x4E
#define UTD_NLEV_ADDR		0x4F
#define UTD_SLEV_H_ADDR		0x50
#define UTD_SLEV_L_ADDR		0x51
#define UTD_DELT_ADDR		0x52
#define UTD_RBRK_ADDR		0x53
#define UTD_RTIME_ADDR		0x54
#define UTD_EBRK_ADDR		0x55
#define UTD_ETIME_ADDR		0x56

#define DTMF_LEV_ADDR		0x30
#define DTMF_TWI_ADDR		0x31
#define DTMF_NCF_H_ADDR		0x32
#define DTMF_NCF_L_ADDR		0x33
#define DTMF_NBW_H_ADDR		0x34
#define DTMF_NBW_L_ADDR		0x35
#define DTMF_GAIN_ADDR		0x36
#define DTMF_RES1_ADDR		0x37
#define DTMF_RES2_ADDR		0x38
#define DTMF_RES3_ADDR		0x39

#define CIS_LEV_H_ADDR		0x43
#define CIS_LEV_L_ADDR		0x44
#define CIS_BRS_ADDR		0x45
#define CIS_SEIZ_H_ADDR		0x46
#define CIS_SEIZ_L_ADDR		0x47
#define CIS_MARK_H_ADDR		0x48
#define CIS_MARK_L_ADDR		0x49
#define CIS_LEC_MODE_ADDR	0x4A

/*=====================================*/

#define HOOK_LEV_ACT_START_ADDR 0x89
#define RO1_START_ADDR		0x70
#define RO2_START_ADDR		0x95
#define RO3_START_ADDR		0x96

#define TG1_FREQ_START_ADDR	0x38
#define TG1_GAIN_START_ADDR	0x39
#define TG1_BANDPASS_START_ADDR 0x3B
#define TG1_BANDPASS_END_ADDR	0x3D

#define TG2_FREQ_START_ADDR	0x40
#define TG2_GAIN_START_ADDR	0x41
#define TG2_BANDPASS_START_ADDR 0x43
#define TG2_BANDPASS_END_ADDR	0x45

/*====================================*/

#define PCM_HW_B		0x80
#define PCM_HW_A		0x00
#define PCM_TIME_SLOT_0		0x00   /*  Byte 0 of PCM Frame (by default is assigned to channel A ) */
#define PCM_TIME_SLOT_1		0x01   /*  Byte 1 of PCM Frame (by default is assigned to channel B ) */
#define PCM_TIME_SLOT_4		0x04   /*  Byte 4 of PCM Frame (Corresponds to B1 of the Second GCI ) */

#define	 RX_LEV_ADDR	0x28
#define	 TX_LEV_ADDR	0x30
#define	 Ik1_ADDR	0x83

#define	 AR_ROW		3 /* Is the row (AR Params) of the ac_Coeff array in SMS_CODEC_Defaults struct	*/
#define	 AX_ROW		6 /* Is the row (AX Params) of the ac_Coeff array in SMS_CODEC_Defaults struct	*/
#define	 DCF_ROW	0 /* Is the row (DCF Params) of the dc_Coeff array in SMS_CODEC_Defaults struct */

/* Mark the start byte of Duslic parameters that we use with configurator */
#define	 Ik1_START_BYTE		3
#define	 RX_LEV_START_BYTE	0
#define	 TX_LEV_START_BYTE	0

/************************************************/

#define INTREG4_CIS_ACT		(1 << 0)

#define BCR1_SLEEP		0x20
#define BCR1_REVPOL		0x10
#define BCR1_ACTR		0x08
#define BCR1_ACTL		0x04
#define BCR1_SLIC_MASK		0x03

#define BCR2_HARD_POL_REV	0x40
#define BCR2_TTX		0x20
#define BCR2_TTX_12K		0x10
#define BCR2_HIMAN		0x08
#define BCR2_PDOT		0x01

#define BCR3_PCMX_EN		(1 << 4)

#define BCR5_DTMF_EN		(1 << 0)
#define BCR5_DTMF_SRC		(1 << 1)
#define BCR5_LEC_EN		(1 << 2)
#define BCR5_LEC_OUT		(1 << 3)
#define BCR5_CIS_EN		(1 << 4)
#define BCR5_CIS_AUTO		(1 << 5)
#define BCR5_UTDX_EN		(1 << 6)
#define BCR5_UTDR_EN		(1 << 7)

#define DSCR_TG1_EN		(1 << 0)
#define DSCR_TG2_EN		(1 << 1)
#define DSCR_PTG		(1 << 2)
#define DSCR_COR8		(1 << 3)
#define DSCR_DG_KEY(x)		(((x) & 0x0F) << 4)

#define CIS_LEC_MODE_CIS_V23	(1 << 0)
#define CIS_LEC_MODE_CIS_FRM	(1 << 1)
#define CIS_LEC_MODE_NLP_EN	(1 << 2)
#define CIS_LEC_MODE_UTDR_SUM	(1 << 4)
#define CIS_LEC_MODE_UTDX_SUM	(1 << 5)
#define CIS_LEC_MODE_LEC_FREEZE (1 << 6)
#define CIS_LEC_MODE_LEC_ADAPT	(1 << 7)

#define TSTR4_COR_64		(1 << 5)

#define TSTR3_AC_DLB_8K		(1 << 2)
#define TSTR3_AC_DLB_32K	(1 << 3)
#define TSTR3_AC_DLB_4M		(1 << 5)


#define LMCR1_TEST_EN		(1 << 7)
#define LMCR1_LM_EN		(1 << 6)
#define LMCR1_LM_THM		(1 << 5)
#define LMCR1_LM_ONCE		(1 << 2)
#define LMCR1_LM_MASK		(1 << 1)

#define LMCR2_LM_RECT			(1 << 5)
#define LMCR2_LM_SEL_VDD		0x0D
#define LMCR2_LM_SEL_IO3		0x0A
#define LMCR2_LM_SEL_IO4		0x0B
#define LMCR2_LM_SEL_IO4_MINUS_IO3	0x0F

#define LMCR3_RTR_SEL		(1 << 6)

#define LMCR3_RNG_OFFSET_NONE	0x00
#define LMCR3_RNG_OFFSET_1	0x01
#define LMCR3_RNG_OFFSET_2	0x02
#define LMCR3_RNG_OFFSET_3	0x03

#define TSTR5_DC_HOLD		(1 << 3)

/************************************************/

#define TARGET_ONHOOK_BATH_x100		4600	/* 46.0 Volt */
#define TARGET_ONHOOK_BATL_x100		2500	/* 25.0 Volt */
#define TARGET_V_DIVIDER_RATIO_x100	21376L	/* (R1+R2)/R2 = 213.76 */
#define DIVIDER_RATIO_ACCURx100		(22 * 100)
#define V_AD_x10000			10834L	/* VAD = 1.0834 */
#define TARGET_VDDx100			330	/* VDD = 3.3 * 10 */
#define VDD_MAX_DIFFx100		20	/* VDD Accur = 0.2*100 */

#define RMS_MULTIPLIERx100		111	/* pi/(2xsqrt(2)) = 1.11*/
#define K_INTDC_RECT_ON			4	/* When Rectifier is ON this value is necessary(2^4) */
#define K_INTDC_RECT_OFF		2	/* 2^2 */
#define RNG_FREQ			25
#define SAMPLING_FREQ			(2000L)
#define N_SAMPLES			(SAMPLING_FREQ/RNG_FREQ)     /* for Ring Freq =25Hz (40ms Integration Period)[Sampling rate 2KHz -->1 Sample every 500us] */
#define HOOK_THRESH_RING_START_ADDR	0x8B
#define RING_PARAMS_START_ADDR		0x70

#define V_OUT_BATH_MAX_DIFFx100		300	/* 3.0 x100 */
#define V_OUT_BATL_MAX_DIFFx100		400	/* 4.0 x100 */
#define MAX_V_RING_MEANx100		50
#define TARGET_V_RING_RMSx100		2720
#define V_RMS_RING_MAX_DIFFx100		250

#define LM_OK_SRC_IRG_2			(1 << 4)

/************************************************/

#define PORTB		(((volatile immap_t *)CONFIG_SYS_IMMR)->im_cpm.cp_pbdat)
#define PORTC		(((volatile immap_t *)CONFIG_SYS_IMMR)->im_ioport.iop_pcdat)
#define PORTD		(((volatile immap_t *)CONFIG_SYS_IMMR)->im_ioport.iop_pddat)

#define _PORTD_SET(mask, state) \
	do { \
		if (state) \
			PORTD |= mask; \
		else \
			PORTD &= ~mask; \
	} while (0)

#define _PORTB_SET(mask, state) \
	do { \
		if (state) \
			PORTB |= mask; \
		else \
			PORTB &= ~mask; \
	} while (0)

#define _PORTB_TGL(mask) do { PORTB ^= mask; } while (0)
#define _PORTB_GET(mask) (!!(PORTB & mask))

#define _PORTC_GET(mask) (!!(PORTC & mask))

/* port B */
#define SPI_RXD		(1 << (31 - 28))
#define SPI_TXD		(1 << (31 - 29))
#define SPI_CLK		(1 << (31 - 30))

/* port C */
#define COM_HOOK1	(1 << (15 - 9))
#define COM_HOOK2	(1 << (15 - 10))

#ifndef CONFIG_NETTA_SWAPHOOK

#define COM_HOOK3	(1 << (15 - 11))
#define COM_HOOK4	(1 << (15 - 12))

#else

#define COM_HOOK3	(1 << (15 - 12))
#define COM_HOOK4	(1 << (15 - 11))

#endif

/* port D */
#define SPIENC1		(1 << (15 - 9))
#define SPIENC2		(1 << (15 - 10))
#define SPIENC3		(1 << (15 - 11))
#define SPIENC4		(1 << (15 - 14))

#define SPI_DELAY() udelay(1)

static inline unsigned int __SPI_Transfer(unsigned int tx)
{
	unsigned int rx;
	int b;

	rx = 0; b = 8;
	while (--b >= 0) {
		_PORTB_SET(SPI_TXD, tx & 0x80);
		tx <<= 1;
		_PORTB_TGL(SPI_CLK);
		SPI_DELAY();
		rx <<= 1;
		rx |= _PORTB_GET(SPI_RXD);
		_PORTB_TGL(SPI_CLK);
		SPI_DELAY();
	}

	return rx;
}

static const char *codsp_dtmf_map = "D1234567890*#ABC";

static const int spienc_mask_tab[4] = { SPIENC1, SPIENC2, SPIENC3, SPIENC4 };
static const int com_hook_mask_tab[4] = { COM_HOOK1, COM_HOOK2, COM_HOOK3, COM_HOOK4 };

static unsigned int codsp_send(int duslic_id, const unsigned char *cmd, int cmdlen, unsigned char *res, int reslen)
{
	unsigned int rx;
	int i;

	/* just some sanity checks */
	if (cmd == 0 || cmdlen < 0)
		return -1;

	_PORTD_SET(spienc_mask_tab[duslic_id], 0);

	/* first 2 bytes are without response */
	i = 2;
	while (i-- > 0 && cmdlen-- > 0)
		__SPI_Transfer(*cmd++);

	while (cmdlen-- > 0) {
		rx = __SPI_Transfer(*cmd++);
		if (res != 0 && reslen-- > 0)
			*res++ = (unsigned char)rx;
	}
	if (res != 0) {
		while (reslen-- > 0)
			*res++ = __SPI_Transfer(0xFF);
	}

	_PORTD_SET(spienc_mask_tab[duslic_id], 1);

	return 0;
}

/****************************************************************************/

void codsp_set_ciop_m(int duslic_id, int channel, unsigned char m)
{
	unsigned char cmd = CODSP_WR | CODSP_ADR(channel) | CODSP_M(m);
	codsp_send(duslic_id, &cmd, 1, 0, 0);
}

void codsp_reset_chip(int duslic_id)
{
	static const unsigned char cmd = CODSP_WR | CODSP_OP | CODSP_CMD_SOFT_RESET;
	codsp_send(duslic_id, &cmd, 1, 0, 0);
}

void codsp_reset_channel(int duslic_id, int channel)
{
	unsigned char cmd = CODSP_WR | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_RESET_CH;
	codsp_send(duslic_id, &cmd, 1, 0, 0);
}

void codsp_resync_channel(int duslic_id, int channel)
{
	unsigned char cmd = CODSP_WR | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_RESYNC;
	codsp_send(duslic_id, &cmd, 1, 0, 0);
}

/****************************************************************************/

void codsp_write_sop_char(int duslic_id, int channel, unsigned char regno, unsigned char val)
{
	unsigned char cmd[3];

	cmd[0] = CODSP_WR | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_SOP;
	cmd[1] = regno;
	cmd[2] = val;

	codsp_send(duslic_id, cmd, 3, 0, 0);
}

void codsp_write_sop_short(int duslic_id, int channel, unsigned char regno, unsigned short val)
{
	unsigned char cmd[4];

	cmd[0] = CODSP_WR | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_SOP;
	cmd[1] = regno;
	cmd[2] = (unsigned char)(val >> 8);
	cmd[3] = (unsigned char)val;

	codsp_send(duslic_id, cmd, 4, 0, 0);
}

void codsp_write_sop_int(int duslic_id, int channel, unsigned char regno, unsigned int val)
{
	unsigned char cmd[6];

	cmd[0] = CODSP_WR | CODSP_ADR(channel) | CODSP_CMD_SOP;
	cmd[1] = regno;
	cmd[2] = (unsigned char)(val >> 24);
	cmd[3] = (unsigned char)(val >> 16);
	cmd[4] = (unsigned char)(val >> 8);
	cmd[5] = (unsigned char)val;

	codsp_send(duslic_id, cmd, 6, 0, 0);
}

unsigned char codsp_read_sop_char(int duslic_id, int channel, unsigned char regno)
{
	unsigned char cmd[3];
	unsigned char res[2];

	cmd[0] = CODSP_RD | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_SOP;
	cmd[1] = regno;

	codsp_send(duslic_id, cmd, 2, res, 2);

	return res[1];
}

unsigned short codsp_read_sop_short(int duslic_id, int channel, unsigned char regno)
{
	unsigned char cmd[2];
	unsigned char res[3];

	cmd[0] = CODSP_RD | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_SOP;
	cmd[1] = regno;

	codsp_send(duslic_id, cmd, 2, res, 3);

	return ((unsigned short)res[1] << 8) | res[2];
}

unsigned int codsp_read_sop_int(int duslic_id, int channel, unsigned char regno)
{
	unsigned char cmd[2];
	unsigned char res[5];

	cmd[0] = CODSP_RD | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_SOP;
	cmd[1] = regno;

	codsp_send(duslic_id, cmd, 2, res, 5);

	return ((unsigned int)res[1] << 24) | ((unsigned int)res[2] << 16) | ((unsigned int)res[3] << 8) | res[4];
}

/****************************************************************************/

void codsp_write_cop_block(int duslic_id, int channel, unsigned char addr, const unsigned char *block)
{
	unsigned char cmd[10];

	cmd[0] = CODSP_WR | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_COP;
	cmd[1] = addr;
	memcpy(cmd + 2, block, 8);
	codsp_send(duslic_id, cmd, 10, 0, 0);
}

void codsp_write_cop_char(int duslic_id, int channel, unsigned char addr, unsigned char val)
{
	unsigned char cmd[3];

	cmd[0] = CODSP_WR | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_COP;
	cmd[1] = addr;
	cmd[2] = val;
	codsp_send(duslic_id, cmd, 3, 0, 0);
}

void codsp_write_cop_short(int duslic_id, int channel, unsigned char addr, unsigned short val)
{
	unsigned char cmd[4];

	cmd[0] = CODSP_WR | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_COP;
	cmd[1] = addr;
	cmd[2] = (unsigned char)(val >> 8);
	cmd[3] = (unsigned char)val;

	codsp_send(duslic_id, cmd, 4, 0, 0);
}

void codsp_read_cop_block(int duslic_id, int channel, unsigned char addr, unsigned char *block)
{
	unsigned char cmd[2];
	unsigned char res[9];

	cmd[0] = CODSP_RD | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_COP;
	cmd[1] = addr;
	codsp_send(duslic_id, cmd, 2, res, 9);
	memcpy(block, res + 1, 8);
}

unsigned char codsp_read_cop_char(int duslic_id, int channel, unsigned char addr)
{
	unsigned char cmd[2];
	unsigned char res[2];

	cmd[0] = CODSP_RD | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_COP;
	cmd[1] = addr;
	codsp_send(duslic_id, cmd, 2, res, 2);
	return res[1];
}

unsigned short codsp_read_cop_short(int duslic_id, int channel, unsigned char addr)
{
	unsigned char cmd[2];
	unsigned char res[3];

	cmd[0] = CODSP_RD | CODSP_OP | CODSP_ADR(channel) | CODSP_CMD_COP;
	cmd[1] = addr;

	codsp_send(duslic_id, cmd, 2, res, 3);

	return ((unsigned short)res[1] << 8) | res[2];
}

/****************************************************************************/

#define MAX_POP_BLOCK	50

void codsp_write_pop_block (int duslic_id, int channel, unsigned char addr,
			    const unsigned char *block, int len)
{
	unsigned char cmd[2 + MAX_POP_BLOCK];

	if (len > MAX_POP_BLOCK)	/* truncate */
		len = MAX_POP_BLOCK;

	cmd[0] = CODSP_WR | CODSP_OP | CODSP_ADR (channel) | CODSP_CMD_POP;
	cmd[1] = addr;
	memcpy (cmd + 2, block, len);
	codsp_send (duslic_id, cmd, 2 + len, 0, 0);
}

void codsp_write_pop_char (int duslic_id, int channel, unsigned char regno,
			   unsigned char val)
{
	unsigned char cmd[3];

	cmd[0] = CODSP_WR | CODSP_OP | CODSP_ADR (channel) | CODSP_CMD_POP;
	cmd[1] = regno;
	cmd[2] = val;

	codsp_send (duslic_id, cmd, 3, 0, 0);
}

void codsp_write_pop_short (int duslic_id, int channel, unsigned char regno,
			    unsigned short val)
{
	unsigned char cmd[4];

	cmd[0] = CODSP_WR | CODSP_OP | CODSP_ADR (channel) | CODSP_CMD_POP;
	cmd[1] = regno;
	cmd[2] = (unsigned char) (val >> 8);
	cmd[3] = (unsigned char) val;

	codsp_send (duslic_id, cmd, 4, 0, 0);
}

void codsp_write_pop_int (int duslic_id, int channel, unsigned char regno,
			  unsigned int val)
{
	unsigned char cmd[6];

	cmd[0] = CODSP_WR | CODSP_ADR (channel) | CODSP_CMD_POP;
	cmd[1] = regno;
	cmd[2] = (unsigned char) (val >> 24);
	cmd[3] = (unsigned char) (val >> 16);
	cmd[4] = (unsigned char) (val >> 8);
	cmd[5] = (unsigned char) val;

	codsp_send (duslic_id, cmd, 6, 0, 0);
}

unsigned char codsp_read_pop_char (int duslic_id, int channel,
				   unsigned char regno)
{
	unsigned char cmd[3];
	unsigned char res[2];

	cmd[0] = CODSP_RD | CODSP_OP | CODSP_ADR (channel) | CODSP_CMD_POP;
	cmd[1] = regno;

	codsp_send (duslic_id, cmd, 2, res, 2);

	return res[1];
}

unsigned short codsp_read_pop_short (int duslic_id, int channel,
				     unsigned char regno)
{
	unsigned char cmd[2];
	unsigned char res[3];

	cmd[0] = CODSP_RD | CODSP_OP | CODSP_ADR (channel) | CODSP_CMD_POP;
	cmd[1] = regno;

	codsp_send (duslic_id, cmd, 2, res, 3);

	return ((unsigned short) res[1] << 8) | res[2];
}

unsigned int codsp_read_pop_int (int duslic_id, int channel,
				 unsigned char regno)
{
	unsigned char cmd[2];
	unsigned char res[5];

	cmd[0] = CODSP_RD | CODSP_OP | CODSP_ADR (channel) | CODSP_CMD_POP;
	cmd[1] = regno;

	codsp_send (duslic_id, cmd, 2, res, 5);

	return (((unsigned int) res[1] << 24) |
		((unsigned int) res[2] << 16) |
		((unsigned int) res[3] <<  8) |
		res[4] );
}
/****************************************************************************/

struct _coeffs {
	unsigned char addr;
	unsigned char values[8];
};

struct _coeffs ac_coeffs[11] = {
	{ 0x60, {0xAD,0xDA,0xB5,0x9B,0xC7,0x2A,0x9D,0x00} }, /* 0x60 IM-Filter part 1 */
	{ 0x68, {0x10,0x00,0xA9,0x82,0x0D,0x77,0x0A,0x00} }, /* 0x68 IM-Filter part 2 */
	{ 0x18, {0x08,0xC0,0xD2,0xAB,0xA5,0xE2,0xAB,0x07} }, /* 0x18 FRR-Filter	      */
	{ 0x28, {0x44,0x93,0xF5,0x92,0x88,0x00,0x00,0x00} }, /* 0x28 AR-Filter	      */
	{ 0x48, {0x96,0x38,0x29,0x96,0xC9,0x2B,0x8B,0x00} }, /* 0x48 LPR-Filter	      */
	{ 0x20, {0x08,0xB0,0xDA,0x9D,0xA7,0xFA,0x93,0x06} }, /* 0x20 FRX-Filter	      */
	{ 0x30, {0xBA,0xAC,0x00,0x01,0x85,0x50,0xC0,0x1A} }, /* 0x30 AX-Filter	      */
	{ 0x50, {0x96,0x38,0x29,0xF5,0xFA,0x2B,0x8B,0x00} }, /* 0x50 LPX-Filter	      */
	{ 0x00, {0x00,0x08,0x08,0x81,0x00,0x80,0x00,0x08} }, /* 0x00 TH-Filter part 1 */
	{ 0x08, {0x81,0x00,0x80,0x00,0xD7,0x33,0xBA,0x01} }, /* 0x08 TH-Filter part 2 */
	{ 0x10, {0xB3,0x6C,0xDC,0xA3,0xA4,0xE5,0x88,0x00} }  /* 0x10 TH-Filter part 3 */
};

struct _coeffs ac_coeffs_0dB[11] = {
	{ 0x60, {0xAC,0x2A,0xB5,0x9A,0xB7,0x2A,0x9D,0x00} },
	{ 0x68, {0x10,0x00,0xA9,0x82,0x0D,0x83,0x0A,0x00} },
	{ 0x18, {0x08,0x20,0xD4,0xA4,0x65,0xEE,0x92,0x07} },
	{ 0x28, {0x2B,0xAB,0x36,0xA5,0x88,0x00,0x00,0x00} },
	{ 0x48, {0xAB,0xE9,0x4E,0x32,0xAB,0x25,0xA5,0x03} },
	{ 0x20, {0x08,0x20,0xDB,0x9C,0xA7,0xFA,0xB4,0x07} },
	{ 0x30, {0xF3,0x10,0x07,0x60,0x85,0x40,0xC0,0x1A} },
	{ 0x50, {0x96,0x38,0x29,0x97,0x39,0x19,0x8B,0x00} },
	{ 0x00, {0x00,0x08,0x08,0x81,0x00,0x80,0x00,0x08} },
	{ 0x08, {0x81,0x00,0x80,0x00,0x47,0x3C,0xD2,0x01} },
	{ 0x10, {0x62,0xDB,0x4A,0x87,0x73,0x28,0x88,0x00} }
};

struct _coeffs dc_coeffs[9] = {
	{ 0x80, {0x25,0x59,0x9C,0x23,0x24,0x23,0x32,0x1C} }, /* 0x80 DC-Parameter     */
	{ 0x70, {0x90,0x30,0x1B,0xC0,0x33,0x43,0xAC,0x02} }, /* 0x70 Ringing	      */
	{ 0x90, {0x3F,0xC3,0x2E,0x3A,0x80,0x90,0x00,0x09} }, /* 0x90 LP-Filters	      */
	{ 0x88, {0xAF,0x80,0x27,0x7B,0x01,0x4C,0x7B,0x02} }, /* 0x88 Hook Levels      */
	{ 0x78, {0x00,0xC0,0x6D,0x7A,0xB3,0x78,0x89,0x00} }, /* 0x78 Ramp Generator   */
	{ 0x58, {0xA5,0x44,0x34,0xDB,0x0E,0xA2,0x2A,0x00} }, /* 0x58 TTX	      */
	{ 0x38, {0x33,0x49,0x9A,0x65,0xBB,0x00,0x00,0x00} }, /* 0x38 TG1	      */
	{ 0x40, {0x33,0x49,0x9A,0x65,0xBB,0x00,0x00,0x00} }, /* 0x40 TG2	      */
	{ 0x98, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} }  /* 0x98 Reserved	      */
};

void program_coeffs(int duslic_id, int channel, struct _coeffs *coeffs, int tab_size)
{
	int i;

	for (i = 0; i < tab_size; i++)
	codsp_write_cop_block(duslic_id, channel, coeffs[i].addr, coeffs[i].values);
}

#define SS_OPEN_CIRCUIT			0
#define SS_RING_PAUSE			1
#define SS_ACTIVE			2
#define SS_ACTIVE_HIGH			3
#define SS_ACTIVE_RING			4
#define SS_RINGING			5
#define SS_ACTIVE_WITH_METERING		6
#define SS_ONHOOKTRNSM			7
#define SS_STANDBY			8
#define SS_MAX				8

static void codsp_set_slic(int duslic_id, int channel, int state)
{
	unsigned char v;

	v = codsp_read_sop_char(duslic_id, channel, BCR1_ADDR);

	switch (state) {

		case SS_ACTIVE:
			codsp_write_sop_char(duslic_id, channel, BCR1_ADDR, (v & ~BCR1_ACTR) | BCR1_ACTL);
			codsp_set_ciop_m(duslic_id, channel, CODSP_M_ANY_ACT);
			break;

		case SS_ACTIVE_HIGH:
			codsp_write_sop_char(duslic_id, channel, BCR1_ADDR, v & ~(BCR1_ACTR | BCR1_ACTL));
			codsp_set_ciop_m(duslic_id, channel, CODSP_M_ANY_ACT);
			break;

		case SS_ACTIVE_RING:
		case SS_ONHOOKTRNSM:
			codsp_write_sop_char(duslic_id, channel, BCR1_ADDR, (v & ~BCR1_ACTL) | BCR1_ACTR);
			codsp_set_ciop_m(duslic_id, channel, CODSP_M_ANY_ACT);
			break;

		case SS_STANDBY:
			codsp_write_sop_char(duslic_id, channel, BCR1_ADDR, v & ~(BCR1_ACTL | BCR1_ACTR));
			codsp_set_ciop_m(duslic_id, channel, CODSP_M_SLEEP_PWRDN);
			break;

		case SS_OPEN_CIRCUIT:
			codsp_set_ciop_m(duslic_id, channel, CODSP_M_PWRDN_HIZ);
			break;

		case SS_RINGING:
			codsp_set_ciop_m(duslic_id, channel, CODSP_M_RING);
			break;

		case SS_RING_PAUSE:
			codsp_set_ciop_m(duslic_id, channel, CODSP_M_RING_PAUSE);
			break;
	}
}

const unsigned char Ring_Sin_28Vrms_25Hz[8] = { 0x90, 0x30, 0x1B, 0xC0, 0xC3, 0x9C, 0x88, 0x00 };
const unsigned char Max_HookRingTh[3] = { 0x7B, 0x41, 0x62 };

void retrieve_slic_state(int slic_id)
{
	int duslic_id = slic_id >> 1;
	int channel = slic_id & 1;

	/* Retrieve the state of the SLICs */
	codsp_write_sop_char(duslic_id, channel, LMCR2_ADDR, 0x00);

	/* wait at least 1000us to clear the LM_OK and 500us to set the LM_OK ==> for the LM to make the first Measurement */
	udelay(10000);

	codsp_write_sop_char(duslic_id, channel, LMCR1_ADDR, LMCR1_LM_THM | LMCR1_LM_MASK);
	codsp_set_slic(duslic_id, channel, SS_ACTIVE_HIGH);
	codsp_write_sop_char(duslic_id, channel, LMCR3_ADDR, 0x40);

	/* Program Default Hook Ring thresholds */
	codsp_write_cop_block(duslic_id, channel, dc_coeffs[1].addr, dc_coeffs[1].values);

	/* Now program Hook Threshold while Ring and ac RingTrip to max values */
	codsp_write_cop_block(duslic_id, channel, dc_coeffs[3].addr, dc_coeffs[3].values);

	codsp_write_sop_short(duslic_id, channel, OFR1_ADDR, 0x0000);

	udelay(40000);
}

int wait_level_metering_finish(int duslic_id, int channel)
{
	int cnt;

	for (cnt = 0; cnt < 1000 &&
		(codsp_read_sop_char(duslic_id, channel, INTREG2_ADDR) & LM_OK_SRC_IRG_2) == 0; cnt++) { }

	return cnt != 1000;
}

int measure_on_hook_voltages(int slic_id, long *vdd,
		long *v_oh_H, long *v_oh_L, long *ring_mean_v, long *ring_rms_v)
{
	short LM_Result, Offset_Compensation;	/* Signed 16 bit */
	long int VDD, VDD_diff, V_in, V_out, Divider_Ratio, Vout_diff ;
	unsigned char err_mask = 0;
	int duslic_id = slic_id >> 1;
	int channel = slic_id & 1;
	int i;

	/* measure VDD */
	/* Now select the VDD level Measurement (but first of all Hold the DC characteristic) */
	codsp_write_sop_char(duslic_id, channel, TSTR5_ADDR, TSTR5_DC_HOLD);

	/* Activate Test Mode ==> To Enable DC Hold !!! */
	/* (else the LMRES is treated as Feeding Current and the Feeding voltage changes */
	/* imediatelly (after 500us when the LMRES Registers is updated for the first time after selection of (IO4-IO3) measurement !!!!))*/
	codsp_write_sop_char(duslic_id, channel, LMCR1_ADDR, LMCR1_TEST_EN | LMCR1_LM_THM | LMCR1_LM_MASK);

	udelay(40000);

	/* Now I Can select what to measure by DC Level Meter (select IO4-IO3) */
	codsp_write_sop_char(duslic_id, channel, LMCR2_ADDR, LMCR2_LM_SEL_VDD);

	/* wait at least 1000us to clear the LM_OK and 500us to set the LM_OK ==> for the LM to make the first Measurement */
	udelay(10000);

	/* Now Read the LM Result Registers */
	LM_Result = codsp_read_sop_short(duslic_id, channel, LMRES1_ADDR);
	VDD = (-1)*((((long int)LM_Result) * 390L ) >> 15) ;	/* VDDx100 */

	*vdd = VDD;

	VDD_diff = VDD - TARGET_VDDx100;

	if (VDD_diff < 0)
		VDD_diff = -VDD_diff;

	if (VDD_diff > VDD_MAX_DIFFx100)
		err_mask |= 1;

	Divider_Ratio = TARGET_V_DIVIDER_RATIO_x100;

	codsp_write_sop_char(duslic_id, channel, LMCR2_ADDR, 0x00);
	codsp_write_sop_char(duslic_id, channel, LMCR1_ADDR, LMCR1_LM_THM | LMCR1_LM_MASK);

	codsp_set_slic(duslic_id, channel, SS_ACTIVE_HIGH); /* Go back to ONHOOK Voltage */

	udelay(40000);

	codsp_write_sop_char(duslic_id, channel,
		LMCR1_ADDR, LMCR1_TEST_EN | LMCR1_LM_THM | LMCR1_LM_MASK);

	udelay(40000);

	/* Now I Can select what to measure by DC Level Meter (select IO4-IO3) */
	codsp_write_sop_char(duslic_id, channel, LMCR2_ADDR, LMCR2_LM_SEL_IO4_MINUS_IO3);

	/* wait at least 1000us to clear the LM_OK and 500us to set the LM_OK ==> for the LM to make the first Measurement */
	udelay(10000);

	/* Now Read the LM Result Registers */
	LM_Result = codsp_read_sop_short(duslic_id, channel, LMRES1_ADDR);
	V_in = (-1)* ((((long int)LM_Result) * V_AD_x10000 ) >> 15) ;  /* Vin x 10000*/

	V_out = (V_in * Divider_Ratio) / 10000L ;	/* Vout x100 */

	*v_oh_H = V_out;

	Vout_diff = V_out - TARGET_ONHOOK_BATH_x100;

	if (Vout_diff < 0)
		Vout_diff = -Vout_diff;

	if (Vout_diff > V_OUT_BATH_MAX_DIFFx100)
		err_mask |= 2;

	codsp_set_slic(duslic_id, channel, SS_ACTIVE); /* Go back to ONHOOK Voltage */

	udelay(40000);

	/* Now Read the LM Result Registers */
	LM_Result = codsp_read_sop_short(duslic_id, channel, LMRES1_ADDR);

	V_in = (-1)* ((((long int)LM_Result) * V_AD_x10000 ) >> 15) ;  /* Vin x 10000*/

	V_out = (V_in * Divider_Ratio) / 10000L ;	/* Vout x100 */

	*v_oh_L = V_out;

	Vout_diff = V_out - TARGET_ONHOOK_BATL_x100;

	if (Vout_diff < 0)
		Vout_diff = -Vout_diff;

	if (Vout_diff > V_OUT_BATL_MAX_DIFFx100)
		err_mask |= 4;

	/* perform ring tests */

	codsp_write_sop_char(duslic_id, channel, LMCR2_ADDR, 0x00);
	codsp_write_sop_char(duslic_id, channel, LMCR1_ADDR, LMCR1_LM_THM | LMCR1_LM_MASK);

	udelay(40000);

	codsp_write_sop_char(duslic_id, channel, LMCR3_ADDR, LMCR3_RTR_SEL | LMCR3_RNG_OFFSET_NONE);

	/* Now program RO1 =0V , Ring Amplitude and frequency and shift factor K = 1 (LMDC=0x0088)*/
	codsp_write_cop_block(duslic_id, channel, RING_PARAMS_START_ADDR, Ring_Sin_28Vrms_25Hz);

	/* By Default RO1 is selected when ringing RNG-OFFSET = 00 */

	/* Now program Hook Threshold while Ring and ac RingTrip to max values */
	for(i = 0; i < sizeof(Max_HookRingTh); i++)
		codsp_write_cop_char(duslic_id, channel, HOOK_THRESH_RING_START_ADDR + i, Max_HookRingTh[i]);

	codsp_write_sop_short(duslic_id, channel, OFR1_ADDR, 0x0000);

	codsp_set_slic(duslic_id, channel, SS_RING_PAUSE); /* Start Ringing */

	/* select source for the levelmeter to be IO4-IO3 */
	codsp_write_sop_char(duslic_id, channel, LMCR2_ADDR, LMCR2_LM_SEL_IO4_MINUS_IO3);

	udelay(40000);

	/* Before Enabling Level Meter Programm the apropriate shift factor K_INTDC=(4 if Rectifier Enabled and 2 if Rectifier Disabled) */
	codsp_write_cop_char(duslic_id, channel, RING_PARAMS_START_ADDR + 7, K_INTDC_RECT_OFF);

	udelay(10000);

	/* Enable LevelMeter to Integrate only once (Rectifier Disabled) */
	codsp_write_sop_char(duslic_id, channel,
			LMCR1_ADDR, LMCR1_LM_THM | LMCR1_LM_MASK | LMCR1_LM_EN | LMCR1_LM_ONCE);

	udelay(40000); /* Integration Period == Ring Period = 40ms (for 25Hz Ring) */

	if (wait_level_metering_finish(duslic_id, channel)) {

		udelay(10000); /* To be sure that Integration Results are Valid wait at least 500us !!! */

		/* Now Read the LM Result Registers (Will be valid until LM_EN becomes zero again( after that the Result is updated every 500us) ) */
		Offset_Compensation = codsp_read_sop_short(duslic_id, channel, LMRES1_ADDR);
		Offset_Compensation = (-1) * ((Offset_Compensation * (1 << K_INTDC_RECT_OFF)) / N_SAMPLES);

		/* Disable LevelMeter ==> In order to be able to restart Integrator again (for the next integration) */
		codsp_write_sop_char(duslic_id, channel, LMCR1_ADDR, LMCR1_LM_THM | LMCR1_LM_MASK | LMCR1_LM_ONCE);

		/* Now programm Integrator Offset Registers !!! */
		codsp_write_sop_short(duslic_id, channel, OFR1_ADDR, Offset_Compensation);

		codsp_set_slic(duslic_id, channel, SS_RINGING); /* Start Ringing */

		udelay(40000);

		/* Reenable Level Meter Integrator (The Result will be valid after Integration Period=Ring Period and until LN_EN become zero again) */
		codsp_write_sop_char(duslic_id, channel,
				LMCR1_ADDR, LMCR1_LM_THM | LMCR1_LM_MASK | LMCR1_LM_EN | LMCR1_LM_ONCE);

		udelay(40000); /* Integration Period == Ring Period = 40ms (for 25Hz Ring) */

		/* Poll the LM_OK bit to see when Integration Result is Ready */
		if (wait_level_metering_finish(duslic_id, channel)) {

			udelay(10000); /* wait at least 500us to be sure that the Integration Result are valid !!! */

			/* Now Read the LM Result Registers (They will hold their value until LM_EN become zero again */
			/*				    ==>After that Result Regs will be updated every 500us !!!) */
			LM_Result = codsp_read_sop_short(duslic_id, channel, LMRES1_ADDR);
			V_in = (-1) * ( ( (((long int)LM_Result) * V_AD_x10000) / N_SAMPLES) >> (15 - K_INTDC_RECT_OFF)) ;  /* Vin x 10000*/

			V_out = (V_in * Divider_Ratio) / 10000L ;	/* Vout x100 */

			if (V_out < 0)
				V_out= -V_out;

			if (V_out > MAX_V_RING_MEANx100)
				err_mask |= 8;

			*ring_mean_v = V_out;
		} else {
			err_mask |= 8;
			*ring_mean_v = 0;
		}
	} else {
		err_mask |= 8;
		*ring_mean_v = 0;
	}

	/* Disable LevelMeter ==> In order to be able to restart Integrator again (for the next integration) */
	codsp_write_sop_char(duslic_id, channel, LMCR1_ADDR,
		LMCR1_LM_THM | LMCR1_LM_MASK | LMCR1_LM_ONCE);
	codsp_write_sop_short(duslic_id, channel, OFR1_ADDR, 0x0000);

	codsp_set_slic(duslic_id, channel, SS_RING_PAUSE); /* Start Ringing */

	/* Now Enable Rectifier */
	/* select source for the levelmeter to be IO4-IO3 */
	codsp_write_sop_char(duslic_id, channel, LMCR2_ADDR,
		LMCR2_LM_SEL_IO4_MINUS_IO3 | LMCR2_LM_RECT);

	/* Program the apropriate shift factor K_INTDC (in order to avoid Overflow at Integtation Result !!!) */
	codsp_write_cop_char(duslic_id, channel, RING_PARAMS_START_ADDR + 7, K_INTDC_RECT_ON);

	udelay(40000);

	/* Reenable Level Meter Integrator (The Result will be valid after Integration Period=Ring Period and until LN_EN become zero again) */
	codsp_write_sop_char(duslic_id, channel, LMCR1_ADDR,
			LMCR1_LM_THM | LMCR1_LM_MASK | LMCR1_LM_EN | LMCR1_LM_ONCE);

	udelay(40000);

	/* Poll the LM_OK bit to see when Integration Result is Ready */
	if (wait_level_metering_finish(duslic_id, channel)) {

		udelay(10000);

		/* Now Read the LM Result Registers (They will hold their value until LM_EN become zero again */
		/*				    ==>After that Result Regs will be updated every 500us !!!) */
		Offset_Compensation = codsp_read_sop_short(duslic_id, channel, LMRES1_ADDR);
		Offset_Compensation = (-1) * ((Offset_Compensation * (1 << K_INTDC_RECT_ON)) / N_SAMPLES);

		/* Disable LevelMeter ==> In order to be able to restart Integrator again (for the next integration) */
		codsp_write_sop_char(duslic_id, channel, LMCR1_ADDR, LMCR1_LM_THM | LMCR1_LM_MASK | LMCR1_LM_ONCE);

		/* Now programm Integrator Offset Registers !!! */
		codsp_write_sop_short(duslic_id, channel, OFR1_ADDR, Offset_Compensation);

		/* Be sure that a Ring is generated !!!! */
		codsp_set_slic(duslic_id, channel, SS_RINGING); /* Start Ringing again */

		udelay(40000);

		/* Reenable Level Meter Integrator (The Result will be valid after Integration Period=Ring Period and until LN_EN become zero again) */
		codsp_write_sop_char(duslic_id, channel, LMCR1_ADDR,
				LMCR1_LM_THM | LMCR1_LM_MASK | LMCR1_LM_EN | LMCR1_LM_ONCE);

		udelay(40000);

		/* Poll the LM_OK bit to see when Integration Result is Ready */
		if (wait_level_metering_finish(duslic_id, channel)) {

			udelay(10000);

			/* Now Read the LM Result Registers (They will hold their value until LM_EN become zero again */
			/*				    ==>After that Result Regs will be updated every 500us !!!) */
			LM_Result = codsp_read_sop_short(duslic_id, channel, LMRES1_ADDR);
			V_in = (-1) *  ( ( (((long int)LM_Result) * V_AD_x10000) / N_SAMPLES) >> (15 - K_INTDC_RECT_ON) ) ;  /* Vin x 10000*/

			V_out = (((V_in * Divider_Ratio) / 10000L) * RMS_MULTIPLIERx100) / 100 ;	/* Vout_RMS x100 */
			if (V_out < 0)
				V_out = -V_out;

			Vout_diff = (V_out - TARGET_V_RING_RMSx100);

			if (Vout_diff < 0)
				Vout_diff = -Vout_diff;

			if (Vout_diff > V_RMS_RING_MAX_DIFFx100)
				err_mask |= 16;

			*ring_rms_v = V_out;
		} else {
			err_mask |= 16;
			*ring_rms_v = 0;
		}
	} else {
		err_mask |= 16;
		*ring_rms_v = 0;
	}
	/* Disable LevelMeter ==> In order to be able to restart Integrator again (for the next integration) */
	codsp_write_sop_char(duslic_id, channel, LMCR1_ADDR, LMCR1_LM_THM | LMCR1_LM_MASK);

	retrieve_slic_state(slic_id);

	return(err_mask);
}

int test_dtmf(int slic_id)
{
	unsigned char code;
	unsigned char b;
	unsigned int intreg;
	int duslic_id = slic_id >> 1;
	int channel = slic_id & 1;

	for (code = 0; code < 16; code++) {
		b = codsp_read_sop_char(duslic_id, channel, DSCR_ADDR);
		codsp_write_sop_char(duslic_id, channel, DSCR_ADDR,
			(b & ~(DSCR_PTG | DSCR_DG_KEY(15))) | DSCR_DG_KEY(code) | DSCR_TG1_EN | DSCR_TG2_EN);
		udelay(80000);

		intreg = codsp_read_sop_int(duslic_id, channel, INTREG1_ADDR);
		if ((intreg & CODSP_INTREG_INT_CH) == 0)
			break;

		if ((intreg & CODSP_INTREG_DTMF_OK) == 0 ||
				codsp_dtmf_map[(intreg >> 10) & 15] != codsp_dtmf_map[code])
			break;

		b = codsp_read_sop_char(duslic_id, channel, DSCR_ADDR);
		codsp_write_sop_char(duslic_id, channel, DSCR_ADDR,
				b & ~(DSCR_COR8 | DSCR_TG1_EN | DSCR_TG2_EN));

		udelay(80000);

		intreg = codsp_read_sop_int(duslic_id, channel, INTREG1_ADDR); /* for dtmf_pause irq */
	}

	if (code != 16) {
		b = codsp_read_sop_char(duslic_id, channel, DSCR_ADDR); /* stop dtmf */
		codsp_write_sop_char(duslic_id, channel, DSCR_ADDR,
				b & ~(DSCR_COR8 | DSCR_TG1_EN | DSCR_TG2_EN));
		return(1);
	}

	return(0);
}

void data_up_persist_time(int duslic_id, int channel, int time_ms)
{
	unsigned char b;

	b = codsp_read_sop_char(duslic_id, channel, IOCTL3_ADDR);
	b = (b & 0x0F) | ((time_ms & 0x0F) << 4);
	codsp_write_sop_char(duslic_id, channel, IOCTL3_ADDR, b);
}

static void program_dtmf_params(int duslic_id, int channel)
{
	unsigned char b;

	codsp_write_pop_char(duslic_id, channel, DTMF_LEV_ADDR, 0x10);
	codsp_write_pop_char(duslic_id, channel, DTMF_TWI_ADDR, 0x0C);
	codsp_write_pop_char(duslic_id, channel, DTMF_NCF_H_ADDR, 0x79);
	codsp_write_pop_char(duslic_id, channel, DTMF_NCF_L_ADDR, 0x10);
	codsp_write_pop_char(duslic_id, channel, DTMF_NBW_H_ADDR, 0x02);
	codsp_write_pop_char(duslic_id, channel, DTMF_NBW_L_ADDR, 0xFB);
	codsp_write_pop_char(duslic_id, channel, DTMF_GAIN_ADDR, 0x91);
	codsp_write_pop_char(duslic_id, channel, DTMF_RES1_ADDR, 0x00);
	codsp_write_pop_char(duslic_id, channel, DTMF_RES2_ADDR, 0x00);
	codsp_write_pop_char(duslic_id, channel, DTMF_RES3_ADDR, 0x00);

	b = codsp_read_sop_char(duslic_id, channel, BCR5_ADDR);
	codsp_write_sop_char(duslic_id, channel, BCR5_ADDR, b | BCR5_DTMF_EN);
}

static void codsp_channel_full_reset(int duslic_id, int channel)
{

	program_coeffs(duslic_id, channel, ac_coeffs, sizeof(ac_coeffs) / sizeof(struct _coeffs));
	program_coeffs(duslic_id, channel, dc_coeffs, sizeof(dc_coeffs) / sizeof(struct _coeffs));

	/* program basic configuration registers */
	codsp_write_sop_char(duslic_id, channel, BCR1_ADDR, 0x01);
	codsp_write_sop_char(duslic_id, channel, BCR2_ADDR, 0x41);
	codsp_write_sop_char(duslic_id, channel, BCR3_ADDR, 0x43);
	codsp_write_sop_char(duslic_id, channel, BCR4_ADDR, 0x00);
	codsp_write_sop_char(duslic_id, channel, BCR5_ADDR, 0x00);

	codsp_write_sop_char(duslic_id, channel, DSCR_ADDR, 0x04);		/* PG */

	program_dtmf_params(duslic_id, channel);

	codsp_write_sop_char(duslic_id, channel, LMCR3_ADDR, 0x40);	/* RingTRip_SEL */

	data_up_persist_time(duslic_id, channel, 4);

	codsp_write_sop_char(duslic_id, channel, MASK_ADDR, 0xFF);     /* All interrupts masked */

	codsp_set_slic(duslic_id, channel, SS_ACTIVE_HIGH);
}

static int codsp_chip_full_reset(int duslic_id)
{
	int i, cnt;
	int intreg[NUM_CHANNELS];
	unsigned char pcm_resync;
	unsigned char revision;

	codsp_reset_chip(duslic_id);

	udelay(2000);

	for (i = 0; i < NUM_CHANNELS; i++)
		intreg[i] = codsp_read_sop_int(duslic_id, i, INTREG1_ADDR);

	udelay(1500);

	if (_PORTC_GET(com_hook_mask_tab[duslic_id]) == 0) {
		printf("_HOOK(%d) stayed low\n", duslic_id);
		return -1;
	}

	for (pcm_resync = 0, i = 0; i < NUM_CHANNELS; i++) {
		if (intreg[i] & CODSP_INTREG_SYNC_FAIL)
			pcm_resync |= 1 << i;
	}

	for (cnt = 0; cnt < 5 && pcm_resync; cnt++) {
		for (i = 0; i < NUM_CHANNELS; i++)
			codsp_resync_channel(duslic_id, i);

		udelay(2000);

		pcm_resync = 0;

		for (i = 0; i < NUM_CHANNELS; i++) {
			if (codsp_read_sop_int(duslic_id, i, INTREG1_ADDR) & CODSP_INTREG_SYNC_FAIL)
				pcm_resync |= 1 << i;
		}
	}

	if (cnt == 5) {
		printf("PCM_Resync(%u) not completed\n", duslic_id);
		return -2;
	}

	revision = codsp_read_sop_char(duslic_id, 0, REVISION_ADDR);
	printf("DuSLIC#%d hardware version %d.%d\r\n", duslic_id, (revision & 0xF0) >> 4, revision & 0x0F);

	codsp_write_sop_char(duslic_id, 0, XCR_ADDR, 0x80);	/* EDSP_EN */

	for (i = 0; i < NUM_CHANNELS; i++) {
		codsp_write_sop_char(duslic_id, i, PCMC1_ADDR, 0x01);
		codsp_channel_full_reset(duslic_id, i);
	}

	return 0;
}

int slic_self_test(int duslic_mask)
{
	int slic;
	int i;
	int r;
	long vdd, v_oh_H, v_oh_L, ring_mean_v, ring_rms_v;
	const char *err_txt[] = { "VDD", "V_OH_H", "V_OH_L", "V_RING_MEAN", "V_RING_RMS" };
	int error = 0;

	for (slic = 0; slic < MAX_SLICS; slic++) { /* voltages self test */
		if (duslic_mask & (1 << (slic >> 1))) {
			r = measure_on_hook_voltages(slic, &vdd,
				&v_oh_H, &v_oh_L, &ring_mean_v, &ring_rms_v);

			printf("SLIC %u measured voltages (x100):\n\t"
				    "VDD = %ld\tV_OH_H = %ld\tV_OH_L = %ld\tV_RING_MEAN = %ld\tV_RING_RMS = %ld\n",
				    slic, vdd, v_oh_H, v_oh_L, ring_mean_v, ring_rms_v);

			if (r != 0)
				error |= 1 << slic;

			for (i = 0; i < 5; i++)
				if (r & (1 << i))
					printf("\t%s out of range\n", err_txt[i]);
		}
	}

	for (slic = 0; slic < MAX_SLICS; slic++) { /* voice path self test */
		if (duslic_mask & (1 << (slic >> 1))) {
			printf("SLIC %u VOICE PATH...CHECKING", slic);
			printf("\rSLIC %u VOICE PATH...%s\n", slic,
				(r = test_dtmf(slic)) != 0 ? "FAILED  " : "PASSED  ");

			if (r != 0)
				error |= 1 << slic;
		}
	}

	return(error);
}

#if defined(CONFIG_NETTA_ISDN)

#define SPIENS1		(1 << (31 - 15))
#define SPIENS2		(1 << (31 - 19))

static const int spiens_mask_tab[2] = { SPIENS1, SPIENS2 };
int s_initialized = 0;

static inline unsigned int s_transfer_internal(int s_id, unsigned int address, unsigned int value)
{
	unsigned int rx, v;

	_PORTB_SET(spiens_mask_tab[s_id], 0);

	rx = __SPI_Transfer(address);

	switch (address & 0xF0) {
	case 0x60:	/* write byte register */
	case 0x70:
		rx = __SPI_Transfer(value);
		break;

	case 0xE0:	/* read R6 register */
		v = __SPI_Transfer(0);

		rx = (rx << 8) | v;

		break;

	case 0xF0:	/* read byte register */
		rx = __SPI_Transfer(0);

		break;
	}

	_PORTB_SET(spiens_mask_tab[s_id], 1);

	return rx;
}

static void s_write_BR(int s_id, unsigned int regno, unsigned int val)
{
	unsigned int address;
	unsigned int v;

	address = 0x70 | (regno & 15);
	val &= 0xff;

	v = s_transfer_internal(s_id, address, val);
}

static void s_write_OR(int s_id, unsigned int regno, unsigned int val)
{
	unsigned int address;
	unsigned int v;

	address = 0x70 | (regno & 15);
	val &= 0xff;

	v = s_transfer_internal(s_id, address, val);
}

static void s_write_NR(int s_id, unsigned int regno, unsigned int val)
{
	unsigned int address;
	unsigned int v;

	address = (regno & 7) << 4;
	val &= 0xf;

	v = s_transfer_internal(s_id, address | val, 0x00);
}

#define BR7_IFR			0x08	/* IDL2 free run */
#define BR7_ICSLSB		0x04	/* IDL2 clock speed LSB */

#define BR15_OVRL_REG_EN	0x80
#define OR7_D3VR		0x80	/* disable 3V regulator */

#define OR8_TEME		0x10	/* TE mode enable */
#define OR8_MME			0x08	/* master mode enable */

void s_initialize(void)
{
	int s_id;

	for (s_id = 0; s_id < 2; s_id++) {
		s_write_BR(s_id, 7, BR7_IFR | BR7_ICSLSB);
		s_write_BR(s_id, 15, BR15_OVRL_REG_EN);
		s_write_OR(s_id, 8, OR8_TEME | OR8_MME);
		s_write_OR(s_id, 7, OR7_D3VR);
		s_write_OR(s_id, 6, 0);
		s_write_BR(s_id, 15, 0);
		s_write_NR(s_id, 3, 0);
	}
}

#endif

int board_post_codec(int flags)
{
	int j;
	int r;
	int duslic_mask;

	printf("board_post_dsp\n");

#if defined(CONFIG_NETTA_ISDN)
	if (s_initialized == 0) {
		s_initialize();
		s_initialized = 1;

		printf("s_initialized\n");

		udelay(20000);
	}
#endif
	duslic_mask = 0;

	for (j = 0; j < MAX_DUSLIC; j++) {
		if (codsp_chip_full_reset(j) < 0)
			printf("Error initializing DuSLIC#%d\n", j);
		else
			duslic_mask |= 1 << j;
	}

	if (duslic_mask != 0) {
		printf("Testing SLICs...\n");

		r = slic_self_test(duslic_mask);
		for (j = 0; j < MAX_SLICS; j++) {
			if (duslic_mask & (1 << (j >> 1)))
				printf("SLIC %u...%s\n", j, r & (1 << j) ? "FAULTY" : "OK");
		}
	}
	printf("DuSLIC self test finished\n");

	return 0;	/* return -1 on error */
}
