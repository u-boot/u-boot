/*
 * (C) Copyright 2007 DENX Software Engineering
 *
 * Author: Bartlomiej Sieka <tur@semihalf.com>
 * Author: Grzegorz Bernacki <gjb@semihalf.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _CM5200_H
#define _CM5200_H


/*
 * Definitions and declarations for the modules of the cm5200 platform. Mostly
 * related to reading the hardware identification data (HW ID) from the I2C
 * EEPROM, detection of the particular module we are executing on, and
 * appropriate SDRAM controller initialization.
 */


#define CM5200_UNKNOWN_MODULE	0xffffffff

enum {
	DEVICE_NAME,		/* 0 */
	GENERATION,		/* 1 */
	PCB_NAME,		/* 2 */
	FORM,			/* 3 */
	VERSION,		/* 4 */
	IDENTIFICATION_NUMBER,	/* 5 */
	MAJOR_SW_VERSION,	/* 6 */
	MINOR_SW_VERSION,	/* 7 */
	/* add new alements above this line */
	HW_ID_ELEM_COUNT	/* count */
};

/*
 * Sect. 4.1 "CM1.Q/CMU1.Q Supervisory Microcontroller Interface Definition"
 */

#define DEVICE_NAME_OFFSET		0x02
#define GENERATION_OFFSET		0x0b
#define PCB_NAME_OFFSET			0x0c
#define FORM_OFFSET			0x15
#define VERSION_OFFSET			0x16
#define IDENTIFICATION_NUMBER_OFFSET	0x19
#define MAJOR_SW_VERSION_OFFSET		0x0480
#define MINOR_SW_VERSION_OFFSET		0x0481


#define DEVICE_NAME_LEN			0x09
#define GENERATION_LEN			0x01
#define PCB_NAME_LEN			0x09
#define FORM_LEN			0x01
#define VERSION_LEN			0x03
#define IDENTIFICATION_NUMBER_LEN	0x09
#define MAJOR_SW_VERSION_LEN		0x01
#define MINOR_SW_VERSION_LEN		0x01

#define HW_ID_ELEM_MAXLEN		0x09	/* MAX(XXX_LEN) */

/* entire HW ID in EEPROM is 64 bytes, so longer module name is unlikely */
#define MODULE_NAME_MAXLEN		64


/* storage for HW ID read from EEPROM */
typedef char hw_id_t[HW_ID_ELEM_COUNT][HW_ID_ELEM_MAXLEN];


/* HW ID layout in EEPROM */
static struct {
	unsigned int offset;
	unsigned int length;
} hw_id_format[HW_ID_ELEM_COUNT] = {
	{DEVICE_NAME_OFFSET,		DEVICE_NAME_LEN},
	{GENERATION_OFFSET,		GENERATION_LEN},
	{PCB_NAME_OFFSET,		PCB_NAME_LEN},
	{FORM_OFFSET,			FORM_LEN},
	{VERSION_OFFSET,		VERSION_LEN},
	{IDENTIFICATION_NUMBER_OFFSET,	IDENTIFICATION_NUMBER_LEN},
	{MAJOR_SW_VERSION_OFFSET,	MAJOR_SW_VERSION_LEN},
	{MINOR_SW_VERSION_OFFSET,	MINOR_SW_VERSION_LEN},
};


/* HW ID data found in EEPROM on supported modules */
static char *cm1_qa_hw_id[HW_ID_ELEM_COUNT] = {
	"CM",		/* DEVICE_NAME */
	"1",		/* GENERATION */
	"CM1",		/* PCB_NAME */
	"Q",		/* FORM */
	"A",		/* VERSION */
	"591881",	/* IDENTIFICATION_NUMBER */
	"",		/* MAJOR_SW_VERSION */
	"",		/* MINOR_SW_VERSION */
};

static char *cm11_qa_hw_id[HW_ID_ELEM_COUNT] = {
	"CM",		/* DEVICE_NAME */
	"1",		/* GENERATION */
	"CM11",		/* PCB_NAME */
	"Q",		/* FORM */
	"A",		/* VERSION */
	"594200",	/* IDENTIFICATION_NUMBER */
	"",		/* MAJOR_SW_VERSION */
	"",		/* MINOR_SW_VERSION */
};

static char *cmu1_qa_hw_id[HW_ID_ELEM_COUNT] = {
	"CMU",		/* DEVICE_NAME */
	"1",		/* GENERATION */
	"CMU1",		/* PCB_NAME */
	"Q",		/* FORM */
	"A",		/* VERSION */
	"594128",	/* IDENTIFICATION_NUMBER */
	"",		/* MAJOR_SW_VERSION */
	"",		/* MINOR_SW_VERSION */
};


/* list of known modules */
static char **hw_id_list[] = {
	cm1_qa_hw_id,
	cm11_qa_hw_id,
	cmu1_qa_hw_id,
};

/* indices to the above list - keep in sync */
enum {
	CM1_QA,
	CM11_QA,
	CMU1_QA,
};


/* identify modules based on these hw id elements */
static int hw_id_identify[] = {
	PCB_NAME,
	FORM,
	VERSION,
};


/* Registers' settings for SDRAM controller intialization */
typedef struct {
	ulong mode;
	ulong control;
	ulong config1;
	ulong config2;
} mem_conf_t;

static mem_conf_t k4s561632E = {
	0x00CD0000,      /* CASL 3, burst length 8 */
	0x514F0000,
	0xE2333900,
	0x8EE70000
};

static mem_conf_t mt48lc32m16a2 = {
	0x00CD0000,      /* CASL 3, burst length 8 */
	0x514F0000,
	0xD2322800,
	0x8AD70000
};

static mem_conf_t* memory_config[] = {
	&k4s561632E,
	&mt48lc32m16a2
};

#endif /* _CM5200_H */
