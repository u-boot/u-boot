#!/usr/bin/env python2
# -*- coding: utf-8 -*-
# Copyright (C) 2014, Xilinx.inc.
#
# Hack origin version and just take the part which generate boot.bin
# for U-BOOT SPL.
#
# Copyright (C) 2013, Elphel.inc.
# pre-u-boot configuration of the Xilinx Zynq(R) SoC
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

__author__ = "Andrey Filippov"
__copyright__ = "Copyright 2013, Elphel, Inc."
__license__ = "GPL"
__version__ = "3.0+"
__maintainer__ = "Andrey Filippov"
__email__ = "andrey@elphel.com"
__status__ = "Development"
import os
import struct
import sys, getopt

inputfile = ''
outputfile = ''
argv = sys.argv[1:]
try:
  opts, args = getopt.getopt(argv,"hu:o:",["uboot=","outfile="])
except getopt.GetoptError:
  print 'test.py -u <inputfile> -o <outputfile>'
  sys.exit(2)

if len(argv) == 0:
  print 'test.py -u <inputfile> -o <outputfile>'
  sys.exit()

for opt, arg in opts:
  if opt == '-h':
      print 'test.py -u <inputfile> -o <outputfile>'
      sys.exit()
  elif opt in ("-u", "--uboot"):
      inputfile = arg
  elif opt in ("-o", "--outfile"):
      outputfile = arg
print 'Input file is:', inputfile
print 'Output file is:', outputfile

exit

ACCESSIBLE_REGISTERS=((0xe0001000,0xe0001fff), # UART1 controller registers
                      (0xe000d000,0xe000efff), # QUAD SPI controller registers
                      (0xe0100004,0xe0100057), # SDIO 0 controller registers
                      (0xe0100059,0xe0100fff), # SDIO 0 controller registers
                      (0xe000e000,0xe000efff), # SMC controller
                      (0xf8006000,0xf8006fff), # DDR controller
                      # SLCR_LOCK disables all (0xf8000000,0xf8000b74), but it is locked at reset seems to be unlocked, http://www.xilinx.com/support/answers/47570.html
                      #prohibited: SLCR_SCL, SLCR_LOCK, SLCR_UNLOCK, SLCR_STA
                      (0xf8000100,0xf80001b0), # SLCR registers
                      #DOes not seem to be any gap between 0xf80001b0 and 0xf80001b4
                      (0xf80001b4,0xf80001ff), # SLCR registers
                      #prohibited SLCR_PSS_RST_CTRL 0xf8000200
                      (0xf8000204,0xf8000234), # SLCR registers - is  SLCR_SMC_RST_CTRL 0xf8000234 also prohibited?
                      #prohibited? SLCR_OCM_RST_CTRL 0xf8000238 SLCR_FPGA_RST_CTRL 0xf8000240
                      (0xf800024c,0xf800024c), # SLCR registers SLCR_AWDT_CTRL - watchdog timer reset control
                      #prohibited SLSR_REBOOT_STATUS 0xf8000258, SLCR_BOOT_MODE 0xf800025c, SLCR_APU_CTRL 0xf8000300,
                      (0xf8000304,0xf8000834), # SLCR registers SLCR_AWDT_CLK_SEL,  DDR, MIO
                      #prohibited SLCR_LVL_SHFTR_ON 0xf8000900, SLCR_OCM_CFG 0xf8000910,
                      (0xf8000a00,0xf8000a8c), # SLCR registers All shown "reserved" ???
                      (0xf8000ab0,0xf8000b74)) # SLCR registers iostd, voltages,  - more DDR stuff

def verify_register_accessible(address):
    for interval in ACCESSIBLE_REGISTERS:
        if (address >= interval[0]) and (address <= interval[1]):
            print 'Register accessible:' , hex(interval[0]),'<=', hex(address), '<=', hex(interval[1])
            return True
    else:
        return False

def image_generator (image,
                       reg_sets, # registers,
                       options,
                       user_def,
                       ocm_offset,
                       ocm_len,
                       start_exec):
    reserved0044=0;

    rfi_word=0xeafffffe #from actual image
    waddr=0
    for _ in range (0x20/4):
        image[waddr]=rfi_word # fill reserved for interrupts fields
        waddr+=1
    #width detection
    image[waddr]=0xaa995566 # offset 0x20
    waddr+=1

    #image identification
    image[waddr]=0x584c4e58 # offset 0x24, XLNX
    waddr+=1

    #encryption status
    image[waddr]=0x0 # offset 0x28, no encryption
    waddr+=1

    #User defined word
    image[waddr]=user_def # offset 0x2c
    waddr+=1

    #ocm_offset
    if ocm_offset<0x8c0:
        print 'Start offset should be >= 0x8c0, specified', hex(ocm_offset)
        exit (ERROR_DEFS['HEAD'])
    elif (ocm_offset & 0x3f) != 0:
        print 'Start offset should be 64-bytes aligned, specified', hex(ocm_offset)
        exit (ERROR_DEFS['HEAD'])
    image[waddr]=ocm_offset # offset 0x30
    waddr+=1

    #ocm_len
    if ocm_len>0x30000:
        print 'Loaded to the OCM image should fit into 3 mapped pages of OCM - 192K (0x30000), specified ',hex(ocm_len)
        exit (ERROR_DEFS['HEAD'])
    image[waddr]=ocm_len # offset 0x34
    waddr+=1

    #reserved 0
    image[waddr]=0 # offset 0x38
    waddr+=1

    #start_exec
    if (start_exec>0x30000) or (start_exec<0):
        print 'Start address is relative to  OCM and should fit there - in 192K (0x30000), specified ',hex(start_exec)
        exit (ERROR_DEFS['HEAD'])
    image[waddr]=start_exec # offset 0x3c
    waddr+=1

    #img_len == ocm_len for unsecure images
    img_len = ocm_len
    image[waddr]=img_len # offset 0x40
    waddr+=1

    #reserved 0
    image[waddr]=reserved0044 #0  # offset 0x44
    waddr+=1

    #calculate image checksum
    def add (x,y): return x+y
    checksum=(reduce(add,image[0x20/4:0x48/4]) ^ 0xffffffff) & 0xffffffff
    image[waddr]=checksum # offset 0x48
    waddr+=1
    print 'After checksum waddr=',hex(waddr),' byte addr=',hex(4*waddr)


    #initialize registers
    print 'Number of registers to initialize',len(reg_sets)
    if len (reg_sets)>256:
        print 'Too many registers to initialize, only 256 allowed,',len(reg_sets),'> 256'
    waddr=0xa0/4
    # new_sets.append((addr,data,mask,self.module_name,register_name,self.defs[register_name]))

    for register in reg_sets:
        op=register[0]
        addr=register[1]
        data=register[2]
        if (op != 's'):
            raise Exception ('Can not test registers (0x%08x) in RBL, it should be done in user code'%addr)
        if not verify_register_accessible (addr):
            print 'Tried to set non-accessible register', hex(addr),' with data ', hex(data)
            exit (ERROR_DEFS['NONACCESSIBLE_REGISTER'])
        image[waddr]=addr
        waddr+=1
        image[waddr]=data
        waddr+=1
    #Fill in FFs for unused registers
    while waddr < (0x8c0/4):
        image[waddr]=0xffffffff
        waddr+=1
        image[waddr]=0
        waddr+=1

if (inputfile):
    try:
        uboot_image_len=os.path.getsize(inputfile)
        print 'Using %s to get image length - it is %i (0x%x) bytes'%(os.path.abspath(inputfile),uboot_image_len,uboot_image_len)
    except:
        print 'Specified u-boot.bin file: %s (%s) not found'%(inputfile,os.path.abspath(inputfile))
        sys.exit()
else:
    uboot_image_len=int(raw_options['CONFIG_EZYNQ_BOOT_OCM_IMAGE_LENGTH'],0)
    print 'No u-boot.bin path specified, using provided CONFIG_EZYNQ_BOOT_OCM_IMAGE_LENGTH as image size of %i (0x%x) bytes for the RBL header'%(uboot_image_len,uboot_image_len)

image =[ 0 for k in range (0x8c0/4)]
reg_sets=[]
num_rbl_regs=0

raw_configs=""
raw_options={}


image_generator (image,
                 reg_sets[:num_rbl_regs], #
                 #registers,
                 raw_options,
                 0x1010000, # user_def
                 0x8c0, # ocm_offset,
                 uboot_image_len, #ocm_len,
                 0) #start_exec)

if outputfile:
    print 'Generating binary output ',os.path.abspath(outputfile)
    bf=open(outputfile,'wb')
    data=struct.pack('I' * len(image), *image)
    bf.write(data)

    spl=open(inputfile,'rb')
    bf.write(spl.read())

    bf.close()
    spl.close()
