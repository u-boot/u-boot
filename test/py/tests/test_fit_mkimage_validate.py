# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2025
#
# Test that mkimage validates image references in configurations

import os
import subprocess
import pytest
import fit_util
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

def test_fit_load_addr_overlap(ubman):
    """Test that mkimage fails when load address overlap"""

    its_fname = fit_util.make_fname(ubman, "invalid.its")
    itb_fname = fit_util.make_fname(ubman, "invalid.itb")
    kernel = fit_util.make_kernel(ubman, 'kernel.bin', 'kernel')
    fdt = fit_util.make_dtb(ubman, '''
/dts-v1/;
/ {
    model = "Test FDT";
    compatible = "test";
};
''', 'test')

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
        fdt@1 {
            description = "Test FDT";
            data = /incbin/("test.dtb");
            type = "flat_dt";
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
            fdt = "fdt@1";
        };
    };
};
'''

    with open(its_fname, 'w') as f:
        f.write(its_text)

    mkimage = os.path.join(ubman.config.build_dir, 'tools/mkimage')
    cmd = [mkimage, '-f', its_fname, itb_fname]

    result = subprocess.run(cmd, capture_output=True, text=True)

    assert result.returncode != 0, "mkimage should fail due to memory overlap"
    assert "Error: Overlap detected:" in result.stderr
    # Check that it identifies the specific overlapping components
    assert "kernel@1" in result.stderr and "fdt@1" in result.stderr
