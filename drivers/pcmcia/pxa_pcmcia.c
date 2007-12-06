#include <common.h>
#include <config.h>

#ifdef CONFIG_PXA_PCMCIA

#include <pcmcia.h>
#include <asm/arch/pxa-regs.h>
#include <asm/io.h>

static inline void msWait(unsigned msVal)
{
	udelay(msVal*1000);
}

int pcmcia_on (void)
{
	unsigned int reg_arr[] = {
		0x48000028, CFG_MCMEM0_VAL,
		0x4800002c, CFG_MCMEM1_VAL,
		0x48000030, CFG_MCATT0_VAL,
		0x48000034, CFG_MCATT1_VAL,
		0x48000038, CFG_MCIO0_VAL,
		0x4800003c, CFG_MCIO1_VAL,

		0, 0
	};
	int i, rc;

#ifdef CONFIG_EXADRON1
	int cardDetect;
	volatile unsigned int *v_pBCRReg =
			(volatile unsigned int *) 0x08000000;
#endif

	debug ("%s\n", __FUNCTION__);

	i = 0;
	while (reg_arr[i])
		*((volatile unsigned int *) reg_arr[i++]) |= reg_arr[i++];
	udelay (1000);

	debug ("%s: programmed mem controller \n", __FUNCTION__);

#ifdef CONFIG_EXADRON1

/*define useful BCR masks */
#define BCR_CF_INIT_VAL  		    0x00007230
#define BCR_CF_PWRON_BUSOFF_RESETOFF_VAL    0x00007231
#define BCR_CF_PWRON_BUSOFF_RESETON_VAL     0x00007233
#define BCR_CF_PWRON_BUSON_RESETON_VAL      0x00007213
#define BCR_CF_PWRON_BUSON_RESETOFF_VAL     0x00007211

	/* we see from the GPIO bit if the card is present */
	cardDetect = !(GPLR0 & GPIO_bit (14));

	if (cardDetect) {
		printf ("No PCMCIA card found!\n");
	}

	/* reset the card via the BCR line */
	*v_pBCRReg = (unsigned) BCR_CF_INIT_VAL;
	msWait (500);

	*v_pBCRReg = (unsigned) BCR_CF_PWRON_BUSOFF_RESETOFF_VAL;
	msWait (500);

	*v_pBCRReg = (unsigned) BCR_CF_PWRON_BUSOFF_RESETON_VAL;
	msWait (500);

	*v_pBCRReg = (unsigned) BCR_CF_PWRON_BUSON_RESETON_VAL;
	msWait (500);

	*v_pBCRReg = (unsigned) BCR_CF_PWRON_BUSON_RESETOFF_VAL;
	msWait (1500);

	/* enable address bus */
	GPCR1 = 0x01;
	/* and the first CF slot */
	MECR = 0x00000002;

#endif /* EXADRON 1 */

	rc = check_ide_device (0);	/* use just slot 0 */

	return rc;
}

#if defined(CONFIG_CMD_PCMCIA)
int pcmcia_off (void)
{
	return 0;
}
#endif

#endif /* CONFIG_PXA_PCMCIA */
