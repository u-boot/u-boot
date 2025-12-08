# SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
"""
IOCSR header file generator

Process the hiof file from Quartus and generate iocsr header
usable by U-Boot.

Copyright (C) 2022 Intel Corporation <www.intel.com>

Author: Lee, Kah Jing <kah.jing.lee@intel.com>
"""
import os
import struct
import streamer

class IOCSRGrokker(object):
    """ Decode the .hiof file and produce some C source code
    """
    IOCSR_ROOT_FILENAME = 'iocsr_config'
    IOCSR_SENTINEL = '__SOCFPGA_IOCSR_CONFIG_H__'
    IOCSR_FILE_EXTENSION_MAX_LEN = 6
    PTAG_HPS_IOCSR_INFO = 39
    PTAG_HPS_IOCSR = 40
    PTAG_DEVICE_NAME = 2
    PTAG_TERMINATION = 8

    def __init__(self, deviceFamily, inputDir, outputDir, hiofSrcFileName):
        """ IOCSRGrokker Initialization """
        self.deviceFamily = deviceFamily
        self.inputDir = inputDir
        self.outputDir = outputDir
        self.hiofInFileName = hiofSrcFileName
        self.iocsrFileName =  self.IOCSR_ROOT_FILENAME
        self.headerOut = None
        self.sourceOut = None
        self.createFilesFromHIOF()

    @staticmethod
    def byteArrayToStr(bytes):
        """ Convert a list of bytes into a string
        """
        # We don't like nulls
        bytes = bytes.replace('\x00', '')
        s = ''
        for b in bytes:
            s += b
        return s

    @staticmethod
    def getLengthData(bytes):
        """
        @param: bytes is a chunk of bytes that we need to decode
        There will be a ptag that we may care about.
        If we care about it, we will get the length of the chunk
        that the ptag cares about.
        @rtype: a pair, length of chunk and the chunk itself
        @return: length of the ptag chunk we care about
        @return: data chunk that ptag indicates we need to decode
        """
        blockSize = len(bytes)
        i = 0
        bitlength = 0
        length = 0
        data = []

        while i < blockSize:
            byte = struct.unpack('B', bytes[i:i+1])[0]
            i += 1

            if byte == 1:
                bitlength = struct.unpack('I', bytes[i:i+4])[0]
                i += 4
            elif byte == 2:
                length = struct.unpack('I', bytes[i:i+4])[0]
                i += 4

            elif byte == 5:
                j = 0
                while i < blockSize:
                    data.append(struct.unpack('I', bytes[i:i+4])[0])
                    i += 4
                    j += 1

            else:
                i += 4

        return (bitlength, data)


    def verifyRead(self, tagWeRead, tagWeExpected):
        """ verify the hiof value with tag expected """
        if tagWeRead != tagWeExpected:
            print ("***Error: Expected ptag of %02d, but got %02d" % (tagWeExpected, tagWeRead))

    def createFilesFromHIOF(self):
        """ read the hiof file to create iocsr_config.h """
        self.hiofStream = streamer.Streamer(self.inputDir + os.sep + self.hiofInFileName, 'rb')
        self.iocsrHeaderStream = streamer.Streamer(self.outputDir + os.sep + self.iocsrFileName + '.h', 'w')
        self.hiofStream.open()
        self.iocsrHeaderStream.open()
        self.iocsrHeaderStream.writeLicenseHeader()
        self.iocsrHeaderStream.write('/*\n * Altera SoCFPGA IOCSR configuration\n */\n\n')
        ret = self.iocsrHeaderStream.writeSentinelStart(IOCSRGrokker.IOCSR_SENTINEL)
        if ret == -1:
            print("Empty header written. Exiting.")

        # Read the file extension (typically .hiof)
        # and the file version
        self.fileExtension = self.hiofStream.readBytesAsString(IOCSRGrokker.IOCSR_FILE_EXTENSION_MAX_LEN)
        self.fileVersion = self.hiofStream.readUnsignedInt()

        # Now read the ptags
        # Device name is first
        self.programmerTag = self.hiofStream.readUnsignedShort()
        self.verifyRead(self.programmerTag, self.PTAG_DEVICE_NAME)
        self.deviceNameLength = self.hiofStream.readUnsignedInt()
        self.deviceName = self.hiofStream.readBytesAsString(self.deviceNameLength)

        # Basic information of the HIOF files
        # This is not used by the preloader generator, but we read it and ignore the
        # contents.
        programmerTag = self.hiofStream.readUnsignedShort()
        self.verifyRead(programmerTag, self.PTAG_HPS_IOCSR_INFO)
        basicHPSIOCSRInfoLength = self.hiofStream.readUnsignedInt()
        self.hiofStream.read(basicHPSIOCSRInfoLength)

        # Actual content of IOCSR information
        self.programmerTag1 = self.hiofStream.readUnsignedShort()
        self.verifyRead(self.programmerTag1, self.PTAG_HPS_IOCSR)
        self.HPSIOCSRLength1 = self.hiofStream.readUnsignedInt()
        self.HPSIOCSRBytes1 = self.hiofStream.read(self.HPSIOCSRLength1)
        self.HPSIOCSRDataLength1, self.HPSIOCSRData1 = IOCSRGrokker.getLengthData(self.HPSIOCSRBytes1)

        # Actual content of IOCSR information
        self.programmerTag2 = self.hiofStream.readUnsignedShort()
        self.verifyRead(self.programmerTag2, self.PTAG_HPS_IOCSR)
        self.HPSIOCSRLength2 = self.hiofStream.readUnsignedInt()
        self.HPSIOCSRBytes2 = self.hiofStream.read(self.HPSIOCSRLength2)
        self.HPSIOCSRDataLength2, self.HPSIOCSRData2 = IOCSRGrokker.getLengthData(self.HPSIOCSRBytes2)

        # Actual content of IOCSR information
        self.programmerTag3 = self.hiofStream.readUnsignedShort()
        self.verifyRead(self.programmerTag3, self.PTAG_HPS_IOCSR)
        self.HPSIOCSRLength3 = self.hiofStream.readUnsignedInt()
        self.HPSIOCSRBytes3 = self.hiofStream.read(self.HPSIOCSRLength3)
        self.HPSIOCSRDataLength3, self.HPSIOCSRData3 = IOCSRGrokker.getLengthData(self.HPSIOCSRBytes3)

        # Actual content of IOCSR information
        self.programmerTag4 = self.hiofStream.readUnsignedShort()
        self.verifyRead(self.programmerTag4, self.PTAG_HPS_IOCSR)
        self.HPSIOCSRLength4 = self.hiofStream.readUnsignedInt()
        self.HPSIOCSRBytes4 = self.hiofStream.read(self.HPSIOCSRLength4)
        self.HPSIOCSRDataLength4, self.HPSIOCSRData4 = IOCSRGrokker.getLengthData(self.HPSIOCSRBytes4)

        # Now we should see the end of the hiof input
        programmerTag = self.hiofStream.readUnsignedShort()
        if 8 != programmerTag:
            print ("I didn't find the end of the .hiof file when I expected to!")

        self.iocsrHeaderStream.write('#define CFG_HPS_IOCSR_SCANCHAIN0_LENGTH\t' +\
                       str(self.HPSIOCSRDataLength1) + '\n')
        self.iocsrHeaderStream.write('#define CFG_HPS_IOCSR_SCANCHAIN1_LENGTH\t' +\
                       str(self.HPSIOCSRDataLength2) + '\n')
        self.iocsrHeaderStream.write('#define CFG_HPS_IOCSR_SCANCHAIN2_LENGTH\t' +\
                       str(self.HPSIOCSRDataLength3) + '\n')
        self.iocsrHeaderStream.write('#define CFG_HPS_IOCSR_SCANCHAIN3_LENGTH\t' +\
                       str(self.HPSIOCSRDataLength4) + '\n')

        self.iocsrHeaderStream.write("\n")

        self.iocsrHeaderStream.write('const unsigned long iocsr_scan_chain0_table[] = {\n')
        for value in self.HPSIOCSRData1:
            hv = '0x%08X' % (value)
            self.iocsrHeaderStream.write('\t' + hv + ',\n')
        self.iocsrHeaderStream.write('};\n')
        self.iocsrHeaderStream.write('\n')

        self.iocsrHeaderStream.write('const unsigned long iocsr_scan_chain1_table[] = {\n')
        for value in self.HPSIOCSRData2:
            hv = '0x%08X' % (value)
            self.iocsrHeaderStream.write('\t' + hv + ',\n')
        self.iocsrHeaderStream.write('};\n')
        self.iocsrHeaderStream.write('\n')

        self.iocsrHeaderStream.write('const unsigned long iocsr_scan_chain2_table[] = {\n')
        for value in self.HPSIOCSRData3:
            hv = '0x%08X' % (value)
            self.iocsrHeaderStream.write('\t' + hv + ',\n')
        self.iocsrHeaderStream.write('};\n')
        self.iocsrHeaderStream.write('\n')

        self.iocsrHeaderStream.write('const unsigned long iocsr_scan_chain3_table[] = {\n')
        for value in self.HPSIOCSRData4:
            hv = '0x%08X' % (value)
            self.iocsrHeaderStream.write('\t' + hv + ',\n')
        self.iocsrHeaderStream.write('};\n')
        self.iocsrHeaderStream.write('\n\n')

        ret = self.iocsrHeaderStream.writeSentinelEnd(IOCSRGrokker.IOCSR_SENTINEL)
        if ret == -1:
            print("Empty header written. Exiting.")

        self.iocsrHeaderStream.close()
