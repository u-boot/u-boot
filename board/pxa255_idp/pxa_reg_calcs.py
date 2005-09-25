#!/usr/bin/python

# (C) Copyright 2004
# BEC Systems <http://bec-systems.com>
# Cliff Brake <cliff.brake@gmail.com>

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA

# calculations for PXA255 registers

class gpio:
	dir = '0'
	set = '0'
	clr = '0'
	alt = '0'
	desc = ''

	def __init__(self, dir=0, set=0, clr=0, alt=0, desc=''):
		self.dir = dir
		self.set = set
		self.clr = clr
		self.alt = alt
		self.desc = desc
		

# the following is a dictionary of all GPIOs in the system
# the key is the GPIO number


pxa255_alt_func = {
	0: ['gpio', 'none', 'none', 'none'],
	1: ['gpio', 'gpio reset', 'none', 'none'],
	2: ['gpio', 'none', 'none', 'none'],
	3: ['gpio', 'none', 'none', 'none'],
	4: ['gpio', 'none', 'none', 'none'],
	5: ['gpio', 'none', 'none', 'none'],
	6: ['gpio', 'MMC clk', 'none', 'none'],
	7: ['gpio', '48MHz clock', 'none', 'none'],
	8: ['gpio', 'MMC CS0', 'none', 'none'],
	9: ['gpio', 'MMC CS1', 'none', 'none'],
	10: ['gpio', 'RTC Clock', 'none', 'none'],
	11: ['gpio', '3.6MHz', 'none', 'none'],
	12: ['gpio', '32KHz', 'none', 'none'],
	13: ['gpio', 'none', 'MBGNT', 'none'],
	14: ['gpio', 'MBREQ', 'none', 'none'],
	15: ['gpio', 'none', 'nCS_1', 'none'],
	16: ['gpio', 'none', 'PWM0', 'none'],
	17: ['gpio', 'none', 'PWM1', 'none'],
	18: ['gpio', 'RDY', 'none', 'none'],
	19: ['gpio', 'DREQ[1]', 'none', 'none'],
	20: ['gpio', 'DREQ[0]', 'none', 'none'],
	21: ['gpio', 'none', 'none', 'none'],
	22: ['gpio', 'none', 'none', 'none'],
	23: ['gpio', 'none', 'SSP SCLK', 'none'],
	24: ['gpio', 'none', 'SSP SFRM', 'none'],
	25: ['gpio', 'none', 'SSP TXD', 'none'],
	26: ['gpio', 'SSP RXD', 'none', 'none'],
	27: ['gpio', 'SSP EXTCLK', 'none', 'none'],
	28: ['gpio', 'AC97 bitclk in, I2S bitclock out', 'I2S bitclock in', 'none'],
	29: ['gpio', 'AC97 SDATA_IN0', 'I2S SDATA_IN', 'none'],
	30: ['gpio', 'I2S SDATA_OUT', 'AC97 SDATA_OUT', 'none'],
	31: ['gpio', 'I2S SYNC', 'AC97 SYNC', 'none'],
	32: ['gpio', 'AC97 SDATA_IN1', 'I2S SYSCLK', 'none'],
	33: ['gpio', 'none', 'nCS_5', 'none'],
	34: ['gpio', 'FF RXD', 'MMC CS0', 'none'],
	35: ['gpio', 'FF CTS', 'none', 'none'],
	36: ['gpio', 'FF DCD', 'none', 'none'],
	37: ['gpio', 'FF DSR', 'none', 'none'],
	38: ['gpio', 'FF RI', 'none', 'none'],
	39: ['gpio', 'MMC CS1', 'FF TXD', 'none'],
	40: ['gpio', 'none', 'FF DTR', 'none'],
	41: ['gpio', 'none', 'FF RTS', 'none'],
	42: ['gpio', 'BT RXD', 'none', 'HW RXD'],
	43: ['gpio', 'none', 'BT TXD', 'HW TXD'],
	44: ['gpio', 'BT CTS', 'none', 'HW CTS'],
	45: ['gpio', 'none', 'BT RTS', 'HW RTS'],
	46: ['gpio', 'ICP_RXD', 'STD RXD', 'none'],
	47: ['gpio', 'STD TXD', 'ICP_TXD', 'none'],
	48: ['gpio', 'HW TXD', 'nPOE', 'none'],
	49: ['gpio', 'HW RXD', 'nPWE', 'none'],
	50: ['gpio', 'HW CTS', 'nPIOR', 'none'],
	51: ['gpio', 'nPIOW', 'HW RTS', 'none'],
	52: ['gpio', 'none', 'nPCE[1]', 'none'],
	53: ['gpio', 'MMC CLK', 'nPCE[2]', 'none'],
	54: ['gpio', 'MMC CLK', 'nPSKSEL', 'none'],
	55: ['gpio', 'none', 'nPREG', 'none'],
	56: ['gpio', 'nPWAIT', 'none', 'none'],
	57: ['gpio', 'nIOIS16', 'none', 'none'],
	58: ['gpio', 'none', 'LDD[0]', 'none'],
	59: ['gpio', 'none', 'LDD[1]', 'none'],
	60: ['gpio', 'none', 'LDD[2]', 'none'],
	61: ['gpio', 'none', 'LDD[3]', 'none'],
	62: ['gpio', 'none', 'LDD[4]', 'none'],
	63: ['gpio', 'none', 'LDD[5]', 'none'],
	64: ['gpio', 'none', 'LDD[6]', 'none'],
	65: ['gpio', 'none', 'LDD[7]', 'none'],
	66: ['gpio', 'MBREQ', 'LDD[8]', 'none'],
	67: ['gpio', 'MMC CS0', 'LDD[9]', 'none'],
	68: ['gpio', 'MMC CS1', 'LDD[10]', 'none'],
	69: ['gpio', 'MMC CLK', 'LDD[11]', 'none'],
	70: ['gpio', 'RTC CLK', 'LDD[12]', 'none'],
	71: ['gpio', '3.6 MHz', 'LDD[13]', 'none'],
	72: ['gpio', '32 KHz', 'LDD[14]', 'none'],
	73: ['gpio', 'MBGNT', 'LDD[15]', 'none'],
	74: ['gpio', 'none', 'LCD_FCLK', 'none'],
	75: ['gpio', 'none', 'LCD_LCLK', 'none'],
	76: ['gpio', 'none', 'LCD_PCLK', 'none'],
	77: ['gpio', 'none', 'LCD_ACBIAS', 'none'],
	78: ['gpio', 'none', 'nCS_2', 'none'],
	79: ['gpio', 'none', 'nCS_3', 'none'],
	80: ['gpio', 'none', 'nCS_4', 'none'],
	81: ['gpio', 'NSSPSCLK', 'none', 'none'],
	82: ['gpio', 'NSSPSFRM', 'none', 'none'],
	83: ['gpio', 'NSSPTXD', 'NSSPRXD', 'none'],
	84: ['gpio', 'NSSPTXD', 'NSSPRXD', 'none'],
}


#def __init__(self, dir=0, set=0, clr=0, alt=0, desc=''):

gpio_list = []

for i in range(0,85):
	gpio_list.append(gpio())

#chip select GPIOs
gpio_list[18] = gpio(0, 0, 0, 1, 'RDY')
gpio_list[33] = gpio(1, 1, 0, 2, 'CS5#')
gpio_list[80] = gpio(1, 1, 0, 2, 'CS4#')
gpio_list[79] = gpio(1, 1, 0, 2, 'CS3#')
gpio_list[78] = gpio(1, 1, 0, 2, 'CS2#')
gpio_list[15] = gpio(1, 1, 0, 2, 'CS1#')
gpio_list[22] = gpio(0, 0, 0, 0, 'Consumer IR, PCC_S1_IRQ_O#')
gpio_list[21] = gpio(0, 0, 0, 0, 'IRQ_IDE, PFI')
gpio_list[19] = gpio(0, 0, 0, 0, 'XB_DREQ1, PCC_SO_IRQ_O#')
gpio_list[20] = gpio(0, 0, 0, 0, 'XB_DREQ0')
gpio_list[20] = gpio(0, 0, 0, 0, 'XB_DREQ0')
gpio_list[17] = gpio(0, 0, 0, 0, 'IRQ_AXB')
gpio_list[16] = gpio(1, 0, 0, 2, 'PWM0')

# PCMCIA stuff
gpio_list[57] = gpio(0, 0, 0, 1, 'PCC_IOIS16#')
gpio_list[56] = gpio(0, 0, 0, 1, 'PCC_WAIT#')
gpio_list[55] = gpio(1, 0, 0, 2, 'PCC_REG#')
gpio_list[54] = gpio(1, 0, 0, 2, 'PCC_SCKSEL')
gpio_list[53] = gpio(1, 1, 0, 2, 'PCC_CE2#')
gpio_list[52] = gpio(1, 1, 0, 2, 'PCC_CE1#')
gpio_list[51] = gpio(1, 1, 0, 1, 'PCC_IOW#')
gpio_list[50] = gpio(1, 1, 0, 2, 'PCC_IOR#')
gpio_list[49] = gpio(1, 1, 0, 2, 'PCC_WE#')
gpio_list[48] = gpio(1, 1, 0, 2, 'PCC_OE#')

# SSP port
gpio_list[26] = gpio(0, 0, 0, 1, 'SSP_RXD')
gpio_list[25] = gpio(0, 0, 0, 0, 'SSP_TXD')
gpio_list[24] = gpio(1, 0, 1, 2, 'SSP_SFRM')
gpio_list[23] = gpio(1, 0, 1, 2, 'SSP_SCLK')
gpio_list[27] = gpio(0, 0, 0, 0, 'SSP_EXTCLK')

# audio codec
gpio_list[32] = gpio(0, 0, 0, 0, 'AUD_SDIN1')
gpio_list[31] = gpio(1, 0, 0, 2, 'AC_SYNC')
gpio_list[30] = gpio(1, 0, 0, 2, 'AC_SDOUT')
gpio_list[29] = gpio(0, 0, 0, 1, 'AUD_SDIN0')
gpio_list[28] = gpio(0, 0, 0, 1, 'AC_BITCLK')

# serial ports
gpio_list[39] = gpio(1, 0, 0, 2, 'FF_TXD')
gpio_list[34] = gpio(0, 0, 0, 1, 'FF_RXD')
gpio_list[41] = gpio(1, 0, 0, 2, 'FF_RTS')
gpio_list[35] = gpio(0, 0, 0, 1, 'FF_CTS')
gpio_list[40] = gpio(1, 0, 0, 2, 'FF_DTR')
gpio_list[37] = gpio(0, 0, 0, 1, 'FF_DSR')
gpio_list[38] = gpio(0, 0, 0, 1, 'FF_RI')
gpio_list[36] = gpio(0, 0, 0, 1, 'FF_DCD')

gpio_list[43] = gpio(1, 0, 0, 2, 'BT_TXD')
gpio_list[42] = gpio(0, 0, 0, 1, 'BT_RXD')
gpio_list[45] = gpio(1, 0, 0, 2, 'BT_RTS')
gpio_list[44] = gpio(0, 0, 0, 1, 'BT_CTS')

gpio_list[47] = gpio(1, 0, 0, 1, 'IR_TXD')
gpio_list[46] = gpio(0, 0, 0, 2, 'IR_RXD')

# misc GPIO signals
gpio_list[14] = gpio(0, 0, 0, 0, 'MBREQ')
gpio_list[13] = gpio(0, 0, 0, 0, 'MBGNT')
gpio_list[12] = gpio(0, 0, 0, 0, 'GPIO_12/32K_CLK')
gpio_list[11] = gpio(0, 0, 0, 0, '3M6_CLK')
gpio_list[10] = gpio(1, 0, 1, 0, 'GPIO_10/RTC_CLK/debug LED')
gpio_list[9] = gpio(0, 0, 0, 0, 'MMC_CD#')
gpio_list[8] = gpio(0, 0, 0, 0, 'PCC_S1_CD#')
gpio_list[7] = gpio(0, 0, 0, 0, 'PCC_S0_CD#')
gpio_list[6] = gpio(1, 0, 0, 1, 'MMC_CLK')
gpio_list[5] = gpio(0, 0, 0, 0, 'IRQ_TOUCH#')
gpio_list[4] = gpio(0, 0, 0, 0, 'IRQ_ETH')
gpio_list[3] = gpio(0, 0, 0, 0, 'MQ_IRQ#')
gpio_list[2] = gpio(0, 0, 0, 0, 'BAT_DATA')
gpio_list[1] = gpio(0, 0, 0, 1, 'USER_RESET#')
gpio_list[0] = gpio(0, 0, 0, 1, 'USER_RESET#')

# LCD GPIOs
gpio_list[58] = gpio(1, 0, 0, 2, 'LDD0')
gpio_list[59] = gpio(1, 0, 0, 2, 'LDD1')
gpio_list[60] = gpio(1, 0, 0, 2, 'LDD2')
gpio_list[61] = gpio(1, 0, 0, 2, 'LDD3')
gpio_list[62] = gpio(1, 0, 0, 2, 'LDD4')
gpio_list[63] = gpio(1, 0, 0, 2, 'LDD5')
gpio_list[64] = gpio(1, 0, 0, 2, 'LDD6')
gpio_list[65] = gpio(1, 0, 0, 2, 'LDD7')
gpio_list[66] = gpio(1, 0, 0, 2, 'LDD8')
gpio_list[67] = gpio(1, 0, 0, 2, 'LDD9')
gpio_list[68] = gpio(1, 0, 0, 2, 'LDD10')
gpio_list[69] = gpio(1, 0, 0, 2, 'LDD11')
gpio_list[70] = gpio(1, 0, 0, 2, 'LDD12')
gpio_list[71] = gpio(1, 0, 0, 2, 'LDD13')
gpio_list[72] = gpio(1, 0, 0, 2, 'LDD14')
gpio_list[73] = gpio(1, 0, 0, 2, 'LDD15')
gpio_list[74] = gpio(1, 0, 0, 2, 'FCLK')
gpio_list[75] = gpio(1, 0, 0, 2, 'LCLK')
gpio_list[76] = gpio(1, 0, 0, 2, 'PCLK')
gpio_list[77] = gpio(1, 0, 0, 2, 'ACBIAS')

# calculate registers
pxa_regs = {
	'gpdr0':0, 'gpdr1':0, 'gpdr2':0,
	'gpsr0':0, 'gpsr1':0, 'gpsr2':0,
	'gpcr0':0, 'gpcr1':0, 'gpcr2':0,
	'gafr0_l':0, 'gafr0_u':0,
	'gafr1_l':0, 'gafr1_u':0,
	'gafr2_l':0, 'gafr2_u':0,
}

# U-boot define names
uboot_reg_names = {
	'gpdr0':'CFG_GPDR0_VAL', 'gpdr1':'CFG_GPDR1_VAL', 'gpdr2':'CFG_GPDR2_VAL',
	'gpsr0':'CFG_GPSR0_VAL', 'gpsr1':'CFG_GPSR1_VAL', 'gpsr2':'CFG_GPSR2_VAL',
	'gpcr0':'CFG_GPCR0_VAL', 'gpcr1':'CFG_GPCR1_VAL', 'gpcr2':'CFG_GPCR2_VAL',
	'gafr0_l':'CFG_GAFR0_L_VAL', 'gafr0_u':'CFG_GAFR0_U_VAL',
	'gafr1_l':'CFG_GAFR1_L_VAL', 'gafr1_u':'CFG_GAFR1_U_VAL',
	'gafr2_l':'CFG_GAFR2_L_VAL', 'gafr2_u':'CFG_GAFR2_U_VAL',
}

# bit mappings

bit_mappings = [

{ 'gpio':(0,32),  'shift':1, 'regs':{'dir':'gpdr0', 'set':'gpsr0', 'clr':'gpcr0'} },
{ 'gpio':(32,64), 'shift':1, 'regs':{'dir':'gpdr1', 'set':'gpsr1', 'clr':'gpcr1'} },
{ 'gpio':(64,85), 'shift':1, 'regs':{'dir':'gpdr2', 'set':'gpsr2', 'clr':'gpcr2'} },
{ 'gpio':(0,16),  'shift':2, 'regs':{'alt':'gafr0_l'} },
{ 'gpio':(16,32), 'shift':2, 'regs':{'alt':'gafr0_u'} },
{ 'gpio':(32,48), 'shift':2, 'regs':{'alt':'gafr1_l'} },
{ 'gpio':(48,64), 'shift':2, 'regs':{'alt':'gafr1_u'} },
{ 'gpio':(64,80), 'shift':2, 'regs':{'alt':'gafr2_l'} },
{ 'gpio':(80,85), 'shift':2, 'regs':{'alt':'gafr2_u'} },

]

def stuff_bits(bit_mapping, gpio_list):
	gpios = range( bit_mapping['gpio'][0], bit_mapping['gpio'][1])

	for gpio in gpios:
		for reg in bit_mapping['regs'].keys():
			value = eval( 'gpio_list[gpio].%s' % (reg) )
			if ( value ):
				# we have a high bit
				bit_shift = (gpio - bit_mapping['gpio'][0]) * bit_mapping['shift']
				bit = value << (bit_shift)
				pxa_regs[bit_mapping['regs'][reg]] |= bit

for i in bit_mappings:
	stuff_bits(i, gpio_list)

# now print out all regs
registers = pxa_regs.keys()
registers.sort()
for reg in registers:
	print '%s: 0x%x' % (reg, pxa_regs[reg])

# print define to past right into U-Boot source code

print 
print 

for reg in registers:
	print '#define %s	0x%x' % (uboot_reg_names[reg], pxa_regs[reg])

# print all GPIOS
print
print

for i in range(len(gpio_list)):
	gpio_i = gpio_list[i]
	alt_func_desc = pxa255_alt_func[i][gpio_i.alt]
	print 'GPIO: %i, dir=%i, set=%i, clr=%i, alt=%s, desc=%s' % (i, gpio_i.dir, gpio_i.set, gpio_i.clr, alt_func_desc, gpio_i.desc)


