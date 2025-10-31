#! /usr/bin/env python
# SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
"""
Bsp preloader header file generator

Process the handoff files from Quartus and convert them to headers
usable by U-Boot. Includes the qts filter.sh capability to generate
correct format for headers to be used for mainline Uboot on FPGA,
namely Cyclone V & Arria V.

Copyright (C) 2022 Intel Corporation <www.intel.com>

Author: Lee, Kah Jing <kah.jing.lee@intel.com>
"""
import glob
import optparse
import os
import shutil
import emif
import hps
import iocsr
import renderer
import model
import collections
import sys

def printUsage():
    """ usage string """
    print ("Usage:\n\t%s\n" % ("sys.argv[0], --input_dir=<path to iswinfo directory> --output_dir=<path store output files>"))
    exit(1)

def verifyInputDir(dir):
    """ check if the input directory exists """
    if not os.path.isdir(dir):
        print ("There is no such directory '%s'!\n" % (dir))
        exit(1)

def verifyOutputDir(dir):
    """ check if the output directory exists """
    if not os.path.isdir(dir):
        os.makedirs(dir)

if __name__ == '__main__':
    # Do some rudimentary command line processing until it is proven we need something
    # heavier, such as argparse (preferred, but 2.7+ only) or optparse

    inputDir = '.'
    outputDir = '.'

    progVersion = '%prog 1.0'
    progDesc = 'Generate board-specific files for the preloader'
    optParser = optparse.OptionParser(version=progVersion, description=progDesc)
    optParser.add_option('-i', '--input-dir', action='store', type='string', dest='inputDir', default='.',
                         help='input-dir is usually the iswinfo directory')
    optParser.add_option('-o', '--output-dir', action='store', type='string', dest='outputDir', default='.',
                         help='output-dir is usually the directory containing the preloader source')

    (options, args) = optParser.parse_args()

    for arg in args:
        print ("***WARNING: I don't understand '%s', so I am ignoring it\n" % (arg))

    inputDir = options.inputDir
    verifyInputDir(inputDir)
    outputDir = options.outputDir

    verifyOutputDir(outputDir)

    emif = emif.EMIFGrokker(inputDir, outputDir, 'emif.xml')
    hps = hps.HPSGrokker(inputDir, outputDir)

    pllConfigH = outputDir + "/" + "pll_config.h"
    print ("Generating file: " + pllConfigH)
    hpsModel = model.hps.create(inputDir + "/" + "hps.xml")
    emifModel = model.emif.create(inputDir +"/" + "emif.xml")

    content=str(renderer.pll_config_h(hpsModel, emifModel))
    f = open(pllConfigH, "w")
    f.write(content)
    f.close()

    # For all the .hiof files, make a iocsr_config.[h|c]
    # Only support single hiof file currently
    hiof_list = glob.glob(inputDir + os.sep + "*.hiof")
    if len(hiof_list) < 1:
        print ("***Error: No .hiof files found in input!")

    elif len(hiof_list) > 1:
        print ("***Error: We don't handle more than one .hiof file yet")
        print ("          Only the last .hiof file in the list will be converted")
        print ("          hiof files found:")
        for f in hiof_list:
            print ("              " + f)

    for hiof_file_path in hiof_list:
        hiof_file = os.path.basename(hiof_file_path)
        # Avoid IOCSRGrokker having to parse hps.xml to determine
        # device family for output file name, instead we'll just
        # get it from HPSGrokker
        iocsr = iocsr.IOCSRGrokker(hps.getDeviceFamily(), inputDir, outputDir, hiof_file)
