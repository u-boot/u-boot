#include <common.h>
#include <commproc.h>

#if defined(CFG_I2C_UCODE_PATCH) || defined(CFG_SPI_UCODE_PATCH)

static void UcodeCopy (volatile cpm8xx_t *cpm);

void cpm_load_patch (volatile immap_t *immr)
{
	immr->im_cpm.cp_rccr &= ~0x0003;	/* Disable microcode program area */

	UcodeCopy ((cpm8xx_t *)&immr->im_cpm);	/* Copy ucode patch to DPRAM   */
#ifdef CFG_SPI_UCODE_PATCH
    {
	volatile spi_t *spi = (spi_t *) & immr->im_cpm.cp_dparam[PROFF_SPI];
	/* Activate the microcode per the instructions in the microcode manual */
	/* NOTE:  We're only relocating the SPI parameters (not I2C).          */
	immr->im_cpm.cp_cpmcr1 = 0x802a;	/* Write Trap register 1 value */
	immr->im_cpm.cp_cpmcr2 = 0x8028;	/* Write Trap register 2 value */
	spi->spi_rpbase = CFG_SPI_DPMEM_OFFSET;	/* Where to relocte SPI params */
    }
#endif

#ifdef CFG_I2C_UCODE_PATCH
    {
	volatile iic_t *iip = (iic_t *) & immr->im_cpm.cp_dparam[PROFF_IIC];
	/* Activate the microcode per the instructions in the microcode manual */
	/* NOTE:  We're only relocating the I2C parameters (not SPI).          */
	immr->im_cpm.cp_cpmcr3 = 0x802e;	/* Write Trap register 3 value */
	immr->im_cpm.cp_cpmcr4 = 0x802c;	/* Write Trap register 4 value */
	iip->iic_rpbase = CFG_I2C_DPMEM_OFFSET;	/* Where to relocte I2C params */
    }
#endif

	/*
	 * Enable DPRAM microcode to execute from the first 512 bytes
	 * and a 256 byte extension of DPRAM.
	 */
	immr->im_cpm.cp_rccr |= 0x0001;
}

static ulong patch_2000[] = {
	0x7FFFEFD9, 0x3FFD0000, 0x7FFB49F7, 0x7FF90000,
	0x5FEFADF7, 0x5F88ADF7, 0x5FEFAFF7, 0x5F88AFF7,
	0x3A9CFBC8, 0x77CAE1BB, 0xF4DE7FAD, 0xABAE9330,
	0x4E08FDCF, 0x6E0FAFF8, 0x7CCF76CF, 0xFDAFF9CF,
	0xABF88DC8, 0xAB5879F7, 0xB0927383, 0xDFD079F7,
	0xB090E6BB, 0xE5BBE74F, 0xB3FA6F0F, 0x6FFB76CE,
	0xEE0CF9CF, 0x2BFBEFEF, 0xCFEEF9CF, 0x76CEAD23,
	0x90B3DF99, 0x7FDDD0C1, 0x4BF847FD, 0x7CCF76CE,
	0xCFEF77CA, 0x7EAF7FAD, 0x7DFDF0B7, 0xEF7A7FCA,
	0x77CAFBC8, 0x6079E722, 0xFBC85FFF, 0xDFFF5FB3,
	0xFFFBFBC8, 0xF3C894A5, 0xE7C9EDF9, 0x7F9A7FAD,
	0x5F36AFE8, 0x5F5BFFDF, 0xDF95CB9E, 0xAF7D5FC3,
	0xAFED8C1B, 0x5FC3AFDD, 0x5FC5DF99, 0x7EFDB0B3,
	0x5FB3FFFE, 0xABAE5FB3, 0xFFFE5FD0, 0x600BE6BB,
	0x600B5FD0, 0xDFC827FB, 0xEFDF5FCA, 0xCFDE3A9C,
	0xE7C9EDF9, 0xF3C87F9E, 0x54CA7FED, 0x2D3A3637,
	0x756F7E9A, 0xF1CE37EF, 0x2E677FEE, 0x10EBADF8,
	0xEFDECFEA, 0xE52F7D9F, 0xE12BF1CE, 0x5F647E9A,
	0x4DF8CFEA, 0x5F717D9B, 0xEFEECFEA, 0x5F73E522,
	0xEFDE5F73, 0xCFDA0B61, 0x7385DF61, 0xE7C9EDF9,
	0x7E9A30D5, 0x1458BFFF, 0xF3C85FFF, 0xDFFFA7F8,
	0x5F5BBFFE, 0x7F7D10D0, 0x144D5F33, 0xBFFFAF78,
	0x5F5BBFFD, 0xA7F85F33, 0xBFFE77FD, 0x30BD4E08,
	0xFDCFE5FF, 0x6E0FAFF8, 0x7EEF7E9F, 0xFDEFF1CF,
	0x5F17ABF8, 0x0D5B5F5B, 0xFFEF79F7, 0x309EAFDD,
	0x5F3147F8, 0x5F31AFED, 0x7FDD50AF, 0x497847FD,
	0x7F9E7FED, 0x7DFD70A9, 0xEF7E7ECE, 0x6BA07F9E,
	0x2D227EFD, 0x30DB5F5B, 0xFFFD5F5B, 0xFFEF5F5B,
	0xFFDF0C9C, 0xAFED0A9A, 0xAFDD0C37, 0x5F37AFBD,
	0x7FBDB081, 0x5F8147F8,
};

static ulong patch_2F00[] = {
	0x3E303430, 0x34343737, 0xABBF9B99, 0x4B4FBDBD,
	0x59949334, 0x9FFF37FB, 0x9B177DD9, 0x936956BB,
	0xFBDD697B, 0xDD2FD113, 0x1DB9F7BB, 0x36313963,
	0x79373369, 0x3193137F, 0x7331737A, 0xF7BB9B99,
	0x9BB19795, 0x77FDFD3D, 0x573B773F, 0x737933F7,
	0xB991D115, 0x31699315, 0x31531694, 0xBF4FBDBD,
	0x35931497, 0x35376956, 0xBD697B9D, 0x96931313,
	0x19797937, 0x69350000,
};

static void UcodeCopy (volatile cpm8xx_t *cpm)
{
	vu_long *p;
	int i;

	p = (vu_long *)&(cpm->cp_dpmem[0x0000]);
	for (i=0; i < sizeof(patch_2000)/4; ++i) {
		p[i] = patch_2000[i];
	}

	p = (vu_long *)&(cpm->cp_dpmem[0x0F00]);
	for (i=0; i < sizeof(patch_2F00)/4; ++i) {
		p[i] = patch_2F00[i];
	}
}

#endif	/* CFG_I2C_UCODE_PATCH, CFG_SPI_UCODE_PATCH */
