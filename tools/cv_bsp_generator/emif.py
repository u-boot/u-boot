# SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
"""
SDRAM header file generator

Process the handoff files from Quartus and convert them to headers
usable by U-Boot.

Copyright (C) 2022 Intel Corporation <www.intel.com>

Author: Lee, Kah Jing <kah.jing.lee@intel.com>
"""

import os
import re
import xml.dom.minidom
import streamer
import xmlgrok

class EMIFGrokker(object):
    """ parse an emif.xml input and translate to various
    outputs
    """
    SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
    TEMPLATE_DIR = os.path.dirname(SCRIPT_DIR) + '/src'
    SDRAM_FILE_HEADER = '/*\n' + ' * Altera SoCFPGA SDRAM configuration\n' + ' *\n' + ' */\n\n'
    SDRAM_SENTINEL = '__SOCFPGA_SDRAM_CONFIG_H__'
    SDRAM_MATCH = r'#define (CFG_HPS_SDR_CTRLCFG_CTRLCFG_MEMTYPE|CFG_HPS_SDR_CTRLCFG_CTRLCFG_MEMBL|CFG_HPS_SDR_CTRLCFG_CTRLCFG_ADDRORDER|CFG_HPS_SDR_CTRLCFG_CTRLCFG_ECCEN|CFG_HPS_SDR_CTRLCFG_CTRLCFG_ECCCORREN|CFG_HPS_SDR_CTRLCFG_CTRLCFG_REORDEREN|CFG_HPS_SDR_CTRLCFG_CTRLCFG_STARVELIMIT|CFG_HPS_SDR_CTRLCFG_CTRLCFG_DQSTRKEN|CFG_HPS_SDR_CTRLCFG_CTRLCFG_NODMPINS|CFG_HPS_SDR_CTRLCFG_DRAMTIMING1_TCWL|CFG_HPS_SDR_CTRLCFG_DRAMTIMING1_AL|CFG_HPS_SDR_CTRLCFG_DRAMTIMING1_TCL|CFG_HPS_SDR_CTRLCFG_DRAMTIMING1_TRRD|CFG_HPS_SDR_CTRLCFG_DRAMTIMING1_TFAW|CFG_HPS_SDR_CTRLCFG_DRAMTIMING1_TRFC|CFG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TREFI|CFG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TRCD|CFG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TRP|CFG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TWR|CFG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TWTR|CFG_HPS_SDR_CTRLCFG_DRAMTIMING3_TRTP|CFG_HPS_SDR_CTRLCFG_DRAMTIMING3_TRAS|CFG_HPS_SDR_CTRLCFG_DRAMTIMING3_TRC|CFG_HPS_SDR_CTRLCFG_DRAMTIMING3_TMRD|CFG_HPS_SDR_CTRLCFG_DRAMTIMING3_TCCD|CFG_HPS_SDR_CTRLCFG_DRAMTIMING4_SELFRFSHEXIT|CFG_HPS_SDR_CTRLCFG_DRAMTIMING4_PWRDOWNEXIT|CFG_HPS_SDR_CTRLCFG_LOWPWRTIMING_AUTOPDCYCLES|CFG_HPS_SDR_CTRLCFG_LOWPWRTIMING_CLKDISABLECYCLES|CFG_HPS_SDR_CTRLCFG_DRAMODT_READ|CFG_HPS_SDR_CTRLCFG_DRAMODT_WRITE|CFG_HPS_SDR_CTRLCFG_DRAMADDRW_COLBITS|CFG_HPS_SDR_CTRLCFG_DRAMADDRW_ROWBITS|CFG_HPS_SDR_CTRLCFG_DRAMADDRW_BANKBITS|CFG_HPS_SDR_CTRLCFG_DRAMADDRW_CSBITS|CFG_HPS_SDR_CTRLCFG_DRAMIFWIDTH_IFWIDTH|CFG_HPS_SDR_CTRLCFG_DRAMDEVWIDTH_DEVWIDTH|CFG_HPS_SDR_CTRLCFG_DRAMINTR_INTREN|CFG_HPS_SDR_CTRLCFG_LOWPWREQ_SELFRFSHMASK|CFG_HPS_SDR_CTRLCFG_STATICCFG_MEMBL|CFG_HPS_SDR_CTRLCFG_STATICCFG_USEECCASDATA|CFG_HPS_SDR_CTRLCFG_CTRLWIDTH_CTRLWIDTH|CFG_HPS_SDR_CTRLCFG_CPORTWIDTH_CPORTWIDTH|CFG_HPS_SDR_CTRLCFG_CPORTWMAP_CPORTWMAP|CFG_HPS_SDR_CTRLCFG_CPORTRMAP_CPORTRMAP|CFG_HPS_SDR_CTRLCFG_RFIFOCMAP_RFIFOCMAP|CFG_HPS_SDR_CTRLCFG_WFIFOCMAP_WFIFOCMAP|CFG_HPS_SDR_CTRLCFG_CPORTRDWR_CPORTRDWR|CFG_HPS_SDR_CTRLCFG_PORTCFG_AUTOPCHEN|CFG_HPS_SDR_CTRLCFG_FPGAPORTRST|CFG_HPS_SDR_CTRLCFG_FIFOCFG_SYNCMODE|CFG_HPS_SDR_CTRLCFG_FIFOCFG_INCSYNC|CFG_HPS_SDR_CTRLCFG_MPPRIORITY_USERPRIORITY|CFG_HPS_SDR_CTRLCFG_MPWIEIGHT_0_STATICWEIGHT_31_0|CFG_HPS_SDR_CTRLCFG_MPWIEIGHT_1_STATICWEIGHT_49_32|CFG_HPS_SDR_CTRLCFG_MPWIEIGHT_1_SUMOFWEIGHT_13_0|CFG_HPS_SDR_CTRLCFG_MPWIEIGHT_2_SUMOFWEIGHT_45_14|CFG_HPS_SDR_CTRLCFG_MPWIEIGHT_3_SUMOFWEIGHT_63_46|CFG_HPS_SDR_CTRLCFG_PHYCTRL_PHYCTRL_0|CFG_HPS_SDR_CTRLCFG_MPPACING_0_THRESHOLD1_31_0|CFG_HPS_SDR_CTRLCFG_MPPACING_1_THRESHOLD1_59_32|CFG_HPS_SDR_CTRLCFG_MPPACING_1_THRESHOLD2_3_0|CFG_HPS_SDR_CTRLCFG_MPPACING_2_THRESHOLD2_35_4|CFG_HPS_SDR_CTRLCFG_MPPACING_3_THRESHOLD2_59_36|CFG_HPS_SDR_CTRLCFG_MPTHRESHOLDRST_0_THRESHOLDRSTCYCLES_31_0|CFG_HPS_SDR_CTRLCFG_MPTHRESHOLDRST_1_THRESHOLDRSTCYCLES_63_32|CFG_HPS_SDR_CTRLCFG_MPTHRESHOLDRST_2_THRESHOLDRSTCYCLES_79_64|RW_MGR_ACTIVATE_0_AND_1|RW_MGR_ACTIVATE_0_AND_1_WAIT1|RW_MGR_ACTIVATE_0_AND_1_WAIT2|RW_MGR_ACTIVATE_1|RW_MGR_CLEAR_DQS_ENABLE|RW_MGR_EMR_OCD_ENABLE|RW_MGR_EMR|RW_MGR_EMR2|RW_MGR_EMR3|RW_MGR_GUARANTEED_READ|RW_MGR_GUARANTEED_READ_CONT|RW_MGR_GUARANTEED_WRITE|RW_MGR_GUARANTEED_WRITE_WAIT0|RW_MGR_GUARANTEED_WRITE_WAIT1|RW_MGR_GUARANTEED_WRITE_WAIT2|RW_MGR_GUARANTEED_WRITE_WAIT3|RW_MGR_IDLE|RW_MGR_IDLE_LOOP1|RW_MGR_IDLE_LOOP2|RW_MGR_INIT_RESET_0_CKE_0|RW_MGR_INIT_RESET_1_CKE_0|RW_MGR_INIT_CKE_0|RW_MGR_LFSR_WR_RD_BANK_0|RW_MGR_LFSR_WR_RD_BANK_0_DATA|RW_MGR_LFSR_WR_RD_BANK_0_DQS|RW_MGR_LFSR_WR_RD_BANK_0_NOP|RW_MGR_LFSR_WR_RD_BANK_0_WAIT|RW_MGR_LFSR_WR_RD_BANK_0_WL_1|RW_MGR_LFSR_WR_RD_DM_BANK_0|RW_MGR_LFSR_WR_RD_DM_BANK_0_DATA|RW_MGR_LFSR_WR_RD_DM_BANK_0_DQS|RW_MGR_LFSR_WR_RD_DM_BANK_0_NOP|RW_MGR_LFSR_WR_RD_DM_BANK_0_WAIT|RW_MGR_LFSR_WR_RD_DM_BANK_0_WL_1|RW_MGR_MR_CALIB|RW_MGR_MR_USER|RW_MGR_MR_DLL_RESET|RW_MGR_MRS0_DLL_RESET|RW_MGR_MRS0_DLL_RESET_MIRR|RW_MGR_MRS0_USER|RW_MGR_MRS0_USER_MIRR|RW_MGR_MRS1|RW_MGR_MRS1_MIRR|RW_MGR_MRS2|RW_MGR_MRS2_MIRR|RW_MGR_MRS3|RW_MGR_MRS3_MIRR|RW_MGR_NOP|RW_MGR_PRECHARGE_ALL|RW_MGR_READ_B2B|RW_MGR_READ_B2B_WAIT1|RW_MGR_READ_B2B_WAIT2|RW_MGR_REFRESH|RW_MGR_REFRESH_ALL|RW_MGR_RETURN|RW_MGR_SGLE_READ|RW_MGR_ZQCL|RW_MGR_TRUE_MEM_DATA_MASK_WIDTH|RW_MGR_MEM_ADDRESS_MIRRORING|RW_MGR_MEM_DATA_MASK_WIDTH|RW_MGR_MEM_DATA_WIDTH|RW_MGR_MEM_DQ_PER_READ_DQS|RW_MGR_MEM_DQ_PER_WRITE_DQS|RW_MGR_MEM_IF_READ_DQS_WIDTH|RW_MGR_MEM_IF_WRITE_DQS_WIDTH|RW_MGR_MEM_NUMBER_OF_CS_PER_DIMM|RW_MGR_MEM_NUMBER_OF_RANKS|RW_MGR_MEM_VIRTUAL_GROUPS_PER_READ_DQS|RW_MGR_MEM_VIRTUAL_GROUPS_PER_WRITE_DQS|IO_DELAY_PER_DCHAIN_TAP|IO_DELAY_PER_DQS_EN_DCHAIN_TAP|IO_DELAY_PER_OPA_TAP|IO_DLL_CHAIN_LENGTH|IO_DQDQS_OUT_PHASE_MAX|IO_DQS_EN_DELAY_MAX|IO_DQS_EN_DELAY_OFFSET|IO_DQS_EN_PHASE_MAX|IO_DQS_IN_DELAY_MAX|IO_DQS_IN_RESERVE|IO_DQS_OUT_RESERVE|IO_IO_IN_DELAY_MAX|IO_IO_OUT1_DELAY_MAX|IO_IO_OUT2_DELAY_MAX|IO_SHIFT_DQS_EN_WHEN_SHIFT_DQS|AFI_RATE_RATIO|AFI_CLK_FREQ|CALIB_LFIFO_OFFSET|CALIB_VFIFO_OFFSET|ENABLE_SUPER_QUICK_CALIBRATION|MAX_LATENCY_COUNT_WIDTH|READ_VALID_FIFO_SIZE|REG_FILE_INIT_SEQ_SIGNATURE|TINIT_CNTR0_VAL|TINIT_CNTR1_VAL|TINIT_CNTR2_VAL|TRESET_CNTR0_VAL|TRESET_CNTR1_VAL|TRESET_CNTR2_VAL|CFG_HPS_SDR_CTRLCFG_EXTRATIME1_CFG_EXTRA_CTL_CLK_RD_TO_WR|CFG_HPS_SDR_CTRLCFG_EXTRATIME1_CFG_EXTRA_CTL_CLK_RD_TO_WR_BC|CFG_HPS_SDR_CTRLCFG_EXTRATIME1_CFG_EXTRA_CTL_CLK_RD_TO_WR_DIFF_CHIP)\s+'

    SDRAM_CONFIG_H_FILENAME = "sdram_config.h"

    sdramHTemplate = ""
    seqAutoTemplate = ""
    seqDefinesTemplate = ""
    seqAutoAcTemplate = ""
    seqAutoInstTemplate = ""
    seqAutoTemplateList = []
    seqDefinesTemplateList = []
    seqAutoAcTemplateList = []
    seqAutoInstTemplateList = []

    def __init__(self, inputDir, outputDir, emifFileName='emif.xml', hpsFileName='hps.xml'):
        """ EMIFGrokker initialization """
        self.inputDir = inputDir
        self.outputDir = outputDir

        sdramDir = self.outputDir
        if not os.path.isdir(sdramDir):
            os.makedirs(sdramDir)

        self.emifFileName = inputDir + os.sep + emifFileName
        self.hpsFileName = inputDir + os.sep + hpsFileName
        self.emifDom = xml.dom.minidom.parse(self.emifFileName)
        self.hpsDom = xml.dom.minidom.parse(self.hpsFileName)
        self.sequencerDefinesStream = None
        self.seqAutoFileName = inputDir + os.sep +  "sequencer_auto.h"
        self.seqDefinesFileName = inputDir + os.sep + "sequencer_defines.h"
        self.seqAutoACFileName = inputDir + os.sep + "sequencer_auto_ac_init.c"
        self.seqAutoInstFileName = inputDir + os.sep + "sequencer_auto_inst_init.c"

        self.createFilesFromEMIF()

    def openSeqFiles(self):
        """ files to retrieve values to written to sdram_config.h """
        self.seq_auto_fd = open(self.seqAutoFileName, "r")
        self.seq_defines_fd = open(self.seqDefinesFileName, "r")
        self.seq_auto_ac_fd = open(self.seqAutoACFileName, "r")
        self.seq_auto_inst_fd = open(self.seqAutoInstFileName, "r")

    def closeSeqFiles(self):
        """ close files """
        self.seq_auto_fd.close()
        self.seq_defines_fd.close()
        self.seq_auto_ac_fd.close()
        self.seq_auto_inst_fd.close()

    def processSeqAuto(self):
        """ process sequencer files to retrieve variable. Regex match is from
        qts-filter.sh
        """
        # replace underscore & bracket in sequencer_auto.h define
        for line in self.seq_auto_fd.readlines():
            if re.match(".*__RW_MGR_", line) and not re.match(".*ac_", line) and not re.match(".*CONTENT_", line):
                line = re.sub("__RW_MGR", "RW_MGR", line)
                if re.match(self.SDRAM_MATCH, line):
                    self.seqAutoTemplateList.append(re.sub(r' (\w+)(\s+)(\d+)', r' \1\t\3', line))
        self.seqAutoTemplateList.sort()
        self.seqAutoTemplate = ''.join([item for item in self.seqAutoTemplateList])

        # replace underscore & bracket in sequencer_defines.h define
        for line in self.seq_defines_fd.readlines():
            if re.match("^#define (\w+_)", line):
                line = re.sub("__", "", line)
                if re.match(self.SDRAM_MATCH, line):
                    self.seqDefinesTemplateList.append(re.sub(r' (\w+)(\s+)(\d+)', r' \1\t\3', line))
        self.seqDefinesTemplateList.sort()
        self.seqDefinesTemplate = ''.join([item for item in self.seqDefinesTemplateList])

        arrayMatchStart = 0
        # replace const variable declaration in sequencer_auto_ac_init.c
        for line in self.seq_auto_ac_fd.readlines():
            if re.match("^const.*\[", line) or arrayMatchStart:
                if arrayMatchStart == 0:
                    line = line.strip() + " "
                arrayMatchStart = 1
                if re.match("};", line):
                    arrayMatchStart = 0
                    self.seqAutoAcTemplateList.append("};")
                    continue
                line = re.sub("alt_u32", "u32", line)
                self.seqAutoAcTemplateList.append(re.sub("\[.*\]", "[]", line))
        self.seqAutoAcTemplate = ''.join([item for item in self.seqAutoAcTemplateList])

        arrayMatchStart = 0
        # replace const variable declaration in sequencer_auto_inst_init.c
        for line in self.seq_auto_inst_fd.readlines():
            if re.match("^const.*\[", line) or arrayMatchStart:
                if arrayMatchStart == 0:
                    line = line.strip() + " "
                arrayMatchStart = 1
                if re.match("};", line):
                    arrayMatchStart = 0
                    self.seqAutoInstTemplateList.append("};")
                    continue
                line = re.sub("alt_u32", "u32", line)
                self.seqAutoInstTemplateList.append(re.sub("\[.*\]", "[]", line))
        self.seqAutoInstTemplate = ''.join([item for item in self.seqAutoInstTemplateList])

    def handleSettingNode(self, settingNode):
        """ create define string from variable name and value """
        if settingNode.hasAttribute('name') and settingNode.hasAttribute('value'):
            name = settingNode.getAttribute('name')
            value = settingNode.getAttribute('value')
            self.sequencerDefinesStream.write("#define " + name + ' ' + '(' + value + ')' + '\n')

    def updateTemplate(self, name, value):
        """ update sdram template """
        pattern = "${" + name + "}"
        self.sdramHTemplate = self.sdramHTemplate.replace(pattern, value)

    def handleEMIFControllerNode(self, node):
        """ retrieve values from emif.xml for controller node """
        derivedNoDmPins = 0
        derivedCtrlWidth = 0
        derivedEccEn = 0
        derivedEccCorrEn = 0

        self.mem_if_rd_to_wr_turnaround_oct = 0

        node = xmlgrok.firstElementChild(node)
        while node != None:
            name = node.getAttribute('name')
            value = node.getAttribute('value')

            if value == "true":
                value = "1"
            elif value == "false":
                value = "0"

            self.updateTemplate(name, value)

            if name == "MEM_IF_DM_PINS_EN":
                if value == "1":
                    derivedNoDmPins = 0
                else:
                    derivedNoDmPins = 1

            if name == "MEM_DQ_WIDTH":
                if value == "8":
                    derivedCtrlWidth = 0
                    derivedEccEn = 0
                    derivedEccCorrEn = 0
                elif value == "16":
                    derivedCtrlWidth = 1
                    derivedEccEn = 0
                    derivedEccCorrEn = 0
                elif value == "24":
                    derivedCtrlWidth = 1
                    derivedEccEn = 1
                    derivedEccCorrEn = 1
                elif value == "32":
                    derivedCtrlWidth = 2
                    derivedEccEn = 0
                    derivedEccCorrEn = 0
                elif value == "40":
                    derivedCtrlWidth = 2
                    derivedEccEn = 1
                    derivedEccCorrEn = 1

            if name == "MEM_IF_RD_TO_WR_TURNAROUND_OCT":
                self.mem_if_rd_to_wr_turnaround_oct = int(value)

            node = xmlgrok.nextElementSibling(node)

        self.updateTemplate("DERIVED_NODMPINS", str(derivedNoDmPins))
        self.updateTemplate("DERIVED_CTRLWIDTH", str(derivedCtrlWidth))
        self.updateTemplate("DERIVED_ECCEN", str(derivedEccEn))
        self.updateTemplate("DERIVED_ECCCORREN", str(derivedEccCorrEn))

    def handleEMIFPllNode(self, node):
        """ retrieve values for pll node """
        node = xmlgrok.firstElementChild(node)
        while node != None:
            name = node.getAttribute('name')
            value = node.getAttribute('value')

            self.updateTemplate(name, value)

            node = xmlgrok.nextElementSibling(node)

    def handleEMIFSequencerNode(self, node):
        """ retrieve values for sequencer node """
        derivedMemtype = 0
        derivedSelfrfshexit = 0

        self.afi_rate_ratio = 0

        node = xmlgrok.firstElementChild(node)
        while node != None:
            name = node.getAttribute('name')
            value = node.getAttribute('value')

            self.updateTemplate(name, value)

            if value.isdigit():
                intValue = int(value)
            else:
                intValue = 0

            if name == "DDR2" and intValue != 0:
                derivedMemtype = 1
                derivedSelfrfshexit = 200
            elif name == "DDR3" and intValue != 0:
                derivedMemtype = 2
                derivedSelfrfshexit = 512
            elif name == "LPDDR1" and intValue != 0:
                derivedMemtype = 3
                derivedSelfrfshexit = 200
            elif name == "LPDDR2" and intValue != 0:
                derivedMemtype = 4
                derivedSelfrfshexit = 200
            elif name == "AFI_RATE_RATIO" and intValue != 0:
                self.afi_rate_ratio = intValue

            node = xmlgrok.nextElementSibling(node)

        self.updateTemplate("DERIVED_MEMTYPE", str(derivedMemtype))
        self.updateTemplate("DERIVED_SELFRFSHEXIT", str(derivedSelfrfshexit))


    def handleHpsFpgaInterfaces(self, node):
        """ retrieve values for fpga interface """
        node = xmlgrok.firstElementChild(node)

        while node != None:
            name = node.getAttribute('name')
            value = node.getAttribute('value')

            self.updateTemplate(name, value)

            node = xmlgrok.nextElementSibling(node)


    def createFilesFromEMIF(self):
        """ create sdram_config.h with the template and value read from xml.
        Different sequencer files are written to individual section, with
        comment at the start.
        """
        self.sdramHTemplate ="""\
#define CFG_HPS_SDR_CTRLCFG_CPORTRDWR_CPORTRDWR		0x5A56A
#define CFG_HPS_SDR_CTRLCFG_CPORTRMAP_CPORTRMAP		0xB00088
#define CFG_HPS_SDR_CTRLCFG_CPORTWIDTH_CPORTWIDTH		0x44555
#define CFG_HPS_SDR_CTRLCFG_CPORTWMAP_CPORTWMAP		0x2C011000
#define CFG_HPS_SDR_CTRLCFG_CTRLCFG_ADDRORDER		${ADDR_ORDER}
#define CFG_HPS_SDR_CTRLCFG_CTRLCFG_DQSTRKEN			${USE_HPS_DQS_TRACKING}
#define CFG_HPS_SDR_CTRLCFG_CTRLCFG_ECCCORREN		${DERIVED_ECCCORREN}
#define CFG_HPS_SDR_CTRLCFG_CTRLCFG_ECCEN			${DERIVED_ECCEN}
#define CFG_HPS_SDR_CTRLCFG_CTRLCFG_MEMBL			${MEM_BURST_LENGTH}
#define CFG_HPS_SDR_CTRLCFG_CTRLCFG_MEMTYPE			${DERIVED_MEMTYPE}
#define CFG_HPS_SDR_CTRLCFG_CTRLCFG_NODMPINS			${DERIVED_NODMPINS}
#define CFG_HPS_SDR_CTRLCFG_CTRLCFG_REORDEREN		1
#define CFG_HPS_SDR_CTRLCFG_CTRLCFG_STARVELIMIT		10
#define CFG_HPS_SDR_CTRLCFG_CTRLWIDTH_CTRLWIDTH		${DERIVED_CTRLWIDTH}
#define CFG_HPS_SDR_CTRLCFG_DRAMADDRW_BANKBITS		${MEM_IF_BANKADDR_WIDTH}
#define CFG_HPS_SDR_CTRLCFG_DRAMADDRW_COLBITS		${MEM_IF_COL_ADDR_WIDTH}
#define CFG_HPS_SDR_CTRLCFG_DRAMADDRW_CSBITS			${DEVICE_DEPTH}
#define CFG_HPS_SDR_CTRLCFG_DRAMADDRW_ROWBITS		${MEM_IF_ROW_ADDR_WIDTH}
#define CFG_HPS_SDR_CTRLCFG_DRAMDEVWIDTH_DEVWIDTH		8
#define CFG_HPS_SDR_CTRLCFG_DRAMIFWIDTH_IFWIDTH		${MEM_DQ_WIDTH}
#define CFG_HPS_SDR_CTRLCFG_DRAMINTR_INTREN			0
#define CFG_HPS_SDR_CTRLCFG_DRAMODT_READ			${CFG_READ_ODT_CHIP}
#define CFG_HPS_SDR_CTRLCFG_DRAMODT_WRITE			${CFG_WRITE_ODT_CHIP}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING1_AL			0
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING1_TCL			${MEM_TCL}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING1_TCWL			${MEM_WTCL_INT}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING1_TFAW			${MEM_TFAW}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING1_TRFC			${MEM_TRFC}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING1_TRRD			${MEM_TRRD}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TRCD		${MEM_TRCD}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TREFI		${MEM_TREFI}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TRP		${MEM_TRP}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TWR		${MEM_TWR}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TWTR		${MEM_TWTR}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING3_TCCD			4
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING3_TMRD			${MEM_TMRD_CK}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING3_TRAS			${MEM_TRAS}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING3_TRC			${MEM_TRC}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING3_TRTP			${MEM_TRTP}
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING4_PWRDOWNEXIT		3
#define CFG_HPS_SDR_CTRLCFG_DRAMTIMING4_SELFRFSHEXIT		${DERIVED_SELFRFSHEXIT}
#define CFG_HPS_SDR_CTRLCFG_EXTRATIME1_CFG_EXTRA_CTL_CLK_RD_TO_WR ${DERIVED_CLK_RD_TO_WR}
#define CFG_HPS_SDR_CTRLCFG_EXTRATIME1_CFG_EXTRA_CTL_CLK_RD_TO_WR_BC ${DERIVED_CLK_RD_TO_WR}
#define CFG_HPS_SDR_CTRLCFG_EXTRATIME1_CFG_EXTRA_CTL_CLK_RD_TO_WR_DIFF_CHIP ${DERIVED_CLK_RD_TO_WR}
#define CFG_HPS_SDR_CTRLCFG_FIFOCFG_INCSYNC			0
#define CFG_HPS_SDR_CTRLCFG_FIFOCFG_SYNCMODE			0
#define CFG_HPS_SDR_CTRLCFG_FPGAPORTRST			${F2SDRAM_RESET_PORT_USED}
#define CFG_HPS_SDR_CTRLCFG_LOWPWREQ_SELFRFSHMASK		3
#define CFG_HPS_SDR_CTRLCFG_LOWPWRTIMING_AUTOPDCYCLES	0
#define CFG_HPS_SDR_CTRLCFG_LOWPWRTIMING_CLKDISABLECYCLES	8
#define CFG_HPS_SDR_CTRLCFG_MPPACING_0_THRESHOLD1_31_0	0x20820820
#define CFG_HPS_SDR_CTRLCFG_MPPACING_1_THRESHOLD1_59_32	0x8208208
#define CFG_HPS_SDR_CTRLCFG_MPPACING_1_THRESHOLD2_3_0	0
#define CFG_HPS_SDR_CTRLCFG_MPPACING_2_THRESHOLD2_35_4	0x41041041
#define CFG_HPS_SDR_CTRLCFG_MPPACING_3_THRESHOLD2_59_36	0x410410
#define CFG_HPS_SDR_CTRLCFG_MPPRIORITY_USERPRIORITY		0x0
#define CFG_HPS_SDR_CTRLCFG_MPTHRESHOLDRST_0_THRESHOLDRSTCYCLES_31_0 0x01010101
#define CFG_HPS_SDR_CTRLCFG_MPTHRESHOLDRST_1_THRESHOLDRSTCYCLES_63_32 0x01010101
#define CFG_HPS_SDR_CTRLCFG_MPTHRESHOLDRST_2_THRESHOLDRSTCYCLES_79_64 0x0101
#define CFG_HPS_SDR_CTRLCFG_MPWIEIGHT_0_STATICWEIGHT_31_0	0x21084210
#define CFG_HPS_SDR_CTRLCFG_MPWIEIGHT_1_STATICWEIGHT_49_32	0x10441
#define CFG_HPS_SDR_CTRLCFG_MPWIEIGHT_1_SUMOFWEIGHT_13_0	0x78
#define CFG_HPS_SDR_CTRLCFG_MPWIEIGHT_2_SUMOFWEIGHT_45_14	0x0
#define CFG_HPS_SDR_CTRLCFG_MPWIEIGHT_3_SUMOFWEIGHT_63_46	0x0
#define CFG_HPS_SDR_CTRLCFG_PHYCTRL_PHYCTRL_0		0x200
#define CFG_HPS_SDR_CTRLCFG_PORTCFG_AUTOPCHEN		0
#define CFG_HPS_SDR_CTRLCFG_RFIFOCMAP_RFIFOCMAP		0x760210
#define CFG_HPS_SDR_CTRLCFG_STATICCFG_MEMBL			2
#define CFG_HPS_SDR_CTRLCFG_STATICCFG_USEECCASDATA		0
#define CFG_HPS_SDR_CTRLCFG_WFIFOCMAP_WFIFOCMAP		0x980543
"""

        # Get a list of all nodes with the emif element name
        emifNodeList = self.emifDom.getElementsByTagName('emif')
        if len(emifNodeList) > 1:
            print ("*** WARNING:" + "Multiple emif Elements found in %s!" % self.emifFileName)
        # For each of the emif element nodes, go through the child list
        # Note that currently there is only one emif Element
        # but this code will handle more than one emif node
        # In the future, multiple emif nodes may need additional code
        # to combine settings from the multiple emif Elements
        for emifNode in emifNodeList:
            # Currently, there are only 3 children of the emif Element:
            #     sequencer, controller, and pll
            # but this is left open-ended for future additions to the
            # specification for the emif.xml
            childNode = xmlgrok.firstElementChild(emifNode)
            while childNode != None:

                if childNode.nodeName == 'controller':
                    self.handleEMIFControllerNode(childNode)
                elif childNode.nodeName == 'sequencer':
                    self.handleEMIFSequencerNode(childNode)
                elif childNode.nodeName == 'pll':
                    self.handleEMIFPllNode(childNode)

                childNode = xmlgrok.nextElementSibling(childNode)

        data_rate_ratio = 2
        dwidth_ratio = self.afi_rate_ratio * data_rate_ratio
        if dwidth_ratio == 0:
            derivedClkRdToWr = 0
        else:
            derivedClkRdToWr = (self.mem_if_rd_to_wr_turnaround_oct / (dwidth_ratio / 2))

            if (self.mem_if_rd_to_wr_turnaround_oct % (dwidth_ratio / 2)) > 0:
                derivedClkRdToWr += 1

        self.updateTemplate("DERIVED_CLK_RD_TO_WR", str(int(derivedClkRdToWr)))

        # MPFE information are stored in hps.xml despite we generate
        # them into sdram_config, so let's load hps.xml
        hpsNodeList = self.hpsDom.getElementsByTagName('hps')

        for hpsNode in hpsNodeList:

            childNode = xmlgrok.firstElementChild(hpsNode)

            while childNode != None:
                # MPFE info is part of fpga_interfaces
                if childNode.nodeName == 'fpga_interfaces':
                    self.handleHpsFpgaInterfaces(childNode)

                childNode = xmlgrok.nextElementSibling(childNode)

        self.sequencerDefinesStream = streamer.Streamer(self.outputDir + os.sep + EMIFGrokker.SDRAM_CONFIG_H_FILENAME, 'w')
        self.sequencerDefinesStream.open()
        self.sequencerDefinesStream.writeLicenseHeader()
        self.sequencerDefinesStream.write(EMIFGrokker.SDRAM_FILE_HEADER)
        ret = self.sequencerDefinesStream.writeSentinelStart(EMIFGrokker.SDRAM_SENTINEL)
        if ret == -1:
            print("Empty header written. Exiting.")
        self.sequencerDefinesStream.write("/* SDRAM configuration */\n")
        self.sequencerDefinesStream.write(self.sdramHTemplate)
        self.openSeqFiles()
        self.processSeqAuto()

        self.sequencerDefinesStream.write("\n")
        self.sequencerDefinesStream.write("/* Sequencer auto configuration */\n")
        self.sequencerDefinesStream.write(self.seqAutoTemplate)
        self.sequencerDefinesStream.write("\n")
        self.sequencerDefinesStream.write("/* Sequencer defines configuration */\n")
        self.sequencerDefinesStream.write(self.seqDefinesTemplate)
        self.sequencerDefinesStream.write("\n")
        self.sequencerDefinesStream.write("/* Sequencer ac_rom_init configuration */\n")
        self.sequencerDefinesStream.write(self.seqAutoAcTemplate)
        self.sequencerDefinesStream.write("\n\n")
        self.sequencerDefinesStream.write("/* Sequencer inst_rom_init configuration */\n")
        self.sequencerDefinesStream.write(self.seqAutoInstTemplate)
        self.sequencerDefinesStream.write("\n")

        ret = self.sequencerDefinesStream.writeSentinelEnd(EMIFGrokker.SDRAM_SENTINEL)
        if ret == -1:
            print("Empty header written. Exiting.")
        self.sequencerDefinesStream.close()
        self.closeSeqFiles()
