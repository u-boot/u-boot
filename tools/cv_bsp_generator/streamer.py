# SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
"""
Generate license, file header and close tag.

Copyright (C) 2022 Intel Corporation <www.intel.com>

Author: Lee, Kah Jing <kah.jing.lee@intel.com>
"""
import os
import struct
import doc

class Streamer(object):
    """ Streamer class to generate license, header, and close tag.
    """
    def __init__(self, fileName, mode='r'):
        """ Streamer initialization """
        self.fileName = fileName
        self.mode = mode
        self.file = None
        self.sentinel = None
        if '+' in mode or 'w' in mode or 'a' in mode:
            self.fileMode = 'write'
        else:
            self.fileMode = 'read'

    def close(self):
        """ file close """
        if self.file != None:
            self.file.close()
            self.file = None

    def open(self):
        """ file open """
        if self.fileName != None:
            if self.file == None:
                if self.fileMode == 'write':
                    print ("Generating file: %s..." % self.fileName)
                else:
                    print ("Reading file: %s..." % self.fileName)
                self.file = open(self.fileName, self.mode)

    def read(self, numBytes):
        """ file read number of bytes """
        if self.file == None:
            print ("***Error: Attempted to read from unopened file %s" \
                  % (self.fileName))
            exit(-1)

        else:
            return self.file.read(numBytes)

    def readUnsignedInt(self):
        """ read unsigned integer """
        return struct.unpack('I', self.read(4))[0]

    def readUnsignedShort(self):
        """ read unsigned short """
        return struct.unpack('H', self.read(2))[0]

    def readBytesAsString(self, numBytes):
        """ Read some bytes from a binary file
        and interpret the data values as a String
        """
        bytes = self.read(numBytes)
        s = bytes.decode('utf-8')

        return s

    def write(self, str):
        """ file write """
        if self.file == None:
            print ("***Error: Attempted to write to unopened file %s" \
                % (self.fileName))
            exit(-1)

        else:
            self.file.write("%s" % str)

    def writeLicenseHeader(self):
        """ write license & copyright """
        # format the license header
        licenseHeader = "/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */\n"
        self.file.write("%s" % licenseHeader)
        copyrightHeader = "/*\n * Copyright (C) 2022 Intel Corporation <www.intel.com>\n *\n */\n"
        self.file.write("%s" % copyrightHeader)

    def writeSentinelStart(self, sentinel):
        """ start header """
        if sentinel == None:
            return -1
        self.sentinel = sentinel
        self.file.write("%s\n%s\n\n" % (\
            "#ifndef " + self.sentinel,
            "#define " + self.sentinel))

    def writeSentinelEnd(self, sentinel):
        """ end header """
        if sentinel == None:
            return -1
        self.sentinel = sentinel
        self.file.write("\n%s\n" % ("#endif /* " + self.sentinel + " */"))
