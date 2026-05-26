# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2025
#
# Test that mkimage validates image references in configurations

import os
import subprocess
import pytest
import fit_util
import utils
import re

@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
def test_fit_invalid_image_reference(ubman):
    """Test that mkimage fails when configuration references a missing image"""

    its_fname = fit_util.make_fname(ubman, "invalid.its")
    itb_fname = fit_util.make_fname(ubman, "invalid.itb")
    kernel = fit_util.make_kernel(ubman, 'kernel.bin', 'kernel')

    # Write ITS with an invalid reference to a nonexistent image
    its_text = '''
/dts-v1/;

/ {
    images {
        kernel@1 {
            description = "Test Kernel";
            data = /incbin/("kernel.bin");
            type = "kernel";
            arch = "sandbox";
            os = "linux";
            compression = "none";
            load = <0x40000>;
            entry = <0x40000>;
        };
    };

    configurations {
        default = "conf@1";
        conf@1 {
            kernel = "kernel@1";
            fdt = "notexist";
        };
    };
};
'''

    with open(its_fname, 'w') as f:
        f.write(its_text)

    mkimage = os.path.join(ubman.config.build_dir, 'tools/mkimage')
    cmd = [mkimage, '-f', its_fname, itb_fname]

    result = subprocess.run(cmd, capture_output=True, text=True)

    assert result.returncode != 0, "mkimage should fail due to missing image reference"
    assert "references undefined image 'notexist'" in result.stderr

@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
def test_fit_invalid_default_config(ubman):
    """Test that mkimage fails when default config is missing"""

    its_fname = fit_util.make_fname(ubman, "invalid.its")
    itb_fname = fit_util.make_fname(ubman, "invalid.itb")
    kernel = fit_util.make_kernel(ubman, 'kernel.bin', 'kernel')

    # Write ITS with an invalid reference to a nonexistent default config
    its_text = '''
/dts-v1/;

/ {
    images {
        kernel@1 {
            description = "Test Kernel";
            data = /incbin/("kernel.bin");
            type = "kernel";
            arch = "sandbox";
            os = "linux";
            compression = "none";
            load = <0x40000>;
            entry = <0x40000>;
        };
    };

    configurations {
        default = "conf@1";
        conf@2 {
            kernel = "kernel@1";
        };
    };
};
'''

    with open(its_fname, 'w') as f:
        f.write(its_text)

    mkimage = os.path.join(ubman.config.build_dir, 'tools/mkimage')
    cmd = [mkimage, '-f', its_fname, itb_fname]

    result = subprocess.run(cmd, capture_output=True, text=True)

    assert result.returncode != 0, "mkimage should fail due to missing default config"
    assert re.search(r"Default configuration '.*' not found under /configurations", result.stderr)

@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtget')
@pytest.mark.parametrize('dtb_relpath,expected_desc', [
    # Crash triggers: last '.' precedes last '/', or leaf has no extension.
    ('./mydt',       'mydt'),
    ('./sub.d/leaf', 'leaf'),
    ('./a.b/c',      'c'),
    # Control case: extension lives in the leaf, no dotted directory.
    ('./mydt.dtb',   'mydt'),
])
def test_fit_auto_basename_dotted_directory(ubman, dtb_relpath, expected_desc):
    """Regression test: mkimage -f auto must not crash when a -b path has a
    '.' in its directory portion.

    Before the fix, get_basename() in tools/fit_image.c searched the whole
    path for both the last '/' and the last '.'. When the '.' fell before
    the '/', the computed length went negative and was passed unchanged to
    memcpy(), which segfaulted. This test exercises three crashing paths
    plus one control input.
    """
    build_dir = ubman.config.build_dir
    kernel = fit_util.make_kernel(ubman, 'kernel.bin', 'kernel')
    itb_fname = fit_util.make_fname(ubman, 'auto_basename.itb')

    # Materialize the dtb at the requested relative path inside build_dir.
    dtb_abs = os.path.join(build_dir, dtb_relpath)
    os.makedirs(os.path.dirname(dtb_abs), exist_ok=True)
    with open(dtb_abs, 'wb') as f:
        f.write(b'dummy')

    cmd = ['./tools/mkimage', '-f', 'auto',
           '-A', 'arm', '-O', 'linux', '-T', 'kernel', '-C', 'none',
           '-a', '0x80000000', '-e', '0x80000000', '-n', 'test',
           '-d', kernel,
           '-b', dtb_relpath,
           itb_fname]
    # Run with cwd=build_dir so both ./tools/mkimage and the relative -b
    # path resolve the same way the bug originally reproduced.
    result = subprocess.run(cmd, capture_output=True, text=True,
                            cwd=build_dir)

    assert result.returncode == 0, (
        f"mkimage crashed or failed on -b {dtb_relpath!r}: "
        f"rc={result.returncode}\nstdout:\n{result.stdout}\n"
        f"stderr:\n{result.stderr}"
    )
    # The fdt sub-image description is set from get_basename(). Read it back
    # from the produced FIT (a device tree) rather than parsing mkimage's
    # console output.
    desc = utils.run_and_log(
        ubman, ['fdtget', itb_fname, '/images/fdt-1', 'description']).strip()
    assert desc == expected_desc, (
        f"Expected /images/fdt-1 description {expected_desc!r}, got {desc!r}"
    )
