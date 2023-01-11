# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
#
# TI Script for Board Configuration Packaging
#

import argparse
import logging
import os
import struct
import tempfile
from shutil import copyfileobj
from shutil import rmtree

BOARDCFG = 0xB
BOARDCFG_SEC = 0xD
BOARDCFG_PM = 0xE
BOARDCFG_RM = 0xC
BOARDCFG_NUM_ELEMS = 4

class BoardCfgDesc():
    """Get board config descriptor for a given file """

    fmt = '<HHHBB'
    index = 0
    offset = 0

    def __init__(self, outfile, devgrp,
                    sw_rev = 0,
                    num_elems = BOARDCFG_NUM_ELEMS):
        self.devgrp = devgrp
        try:
            self.fh = open(outfile, 'wb')
            bytes = self.fh.write(struct.pack('<BB', num_elems, sw_rev))
            self.offset += bytes
            self.offset += num_elems * struct.calcsize(self.fmt)
            self.tmpdir = tempfile.mkdtemp()
            descfile = os.path.join(self.tmpdir, "desc")
            bcfgfile = os.path.join(self.tmpdir, "bcfg")
            self.desc_fh = open(descfile, "wb+")
            self.bcfg_fh = open(bcfgfile, "wb+")
        except:
            raise Exception("File Error")

    def add_boardcfg(self, bcfgtype, bcfgfile):
        with open(bcfgfile, 'rb') as bfh:
            bcfg = bfh.read()
            size = len(bcfg)
            desc = struct.pack(self.fmt, bcfgtype, self.offset, size, self.devgrp, 0)
            self.desc_fh.write(desc)
            self.bcfg_fh.write(bcfg)
            logging.debug("Packing boardcfg data of size [%d bytes] from file %s", size, bcfgfile)
            self.offset += size
            self.index += 1

    def finalize(self):
        try:
            self.desc_fh.seek(0)
            self.bcfg_fh.seek(0)
            copyfileobj(self.desc_fh, self.fh)
            copyfileobj(self.bcfg_fh, self.fh)
        except:
            logging.error("**** Error in finalizing boardcfg file ****")
            raise Exception("File Error")
        finally:
            self.fh.close()
            self.desc_fh.close()
            self.bcfg_fh.close()
            rmtree(self.tmpdir)

def create_sysfw_blob(args):
    """Create a SYSFW data blob to be used as a component in combined image """

    logging.info("#### Creating SYSFW data blob - %s ####", args.output_file.name)
    logging.info("#### SW Rev = %d", args.sw_rev)
    logging.info("#### Device Group = %d", args.devgrp)

    cnt = 0
    if args.boardcfg is not None: cnt = cnt + 1
    if args.boardcfg_sec is not None: cnt = cnt + 1
    if args.boardcfg_pm is not None: cnt = cnt + 1
    if args.boardcfg_rm is not None: cnt = cnt + 1

    blob = BoardCfgDesc(args.output_file.name, args.devgrp, args.sw_rev, cnt)
    if args.boardcfg is not None:
        logging.info("#### Board config binary - %s", args.boardcfg.name)
        blob.add_boardcfg(BOARDCFG, args.boardcfg.name)
    if args.boardcfg_sec is not None:
        logging.info("#### Board config security binary - %s", args.boardcfg_sec.name)
        blob.add_boardcfg(BOARDCFG_SEC, args.boardcfg_sec.name)
    if args.boardcfg_pm is not None:
        logging.info("#### Board config PM binary - %s", args.boardcfg_pm.name)
        blob.add_boardcfg(BOARDCFG_PM, args.boardcfg_pm.name)
    if args.boardcfg_rm is not None:
        logging.info("#### Board config RM binary - %s", args.boardcfg_rm.name)
        blob.add_boardcfg(BOARDCFG_RM, args.boardcfg_rm.name)

    blob.finalize()

# options -> device, sw_rev, boardcfg, security boardcfg, pm boardcfg, rm boardcfg, output file

# parser for mandatory arguments
pp = argparse.ArgumentParser(add_help=False)
pp.add_argument('-l', '--log-level', type=str, default="INFO", choices=["INFO", "DEBUG"])
pp.add_argument('--sw-rev', type=int, default=1)
pp.add_argument('-o', '--output-file', type=argparse.FileType('wb'), default="./sysfw-data.bin")
pp.add_argument('-d', '--devgrp', type=int, default=0)
pp.add_argument('-b', '--boardcfg', type=argparse.FileType('rb'))
pp.add_argument('-s', '--boardcfg-sec', type=argparse.FileType('rb'))
pp.add_argument('-p', '--boardcfg-pm', type=argparse.FileType('rb'))
pp.add_argument('-r', '--boardcfg-rm', type=argparse.FileType('rb'))

args = pp.parse_args()
logging.getLogger().setLevel(args.log_level)
logging.debug(args)
create_sysfw_blob(args)
