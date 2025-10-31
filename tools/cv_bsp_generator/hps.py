# SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
"""
Pinmux header file generator

Process the hps.xml from Quartus and convert them to headers
usable by U-Boot.

Copyright (C) 2022 Intel Corporation <www.intel.com>

Author: Lee, Kah Jing <kah.jing.lee@intel.com>
"""
import os
import re
import streamer
import xmlgrok
import xml.dom.minidom
import collections
import io
from io import StringIO

class CompatStringIO(io.StringIO):
    def write(self, s):
        if hasattr(s, 'decode'):
            # Use unicode for python2 to keep compatible
            return int(super(CompatStringIO, self).write(s.decode('utf-8')))
        else:
            return super(CompatStringIO, self).write(s)
    def getvalue(self):
        return str(super(CompatStringIO, self).getvalue())

class HPSGrokker(object):

    SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
    TEMPLATE_DIR = os.path.dirname(SCRIPT_DIR) + '/src'

    MAKEFILE_FILENAME = "Makefile"
    makefileTemplate = ""
    RESET_CONFIG_H_FILENAME = "reset_config.h"
    resetConfigHTemplate = ""

    # If no device family is specified, assume Cyclone V.
    derivedDeviceFamily = "cyclone5"

    # Assume FPGA DMA 0-7 are not in use by default
    # Note: there appears to be a weird mismatch between sopcinfo
    # value vs hps.xml value of DMA_Enable of string_list hw.tcl
    # type, where sopcinfo uses comma as separator e.g.
    # "No,No,No,..." while hps.xml uses space as separator.
    dmaEnable = "No No No No No No No No"

    def __init__(self, inputDir, outputDir, hpsFileName='hps.xml'):
        """ HPSGrokker initialization """
        self.inputDir = inputDir
        self.outputDir = outputDir
        self.hpsInFileName = inputDir + os.sep + hpsFileName
        self.dom = xml.dom.minidom.parse(self.hpsInFileName)
        self.peripheralStream = None
        self.pinmuxConfigBuffer = None
        self.pinmuxHeaderBuffer = None
        self.pinmuxHeaderFile = None
        self.pinmuxArraySize = 0
        self.config_hps_ = "CFG_HPS_"
        self.clockStream = None
        self.pinmux_regs = self.get_default_pinmux_regs()
        self.pinmux_configs = self.get_default_pinmux_configs()
        self.pinmux_config_h = None

        self.createFilesFromHPS()

    def get_default_pinmux_regs(self):
        """ Set default pinmux values """
        p = collections.OrderedDict()

        p['EMACIO0'] = 0
        p['EMACIO1'] = 0
        p['EMACIO2'] = 0
        p['EMACIO3'] = 0
        p['EMACIO4'] = 0
        p['EMACIO5'] = 0
        p['EMACIO6'] = 0
        p['EMACIO7'] = 0
        p['EMACIO8'] = 0
        p['EMACIO9'] = 0
        p['EMACIO10'] = 0
        p['EMACIO11'] = 0
        p['EMACIO12'] = 0
        p['EMACIO13'] = 0
        p['EMACIO14'] = 0
        p['EMACIO15'] = 0
        p['EMACIO16'] = 0
        p['EMACIO17'] = 0
        p['EMACIO18'] = 0
        p['EMACIO19'] = 0
        p['FLASHIO0'] = 0
        p['FLASHIO1'] = 0
        p['FLASHIO2'] = 0
        p['FLASHIO3'] = 0
        p['FLASHIO4'] = 0
        p['FLASHIO5'] = 0
        p['FLASHIO6'] = 0
        p['FLASHIO7'] = 0
        p['FLASHIO8'] = 0
        p['FLASHIO9'] = 0
        p['FLASHIO10'] = 0
        p['FLASHIO11'] = 0
        p['GENERALIO0'] = 0
        p['GENERALIO1'] = 0
        p['GENERALIO2'] = 0
        p['GENERALIO3'] = 0
        p['GENERALIO4'] = 0
        p['GENERALIO5'] = 0
        p['GENERALIO6'] = 0
        p['GENERALIO7'] = 0
        p['GENERALIO8'] = 0
        p['GENERALIO9'] = 0
        p['GENERALIO10'] = 0
        p['GENERALIO11'] = 0
        p['GENERALIO12'] = 0
        p['GENERALIO13'] = 0
        p['GENERALIO14'] = 0
        p['GENERALIO15'] = 0
        p['GENERALIO16'] = 0
        p['GENERALIO17'] = 0
        p['GENERALIO18'] = 0
        p['GENERALIO19'] = 0
        p['GENERALIO20'] = 0
        p['GENERALIO21'] = 0
        p['GENERALIO22'] = 0
        p['GENERALIO23'] = 0
        p['GENERALIO24'] = 0
        p['GENERALIO25'] = 0
        p['GENERALIO26'] = 0
        p['GENERALIO27'] = 0
        p['GENERALIO28'] = 0
        p['GENERALIO29'] = 0
        p['GENERALIO30'] = 0
        p['GENERALIO31'] = 0
        p['MIXED1IO0'] = 0
        p['MIXED1IO1'] = 0
        p['MIXED1IO2'] = 0
        p['MIXED1IO3'] = 0
        p['MIXED1IO4'] = 0
        p['MIXED1IO5'] = 0
        p['MIXED1IO6'] = 0
        p['MIXED1IO7'] = 0
        p['MIXED1IO8'] = 0
        p['MIXED1IO9'] = 0
        p['MIXED1IO10'] = 0
        p['MIXED1IO11'] = 0
        p['MIXED1IO12'] = 0
        p['MIXED1IO13'] = 0
        p['MIXED1IO14'] = 0
        p['MIXED1IO15'] = 0
        p['MIXED1IO16'] = 0
        p['MIXED1IO17'] = 0
        p['MIXED1IO18'] = 0
        p['MIXED1IO19'] = 0
        p['MIXED1IO20'] = 0
        p['MIXED1IO21'] = 0
        p['MIXED2IO0'] = 0
        p['MIXED2IO1'] = 0
        p['MIXED2IO2'] = 0
        p['MIXED2IO3'] = 0
        p['MIXED2IO4'] = 0
        p['MIXED2IO5'] = 0
        p['MIXED2IO6'] = 0
        p['MIXED2IO7'] = 0
        p['GPLINMUX48'] = 0
        p['GPLINMUX49'] = 0
        p['GPLINMUX50'] = 0
        p['GPLINMUX51'] = 0
        p['GPLINMUX52'] = 0
        p['GPLINMUX53'] = 0
        p['GPLINMUX54'] = 0
        p['GPLINMUX55'] = 0
        p['GPLINMUX56'] = 0
        p['GPLINMUX57'] = 0
        p['GPLINMUX58'] = 0
        p['GPLINMUX59'] = 0
        p['GPLINMUX60'] = 0
        p['GPLINMUX61'] = 0
        p['GPLINMUX62'] = 0
        p['GPLINMUX63'] = 0
        p['GPLINMUX64'] = 0
        p['GPLINMUX65'] = 0
        p['GPLINMUX66'] = 0
        p['GPLINMUX67'] = 0
        p['GPLINMUX68'] = 0
        p['GPLINMUX69'] = 0
        p['GPLINMUX70'] = 0
        p['GPLMUX0'] = 1
        p['GPLMUX1'] = 1
        p['GPLMUX2'] = 1
        p['GPLMUX3'] = 1
        p['GPLMUX4'] = 1
        p['GPLMUX5'] = 1
        p['GPLMUX6'] = 1
        p['GPLMUX7'] = 1
        p['GPLMUX8'] = 1
        p['GPLMUX9'] = 1
        p['GPLMUX10'] = 1
        p['GPLMUX11'] = 1
        p['GPLMUX12'] = 1
        p['GPLMUX13'] = 1
        p['GPLMUX14'] = 1
        p['GPLMUX15'] = 1
        p['GPLMUX16'] = 1
        p['GPLMUX17'] = 1
        p['GPLMUX18'] = 1
        p['GPLMUX19'] = 1
        p['GPLMUX20'] = 1
        p['GPLMUX21'] = 1
        p['GPLMUX22'] = 1
        p['GPLMUX23'] = 1
        p['GPLMUX24'] = 1
        p['GPLMUX25'] = 1
        p['GPLMUX26'] = 1
        p['GPLMUX27'] = 1
        p['GPLMUX28'] = 1
        p['GPLMUX29'] = 1
        p['GPLMUX30'] = 1
        p['GPLMUX31'] = 1
        p['GPLMUX32'] = 1
        p['GPLMUX33'] = 1
        p['GPLMUX34'] = 1
        p['GPLMUX35'] = 1
        p['GPLMUX36'] = 1
        p['GPLMUX37'] = 1
        p['GPLMUX38'] = 1
        p['GPLMUX39'] = 1
        p['GPLMUX40'] = 1
        p['GPLMUX41'] = 1
        p['GPLMUX42'] = 1
        p['GPLMUX43'] = 1
        p['GPLMUX44'] = 1
        p['GPLMUX45'] = 1
        p['GPLMUX46'] = 1
        p['GPLMUX47'] = 1
        p['GPLMUX48'] = 1
        p['GPLMUX49'] = 1
        p['GPLMUX50'] = 1
        p['GPLMUX51'] = 1
        p['GPLMUX52'] = 1
        p['GPLMUX53'] = 1
        p['GPLMUX54'] = 1
        p['GPLMUX55'] = 1
        p['GPLMUX56'] = 1
        p['GPLMUX57'] = 1
        p['GPLMUX58'] = 1
        p['GPLMUX59'] = 1
        p['GPLMUX60'] = 1
        p['GPLMUX61'] = 1
        p['GPLMUX62'] = 1
        p['GPLMUX63'] = 1
        p['GPLMUX64'] = 1
        p['GPLMUX65'] = 1
        p['GPLMUX66'] = 1
        p['GPLMUX67'] = 1
        p['GPLMUX68'] = 1
        p['GPLMUX69'] = 1
        p['GPLMUX70'] = 1
        p['NANDUSEFPGA'] = 0
        p['UART0USEFPGA'] = 0
        p['RGMII1USEFPGA'] = 0
        p['SPIS0USEFPGA'] = 0
        p['CAN0USEFPGA'] = 0
        p['I2C0USEFPGA'] = 0
        p['SDMMCUSEFPGA'] = 0
        p['QSPIUSEFPGA'] = 0
        p['SPIS1USEFPGA'] = 0
        p['RGMII0USEFPGA'] = 0
        p['UART1USEFPGA'] = 0
        p['CAN1USEFPGA'] = 0
        p['USB1USEFPGA'] = 0
        p['I2C3USEFPGA'] = 0
        p['I2C2USEFPGA'] = 0
        p['I2C1USEFPGA'] = 0
        p['SPIM1USEFPGA'] = 0
        p['USB0USEFPGA'] = 0
        p['SPIM0USEFPGA'] = 0

        return p


    def get_default_pinmux_configs(self):
        """ Get default pinmux values """
        p = collections.OrderedDict()

        p['rgmii0'] = { 'name': 'CFG_HPS_EMAC0', 'used': 0 }
        p['rgmii1'] = { 'name': 'CFG_HPS_EMAC1', 'used': 0 }
        p['usb0'] = { 'name': 'CFG_HPS_USB0', 'used': 0 }
        p['usb1'] = { 'name': 'CFG_HPS_USB1', 'used': 0 }
        p['nand'] = { 'name': 'CFG_HPS_NAND', 'used': 0 }
        p['sdmmc'] = { 'name': 'CFG_HPS_SDMMC', 'used': 0 }
        p['CFG_HPS_SDMMC_BUSWIDTH'] = { 'name': 'CFG_HPS_SDMMC_BUSWIDTH', 'used': 0 }
        p['qspi'] = { 'name': 'CFG_HPS_QSPI', 'used': 0 }
        p['CFG_HPS_QSPI_CS3'] = { 'name': 'CFG_HPS_QSPI_CS3', 'used': 0 }
        p['CFG_HPS_QSPI_CS2'] = { 'name': 'CFG_HPS_QSPI_CS2', 'used': 0 }
        p['CFG_HPS_QSPI_CS1'] = { 'name': 'CFG_HPS_QSPI_CS1', 'used': 0 }
        p['CFG_HPS_QSPI_CS0'] = { 'name': 'CFG_HPS_QSPI_CS0', 'used': 0 }
        p['uart0'] = { 'name': 'CFG_HPS_UART0', 'used': 0 }
        p['CFG_HPS_UART0_TX'] = { 'name': 'CFG_HPS_UART0_TX', 'used': 0 }
        p['CFG_HPS_UART0_CTS'] = { 'name': 'CFG_HPS_UART0_CTS', 'used': 0 }
        p['CFG_HPS_UART0_RTS'] = { 'name': 'CFG_HPS_UART0_RTS', 'used': 0 }
        p['CFG_HPS_UART0_RX'] = { 'name': 'CFG_HPS_UART0_RX', 'used': 0 }
        p['uart1'] = { 'name': 'CFG_HPS_UART1', 'used': 0 }
        p['CFG_HPS_UART1_TX'] = { 'name': 'CFG_HPS_UART1_TX', 'used': 0 }
        p['CFG_HPS_UART1_CTS'] = { 'name': 'CFG_HPS_UART1_CTS', 'used': 0 }
        p['CFG_HPS_UART1_RTS'] = { 'name': 'CFG_HPS_UART1_RTS', 'used': 0 }
        p['CFG_HPS_UART1_RX'] = { 'name': 'CFG_HPS_UART1_RX', 'used': 0 }
        p['trace'] = { 'name': 'CFG_HPS_TRACE', 'used': 0 }
        p['i2c0'] = { 'name': 'CFG_HPS_I2C0', 'used': 0 }
        p['i2c1'] = { 'name': 'CFG_HPS_I2C1', 'used': 0 }
        p['i2c2'] = { 'name': 'CFG_HPS_I2C2', 'used': 0 }
        p['i2c3'] = { 'name': 'CFG_HPS_I2C3', 'used': 0 }
        p['spim0'] = { 'name': 'CFG_HPS_SPIM0', 'used': 0 }
        p['spim1'] = { 'name': 'CFG_HPS_SPIM1', 'used': 0 }
        p['spis0'] = { 'name': 'CFG_HPS_SPIS0', 'used': 0 }
        p['spis1'] = { 'name': 'CFG_HPS_SPIS1', 'used': 0 }
        p['can0'] = { 'name': 'CFG_HPS_CAN0', 'used': 0 }
        p['can1'] = { 'name': 'CFG_HPS_CAN1', 'used': 0 }

        p['can1'] = { 'name': 'CFG_HPS_CAN1', 'used': 0 }
        p['can1'] = { 'name': 'CFG_HPS_CAN1', 'used': 0 }
        p['can1'] = { 'name': 'CFG_HPS_CAN1', 'used': 0 }
        p['can1'] = { 'name': 'CFG_HPS_CAN1', 'used': 0 }

        return p

    def updateTemplate(self, name, value):
        """ Update Makefile & reset_config.h """
        pattern = "${" + name + "}"
        self.makefileTemplate = self.makefileTemplate.replace(pattern, value)
        self.resetConfigHTemplate = self.resetConfigHTemplate.replace(pattern, value)

    def romanToInteger(self, roman):
        """
        Convert roman numerals to integer
        Since we only support I,V,X, the
        supported range is 1-39
        """
        table = { 'I':1 , 'V':5, 'X':10 }

        literals = list(roman)

        value = 0
        i = 0

        while(i < (len(literals) - 1)):
            current = table[literals[i]]
            next = table[literals[i + 1]]
            if (current < next):
                value += (next - current)
                i += 2
            else:
                value += current
                i += 1

        if (i < (len(literals))):
            value += table[literals[i]]

        return value

    def getDeviceFamily(self):
        """ Get device family """
        return self.derivedDeviceFamily

    def getDeviceFamilyName(self, deviceFamily):
        """ Get device family name """
        p = re.compile('^(\w+)\s+(\w+)$')
        m = p.match(deviceFamily)
        return m.group(1).lower() + str(self.romanToInteger(m.group(2)))

    def handleHPSSystemNode(self, systemNode):
        """ handleHPSPeripheralsNode(peripheralsNode)
        peripheralsNode is a peripherals element node in hps.xml
        peripheralsNode is a list of peripheralNodes
        """
        configNode = xmlgrok.firstElementChild(systemNode)
        while configNode != None:

            name = configNode.getAttribute('name')
            value = configNode.getAttribute('value')

            self.updateTemplate(name, value)

            if name == "DEVICE_FAMILY":
                self.derivedDeviceFamily = self.getDeviceFamilyName(value)

            if name == "DMA_Enable":
                self.dmaEnable = value

            configNode = xmlgrok.nextElementSibling(configNode)

    def handleHPSPeripheralNode(self, peripheralNode):
        """ This node of the hps.xml may contain a name, value pair
        We need to:
            emit a #define for the peripheral for is 'used' state
            emit a #define for that pair, if it is marked 'used'
        """
        peripheralNode = xmlgrok.firstElementChild(peripheralNode)

        while peripheralNode != None:
            if peripheralNode.hasAttribute('name') and peripheralNode.hasAttribute('used'):
                newLine = "\n"
                name = peripheralNode.getAttribute('name')
                used = peripheralNode.getAttribute('used')

                if used == 'true' or used == True:
                    used = 1
                elif used == 'false' or used == False:
                    used = 0

                configs = collections.OrderedDict()

                configNode = xmlgrok.firstElementChild(peripheralNode)
                while configNode != None:
                    config_define_name = configNode.getAttribute('name')
                    config_define_value = configNode.getAttribute('value')
                    configs[config_define_name] = config_define_value
                    configNode = xmlgrok.nextElementSibling(configNode)
                    if configNode == None:
                        newLine += newLine
                    self.pinmuxConfigBuffer.write("#define " + str(config_define_name) + ' ' + '(' + str(config_define_value) + ')' + newLine)

                entry = self.pinmux_configs[name]
                define_name = entry['name']

                if (len(configs) > 0):
                    self.pinmux_configs[name] = { 'name': define_name, 'used': used, 'configs': configs }
                else:
                    self.pinmux_configs[name] = { 'name': define_name, 'used': used }

                # skip the parent peripheral node
                # since only need to define child config node(s)
                peripheralNode = xmlgrok.nextElementSibling(peripheralNode)

    def handleHPSPinmuxNode(self, pinmuxNode):
        """ For a pinmuxNode, we may emit a #define for the name, value pair
        """
        if pinmuxNode.hasAttribute('name') and pinmuxNode.hasAttribute('value'):
            self.pinmuxArraySize += 1
            name = pinmuxNode.getAttribute('name')
            value = pinmuxNode.getAttribute('value')

    def handleHPSPinmuxesNode(self, pinmuxesNode):
        """ PinmuxesNode is a list of pinmuxNodes
        """
        self.pinmuxHeaderBuffer.write(str("const u8 sys_mgr_init_table[] = {\n"))

        pinmuxNode = xmlgrok.firstElementChild(pinmuxesNode)
        while pinmuxNode != None:
            if pinmuxNode.hasAttribute('name') and pinmuxNode.hasAttribute('value'):
                self.pinmuxArraySize += 1
                name = pinmuxNode.getAttribute('name')
                value = pinmuxNode.getAttribute('value')
                self.pinmux_regs[name] = value
            pinmuxNode = xmlgrok.nextElementSibling(pinmuxNode)

        reg_count = 0
        pinmux_regs_count = len(self.pinmux_regs)
        for reg, value in self.pinmux_regs.items():
            reg_count += 1
            if reg_count < pinmux_regs_count:
                self.pinmuxHeaderBuffer.write(str("\t" + str(value) + ', /* ' + reg + ' */\n' ))
            else:
                self.pinmuxHeaderBuffer.write(str("\t" + str(value) + ' /* ' + reg + ' */\n' ))

        # Write the close of the pin MUX array in the header
        self.pinmuxHeaderBuffer.write(str("};" ))

    def handleHPSClockNode(self, clockNode):
        """ A clockNode may emit a #define for the name, frequency pair
        """
        if clockNode.hasAttribute('name') and clockNode.hasAttribute('frequency'):
            name = clockNode.getAttribute('name')
            frequency = clockNode.getAttribute('frequency')
            self.clockStream.write("#define " + name + ' ' + '(' + frequency + ')' + '\n')

    def handleHPSClocksNode(self, clocksNode):
        """ A list of clockNodes is call clocksNode
        """
        self.clockStream = streamer.Streamer(self.outputDir + os.sep + clocksNode.nodeName + '.h', 'w')
        self.clockStream.open()
        clockNode = xmlgrok.firstElementChild(clocksNode)
        while clockNode != None:
            self.handleHPSClockNode(clockNode)
            clockNode = xmlgrok.nextElementSibling(clockNode)

        self.clockStream.close()

    def handleHpsFpgaInterfaces(self, node):
        """ Update FPGA Interface registers """
        node = xmlgrok.firstElementChild(node)

        while node != None:
            name = node.getAttribute('name')
            used = node.getAttribute('used')

            if used == 'true':
                reset = 0
            else:
                reset = 1

            if name == 'F2H_AXI_SLAVE':
                self.updateTemplate("DERIVED_RESET_ASSERT_FPGA2HPS", str(reset))
            elif name == 'H2F_AXI_MASTER':
                self.updateTemplate("DERIVED_RESET_ASSERT_HPS2FPGA", str(reset))
            elif name == 'LWH2F_AXI_MASTER':
                self.updateTemplate("DERIVED_RESET_ASSERT_LWHPS2FPGA", str(reset))

            node = xmlgrok.nextElementSibling(node)

    def createFilesFromHPS(self):
        """ Parse xml and create pinmux_config.h """
        # Unfortunately we can't determine the file name before
        # parsing the XML, so let's build up the source file
        # content in string buffer
        self.pinmuxHeaderBuffer = CompatStringIO()
        self.pinmuxConfigBuffer = CompatStringIO()

        # Get a list of all nodes with the hps element name
        hpsNodeList = self.dom.getElementsByTagName('hps')
        if len(hpsNodeList) > 1:
            print ("*** WARNING:" + "Multiple hps Elements found in %s!" % self.hpsInFileName)
        # For each of the hps element nodes, go through the child list
        # Note that currently there is only one hps Element
        # but this code will handle more than one hps node
        # In the future, multiple hps nodes may need additional code
        # to combine settings from the multiple hps Elements
        for hpsNode in hpsNodeList:
            # Currently, there are only 3 children of the hps Element:
            #     peripherals, pin_muxes, and clocks
            # but this is left open-ended for future additions to the
            # specification for the hps.xml
            childNode = xmlgrok.firstElementChild(hpsNode)
            while childNode != None:
                if childNode.nodeName == 'pin_muxes':
                    self.handleHPSPinmuxesNode(childNode)
                elif childNode.nodeName == 'system':
                    self.handleHPSSystemNode(childNode)
                elif childNode.nodeName == 'fpga_interfaces':
                    self.handleHpsFpgaInterfaces(childNode)
                elif childNode.nodeName == 'peripherals':
                    self.handleHPSPeripheralNode(childNode)
                else:
                    print ("***Error:Found unexpected HPS child node:%s" % childNode.nodeName)
                childNode = xmlgrok.nextElementSibling(childNode)

        self.updateTemplate("DERIVED_DEVICE_FAMILY", self.derivedDeviceFamily)

        # Now we write string buffers into files once we know the device family
        self.pinmux_config_h = 'pinmux_config.h'
        self.pinmux_config_src = 'pinmux_config_' + self.derivedDeviceFamily + '.c'

        # Create pinmux_config .h
        headerDefine = "__SOCFPGA_PINMUX_CONFIG_H__"
        self.pinmuxHeaderFile = streamer.Streamer(self.outputDir + os.sep + self.pinmux_config_h, 'w')
        self.pinmuxHeaderFile.open()
        self.pinmuxHeaderFile.writeLicenseHeader()
        self.pinmuxHeaderFile.write('/*\n * Altera SoCFPGA PinMux configuration\n */\n\n')

        self.pinmuxHeaderFile.write("#ifndef " + headerDefine + "\n")
        self.pinmuxHeaderFile.write("#define " + headerDefine + "\n\n")
        self.pinmuxHeaderFile.write(self.pinmuxHeaderBuffer.getvalue())
        self.pinmuxHeaderFile.write("\n#endif /* " + headerDefine + " */\n")
        self.pinmuxHeaderFile.close()

        # Free up string buffers
        self.pinmuxHeaderBuffer.close()
        self.pinmuxConfigBuffer.close()
