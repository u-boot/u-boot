# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2016 Google, Inc
#

import command
import os
import shutil
import tempfile

import tout

# Output directly (generally this is temporary)
outdir = None

# True to keep the output directory around after exiting
preserve_outdir = False

# Path to the Chrome OS chroot, if we know it
chroot_path = None

# Search paths to use for Filename(), used to find files
search_paths = []


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

    raise ValueError("Filename '%s' not found in input path (%s) (cwd='%s')" %
                     (fname, ','.join(indir), os.getcwd()))

def Align(pos, align):
    if align:
        mask = align - 1
        pos = (pos + mask) & ~mask
    return pos

def NotPowerOfTwo(num):
    return num and (num & (num - 1))

def Run(name, *args):
    command.Run(name, *args, cwd=outdir)

def Filename(fname):
    """Resolve a file path to an absolute path.

    If fname starts with ##/ and chroot is available, ##/ gets replaced with
    the chroot path. If chroot is not available, this file name can not be
    resolved, `None' is returned.

    If fname is not prepended with the above prefix, and is not an existing
    file, the actual file name is retrieved from the passed in string and the
    search_paths directories (if any) are searched to for the file. If found -
    the path to the found file is returned, `None' is returned otherwise.

    Args:
      fname: a string,  the path to resolve.

    Returns:
      Absolute path to the file or None if not found.
    """
    if fname.startswith('##/'):
      if chroot_path:
        fname = os.path.join(chroot_path, fname[3:])
      else:
        return None

    # Search for a pathname that exists, and return it if found
    if fname and not os.path.exists(fname):
        for path in search_paths:
            pathname = os.path.join(path, os.path.basename(fname))
            if os.path.exists(pathname):
                return pathname

    # If not found, just return the standard, unchanged path
    return fname

def ReadFile(fname):
    """Read and return the contents of a file.

    Args:
      fname: path to filename to read, where ## signifiies the chroot.

    Returns:
      data read from file, as a string.
    """
    with open(Filename(fname), 'rb') as fd:
        data = fd.read()
    #self._out.Info("Read file '%s' size %d (%#0x)" %
                   #(fname, len(data), len(data)))
    return data

def WriteFile(fname, data):
    """Write data into a file.

    Args:
        fname: path to filename to write
        data: data to write to file, as a string
    """
    #self._out.Info("Write file '%s' size %d (%#0x)" %
                   #(fname, len(data), len(data)))
    with open(Filename(fname), 'wb') as fd:
        fd.write(data)
