#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2024 Stephan Gerhold
# Copyright (C) 2026 Casey Connolly
#
# This is a port of qtestsign designed to integrate with the
# U-Boot build system. See the qtestsign repo for more information.
# https://github.com/msm8916-mainline/qtestsign
#
from __future__ import annotations

import argparse
from pathlib import Path

from elf import Elf, Phdr
import hashseg
import sys
from enum import Enum
import struct

verbose = False

def log(*args, **kwargs):
    if verbose:
        print(*args, *kwargs, file=sys.stderr)

def error(*args, **kwargs):
    print("mkmbn: ", file=sys.stderr, end='')
    print(*args, *kwargs, file=sys.stderr)

class SwId(Enum):
    sbl1 = 0x00
    mba = 0x01
    modem = 0x02
    prog = 0x03
    adsp = 0x04
    devcfg = 0x05
    tz = 0x07
    aboot = 0x09
    uefi = 0x09
    rpm = 0x0A
    tz_app = 0x0C
    wcnss = 0x0D
    venus = 0x0E
    wlanmdsp = 0x12
    gpu = 0x14
    hyp = 0x15
    cdsp = 0x17
    slpi = 0x18
    abl = 0x1C
    cmnlib = 0x1F
    aop = 0x21
    qup = 0x24
    xbl_config = 0x25

class MbnData:

    # sw_id 0x9 is aboot/uefi, the most common
    def __init__(self, loadaddr: int, version: int, sw_id: SwId = SwId.aboot):
        self.loadaddr = loadaddr
        self.version = version
        self.sw_id = sw_id


"""
This dictionary is used to map a board or platform to the appropriate load address and
other MBN metadata. When adding support for a new platform to U-Boot, the appropriate
data should be filled out here. The load address can typically be determined by looking
at the uefi.elf or xbl.elf for the platform. For the uefi.elf it is the load address, and
for xbl.elf it is typically the RWX section in the middle, just BEFORE the section loaded
at 0x1495xxxx or similar. Looking at similar platforms in the table below may help.
"""
boards: dict[bytes, MbnData] = {
    # Exact matches for boards, these are preferred
    b"qcom,qcs6490-rb3gen2\0": MbnData(0x9FC00000, 6, SwId.uefi),
    b"qcom,qcs9100-ride-r3\0": MbnData(0xAF000000, 6, SwId.uefi),  # Dragonwing IQ9
    b"qcom,qcs8300-ride\0": MbnData(0xAF000000, 6, SwId.uefi),  # Dragonwing IQ8
    b"qcom,qcs615-ride\0": MbnData(0x9FC00000, 6, SwId.uefi),  # Dragonwing IQ6
    # Fallback/generic matches since most boards for a platform will
    # use the same load address
    b"qcom,qcm6490\0": MbnData(0x9FC00000, 6, SwId.uefi),  # rb3gen2, rubikpi3
    b"qcom,qcs9100\0": MbnData(0xAF000000, 6, SwId.uefi),  # Dragonwing IQ9
    b"qcom,qcs8300\0": MbnData(0xAF000000, 6, SwId.uefi),  # Dragonwing IQ8
    b"qcom,qcs8550\0": MbnData(0xA7000000, 7, SwId.uefi),  # C8550
    b"qcom,sm8550\0": MbnData(0xA7000000, 7, SwId.uefi),  # C8550
    b"qcom,sm8650\0": MbnData(0xA7000000, 7, SwId.uefi),  # SM8650
    b"qcom,qcs615\0": MbnData(0x9FC00000, 6, SwId.uefi),  # Dragonwing IQ6
    b"qcom,ipq5424\0": MbnData(0x8a380000, 6, SwId.aboot),
    b"qcom,ipq9574\0": MbnData(0x4A240000, 6, SwId.aboot),

    # msm8916/apq8016 has an "aboot" partition but the process is the same
    # They use header version 3.
    b"qcom,apq8016\0": MbnData(0x8f600000, 3, SwId.aboot),
    b"qcom,msm8916\0": MbnData(0x8f600000, 3, SwId.aboot),
}

parser = argparse.ArgumentParser(
    description="""
	Create a signed Qualcomm "uefi" ELF image
"""
)
parser.register("type", "hex", lambda s: int(s, 16))
parser.add_argument(
    "-o", "--output", type=Path, default="u-boot.mbn", help="Output file"
)
parser.add_argument(
    "-v", dest="verbose", action="store_true", default=False, help="Verbose"
)
parser.add_argument(
    "bin", type=argparse.FileType("rb"), help="Binary to embed (e.g. u-boot.bin)"
)
args = parser.parse_args()
verbose = args.verbose

elf = Elf()

data: bytes = args.bin.read()

# dtb is at the end, so find the last match
dtb_off = 0
off = 0
dtb_size = 0
while True:
    off = data.find(b"\xd0\x0d\xfe\xed", dtb_off + dtb_size)
    if off == -1:
        break
    (dtb_size,) = struct.unpack_from("I", data, offset=off)
    dtb_off = off

if not dtb_off:
    print("Couldn't find DTB in provided binary!")
    exit(1)

log(f"Found FDT at {dtb_off:#x} size {dtb_size:#x}")

mbn: MbnData|None = None

for match, mbndata in boards.items():
    if data.find(match, dtb_off) != -1:
        mbn = mbndata
        break

if not mbn:
    error(
        "CONFIG_QCOM_GENERATE_MBN is enabled but this platform doesn't appear to be supported\n"
        "Please see tools/qcom/mkmbn/mkmbn.py for details. If you intend to chainload U-Boot\n"
        "then disregard this message and disable CONFIG_QCOM_GENERATE_MBN in your defconfig."
    )
    args.output.unlink(missing_ok=True)
    exit(1)

log(f"Detected board {match.decode('UTF-8')} with load address {mbn.loadaddr:#x}")

elf.phdrs.append(Phdr.from_bin(data, mbn.loadaddr))
elf.ehdr.e_entry = mbn.loadaddr
elf.update()

# QLI boards use v6 sw_id is "aboot"
hashseg.generate(elf, mbn.version, mbn.sw_id.value)
# print(f"after: {elf}")

with open(args.output, "wb") as f:
    elf.save(f)

log(f"Built signed MBN: {args.output.resolve()}")
