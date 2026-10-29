# SPDX-License-Identifier: GPL-2.0+
# Copyright 2026 Canonical Ltd.
#
# Test mkimage import of external data in fit_import_data()

"""Regression tests for fit_import_data().

The import loop used to keep the data pointer and external property name
of the previous image, so an image node carrying data-size but neither
data-offset nor data-position imported the previous image's data and
then made mkimage abort without printing any diagnostic. Such a node
must be skipped by the import and reported by the later processing
stages instead.

The import also read each image's external data at the offset and size
the FIT declares without checking them against the file, so a region
reaching past the end of the file crashed mkimage or imported unrelated
memory as image data. Such a region must be rejected before it is read.
"""

import os
import struct
import subprocess

import pytest

import fit_util

BASE_ITS = '''
/dts-v1/;

/ {
    description = "import-data test";

    images {
        kernel-1 {
            description = "first kernel";
            data = /incbin/("%(kernel1)s");
            type = "kernel";
            arch = "sandbox";
            os = "linux";
            compression = "none";
            load = <0x40000>;
            entry = <0x40000>;
        };
        kernel-2 {
            description = "second kernel";
            data = /incbin/("%(kernel2)s");
            type = "kernel";
            arch = "sandbox";
            os = "linux";
            compression = "none";
            load = <0x80000>;
            entry = <0x80000>;
        };
    };

    configurations {
        default = "conf-1";
        conf-1 {
            kernel = "kernel-1";
        };
    };
};
'''


def fdtput_keep_data(itb, *args, opts=()):
    """Run fdtput on a FIT while keeping its external data

    fdtput rewrites only the device tree and drops any external data that
    follows it, so save that payload first and append it again afterwards
    at the 4-byte alignment fit_import_data() expects.

    Args:
        itb (str): Filename of the FIT to edit
        args (list of str): Node, property and values to pass to fdtput
        opts (list of str): Options to pass before the filename
    """
    with open(itb, 'rb') as inf:
        blob = inf.read()
    totalsize = struct.unpack('>I', blob[4:8])[0]
    payload = blob[(totalsize + 3) & ~3:]

    subprocess.run(['fdtput'] + list(opts) + [itb] + list(args), check=True)

    with open(itb, 'rb') as inf:
        fdt = inf.read()
    totalsize = struct.unpack('>I', fdt[4:8])[0]
    with open(itb, 'wb') as outf:
        outf.write(fdt.ljust((totalsize + 3) & ~3, b'\0') + payload)


@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtput')
def test_fit_import_data_missing_offset(ubman):
    """An image with data-size but no data-offset must not inherit data"""
    mkimage = os.path.join(ubman.config.build_dir, 'tools/mkimage')
    params = {
        'kernel1': fit_util.make_kernel(ubman, 'imp-kernel1.bin', 'first'),
        'kernel2': fit_util.make_kernel(ubman, 'imp-kernel2.bin', 'second'),
    }
    its = fit_util.make_its(ubman, BASE_ITS, params, 'imp.its')
    itb = fit_util.make_fname(ubman, 'imp.itb')

    result = subprocess.run([mkimage, '-E', '-f', its, itb],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr

    # Remove the offset so that only data-size is left on kernel-2
    fdtput_keep_data(itb, '/images/kernel-2', 'data-offset', opts=['-d'])

    # Re-processing must skip the malformed image in the import, so that
    # a later stage reports it; previously the stale pointer made the
    # import write kernel-1's data into kernel-2 and abort silently
    result = subprocess.run([mkimage, '-F', itb],
                            capture_output=True, text=True)
    assert result.returncode != 0
    assert "Can't get image data/size" in result.stderr


@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtput')
def test_fit_import_data_out_of_range(ubman):
    """External data outside the file must be rejected, not read"""
    mkimage = os.path.join(ubman.config.build_dir, 'tools/mkimage')
    params = {
        'kernel1': fit_util.make_kernel(ubman, 'oob-kernel1.bin', 'first'),
        'kernel2': fit_util.make_kernel(ubman, 'oob-kernel2.bin', 'second'),
    }
    its = fit_util.make_its(ubman, BASE_ITS, params, 'oob.its')
    itb = fit_util.make_fname(ubman, 'oob.itb')

    result = subprocess.run([mkimage, '-E', '-f', its, itb],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr

    # Point kernel-2's data far past the end of the file
    fdtput_keep_data(itb, '/images/kernel-2', 'data-offset', '0x40000000',
                     opts=['-t', 'i'])
    result = subprocess.run([mkimage, '-F', itb],
                            capture_output=True, text=True)
    assert result.returncode != 0
    assert "image 'kernel-2': external data" in result.stderr
    assert 'outside the FIT file' in result.stderr

    # A size running just past the end of the file must be rejected too.
    # kernel-2's data comes last, padded to 4 bytes, so 4 bytes more than
    # its real size always reach past the end.
    result = subprocess.run([mkimage, '-E', '-f', its, itb],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    size = os.path.getsize(params['kernel2']) + 4
    fdtput_keep_data(itb, '/images/kernel-2', 'data-size', f'{size:#x}',
                     opts=['-t', 'i'])
    result = subprocess.run([mkimage, '-F', itb],
                            capture_output=True, text=True)
    assert result.returncode != 0
    assert "image 'kernel-2': external data" in result.stderr

    # An empty region has nothing to read, so it is accepted even past the
    # end of the file, which is where mkimage -E -p places it when every
    # image is empty
    for name in ('kernel1', 'kernel2'):
        params[name] = fit_util.make_fname(ubman, f'oob-empty-{name}.bin')
        with open(params[name], 'wb'):
            pass
    its = fit_util.make_its(ubman, BASE_ITS, params, 'oob-empty.its')
    result = subprocess.run([mkimage, '-E', '-p', '0x1000', '-f', its, itb],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    assert os.path.getsize(itb) < 0x1000
    result = subprocess.run([mkimage, '-F', itb],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr


@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtput')
def test_fit_import_data_prop_len(ubman):
    """An external data property that is not exactly one cell is not read"""
    mkimage = os.path.join(ubman.config.build_dir, 'tools/mkimage')
    params = {
        'kernel1': fit_util.make_kernel(ubman, 'len-kernel1.bin', 'first'),
        'kernel2': fit_util.make_kernel(ubman, 'len-kernel2.bin', 'second'),
    }
    its = fit_util.make_its(ubman, BASE_ITS, params, 'len.its')
    itb = fit_util.make_fname(ubman, 'len.itb')

    # A property that is not exactly one cell must be skipped like a
    # missing one, so that mkimage finds no data for the image and fails.
    # An empty one must not be read from the bytes that follow it, and one
    # of two cells must not be taken from its first cell.
    for prop in ('data-offset', 'data-size'):
        for cells in ([], ['0', '0']):
            result = subprocess.run([mkimage, '-E', '-f', its, itb],
                                    capture_output=True, text=True)
            assert result.returncode == 0, result.stderr
            fdtput_keep_data(itb, '/images/kernel-2', prop, *cells,
                             opts=['-t', 'i'])
            result = subprocess.run([mkimage, '-F', itb],
                                    capture_output=True, text=True)
            assert result.returncode != 0
            assert "Can't get image data/size" in result.stderr
