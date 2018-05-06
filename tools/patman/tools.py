# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2016 Google, Inc
#

import os
import shutil
import tempfile

import tout

outdir = None
indirs = None
preserve_outdir = False

def PrepareOutputDir(dirname, preserve=False):
    """Select an output directory, ensuring it exists.

    This either creates a temporary directory or checks that the one supplied
    by the user is valid. For a temporary directory, it makes a note to
    remove it later if required.

    Args:
        dirname: a string, name of the output directory to use to store
                intermediate and output files. If is None - create a temporary
                directory.
        preserve: a Boolean. If outdir above is None and preserve is False, the
                created temporary directory will be destroyed on exit.

    Raises:
        OSError: If it cannot create the output directory.
    """
    global outdir, preserve_outdir

    preserve_outdir = dirname or preserve
    if dirname:
        outdir = dirname
        if not os.path.isdir(outdir):
            try:
                os.makedirs(outdir)
            except OSError as err:
                raise CmdError("Cannot make output directory '%s': '%s'" %
                                (outdir, err.strerror))
        tout.Debug("Using output directory '%s'" % outdir)
    else:
        outdir = tempfile.mkdtemp(prefix='binman.')
        tout.Debug("Using temporary directory '%s'" % outdir)

def _RemoveOutputDir():
    global outdir

    shutil.rmtree(outdir)
    tout.Debug("Deleted temporary directory '%s'" % outdir)
    outdir = None

def FinaliseOutputDir():
    global outdir, preserve_outdir

    """Tidy up: delete output directory if temporary and not preserved."""
    if outdir and not preserve_outdir:
        _RemoveOutputDir()

def GetOutputFilename(fname):
    """Return a filename within the output directory.

    Args:
        fname: Filename to use for new file

    Returns:
        The full path of the filename, within the output directory
    """
    return os.path.join(outdir, fname)

def _FinaliseForTest():
    """Remove the output directory (for use by tests)"""
    global outdir

    if outdir:
        _RemoveOutputDir()

def SetInputDirs(dirname):
    """Add a list of input directories, where input files are kept.

    Args:
        dirname: a list of paths to input directories to use for obtaining
                files needed by binman to place in the image.
    """
    global indir

    indir = dirname
    tout.Debug("Using input directories %s" % indir)

def GetInputFilename(fname):
    """Return a filename for use as input.

    Args:
        fname: Filename to use for new file

    Returns:
        The full path of the filename, within the input directory
    """
    if not indir:
        return fname
    for dirname in indir:
        pathname = os.path.join(dirname, fname)
        if os.path.exists(pathname):
            return pathname

    raise ValueError("Filename '%s' not found in input path (%s)" %
                     (fname, ','.join(indir)))

def Align(pos, align):
    if align:
        mask = align - 1
        pos = (pos + mask) & ~mask
    return pos

def NotPowerOfTwo(num):
    return num and (num & (num - 1))
