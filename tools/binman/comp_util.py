# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Utilities to compress and decompress data"""

import struct
import tempfile

from binman import bintool
from patman import tools

LZ4 = bintool.Bintool.create('lz4')
HAVE_LZ4 = LZ4.is_present()

LZMA_ALONE = bintool.Bintool.create('lzma_alone')
HAVE_LZMA_ALONE = LZMA_ALONE.is_present()


def compress(indata, algo, with_header=True):
    """Compress some data using a given algorithm

    Note that for lzma this uses an old version of the algorithm, not that
    provided by xz.

    This requires 'lz4' and 'lzma_alone' tools. It also requires an output
    directory to be previously set up, by calling PrepareOutputDir().

    Args:
        indata (bytes): Input data to compress
        algo (str): Algorithm to use ('none', 'lz4' or 'lzma')

    Returns:
        bytes: Compressed data
    """
    if algo == 'none':
        return indata
    if algo == 'lz4':
        data = LZ4.compress(indata)
    # cbfstool uses a very old version of lzma
    elif algo == 'lzma':
        data = LZMA_ALONE.compress(indata)
    else:
        raise ValueError("Unknown algorithm '%s'" % algo)
    if with_header:
        hdr = struct.pack('<I', len(data))
        data = hdr + data
    return data

def decompress(indata, algo, with_header=True):
    """Decompress some data using a given algorithm

    Note that for lzma this uses an old version of the algorithm, not that
    provided by xz.

    This requires 'lz4' and 'lzma_alone' tools. It also requires an output
    directory to be previously set up, by calling PrepareOutputDir().

    Args:
        indata (bytes): Input data to decompress
        algo (str): Algorithm to use ('none', 'lz4' or 'lzma')

    Returns:
        (bytes) Compressed data
    """
    if algo == 'none':
        return indata
    if with_header:
        data_len = struct.unpack('<I', indata[:4])[0]
        indata = indata[4:4 + data_len]
    if algo == 'lz4':
        data = LZ4.decompress(indata)
    elif algo == 'lzma':
        data = LZMA_ALONE.decompress(indata)
    else:
        raise ValueError("Unknown algorithm '%s'" % algo)
    return data
