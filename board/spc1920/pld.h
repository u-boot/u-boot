#ifndef __PLD_H__
#define __PLD_H__

typedef struct spc1920_pld {
	uchar com1_en;
	uchar dsp_reset;
	uchar dsp_hpi_on;
	uchar codec_dsp_power_en;
	uchar clk2_en;
	uchar clk3_select;
	uchar clk4_select;
} spc1920_pld_t;

#endif /* __PLD_H__ */
