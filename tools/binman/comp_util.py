# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Utilities to compress and decompress data"""

import struct
import tempfile

from patman import tools

def compress(indata, algo, with_header=True):
    """Compress some data using a given algorithm

    Note that for lzma this uses an old version of the algorithm, not that
    provided by xz.

    This requires 'lz4' and 'lzma_alone' tools. It also requires an output
    directory to be previously set up, by calling PrepareOutputDir().

    Care is taken to use unique temporary files so that this function can be
    called from multiple threads.

    Args:
        indata (bytes): Input data to compress
        algo (str): Algorithm to use ('none', 'gzip', 'lz4' or 'lzma')

    Returns:
        bytes: Compressed data
    """
    if algo == 'none':
        return indata
    fname = tempfile.NamedTemporaryFile(prefix='%s.comp.tmp' % algo,
                                        dir=tools.GetOutputDir()).name
    tools.WriteFile(fname, indata)
    if algo == 'lz4':
        data = tools.Run('lz4', '--no-frame-crc', '-B4', '-5', '-c', fname,
                         binary=True)
    # cbfstool uses a very old version of lzma
    elif algo == 'lzma':
        outfname = tempfile.NamedTemporaryFile(prefix='%s.comp.otmp' % algo,
                                               dir=tools.GetOutputDir()).name
        tools.Run('lzma_alone', 'e', fname, outfname, '-lc1', '-lp0', '-pb0',
                  '-d8')
        data = tools.ReadFile(outfname)
    elif algo == 'gzip':
        data = tools.Run('gzip', '-c', fname, binary=True)
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
        algo (str): Algorithm to use ('none', 'gzip', 'lz4' or 'lzma')

    Returns:
        (bytes) Compressed data
    """
    if algo == 'none':
        return indata
    if with_header:
        data_len = struct.unpack('<I', indata[:4])[0]
        indata = indata[4:4 + data_len]
    fname = tools.GetOutputFilename('%s.decomp.tmp' % algo)
    tools.WriteFile(fname, indata)
    if algo == 'lz4':
        data = tools.Run('lz4', '-dc', fname, binary=True)
    elif algo == 'lzma':
        outfname = tools.GetOutputFilename('%s.decomp.otmp' % algo)
        tools.Run('lzma_alone', 'd', fname, outfname)
        data = tools.ReadFile(outfname, binary=True)
    elif algo == 'gzip':
        data = tools.Run('gzip', '-cd', fname, binary=True)
    else:
        raise ValueError("Unknown algorithm '%s'" % algo)
    return data
