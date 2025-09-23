# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2025 NXP

import os
from binman.etype.mkimage import Entry_mkimage
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_nxp_imx9image(Entry_mkimage):
    """NXP i.MX9 .bin configuration file generator and mkimage invocation

    Properties arguments:
        - append: appends the specified blob file as-is
        - boot-from: indicates the boot device to be used
        - cntr-version: sets the image container format version
        - container: indicates that it is a new container
        - dummy-ddr: specifies the memory address for storing the DDR training
                     data image
        - dummy-v2x: specifies the memory address for storing V2X firmware
        - hold: reserves a specified number of bytes after the previous image
        - image: defines the option type, image filename, and the memory address
                 where the image will be stored
        - soc-type: specifies the target SoC for which the .bin file is generated
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.config_filename = None

    def ReadNode(self):
        super().ReadNode()
        cfg_path = fdt_util.GetString(self._node, 'cfg-path')
        self.config_filename = tools.get_output_filename(cfg_path)
        accepted_keys = [
            'append', 'boot-from', 'cntr-version', 'container', 'dummy-ddr',
            'dummy-v2x', 'hold', 'image', 'soc-type'
        ]
        external_files = ['oei-m33-tcm.bin', 'm33_image.bin', 'bl31.bin']

        with open(self.config_filename, 'w', encoding='utf-8') as f:
            for prop in self._node.props.values():
                key = prop.name
                value = prop.value

                if not any(key.startswith(prefix) for prefix in accepted_keys):
                    continue

                formatted_key = key.replace('-', '_')

                if key.startswith('image') and isinstance(value, list) and len(value) == 3:
                    file = value[1]
                    image_path = os.path.join(tools.get_output_dir(), value[1])
                    value[1] = image_path
                    combined = ' '.join(map(str, value))

                    if file in external_files and not os.path.exists(value[1]):
                        print(f"file '{image_path}' does not exist. flash.bin may be not-functional.")
                    else:
                        f.write(f'image {combined}\n')
                elif isinstance(value, str):
                    if key.startswith('append'):
                        file_path = os.path.join(tools.get_output_dir(), value)
                        if os.path.exists(file_path):
                            f.write(f'append {file_path}\n')
                        else:
                            print(f"file '{file_path}' does not exist. flash.bin may be not-functional.")
                    else:
                        f.write(f'{formatted_key} {value}\n')
                elif isinstance(value, bool):
                    f.write(f'{formatted_key}\n')
                elif isinstance(value, bytes):
                    hex_value = hex(int.from_bytes(value, byteorder='big'))
                    f.write(f'{formatted_key} {hex_value}\n')

    def BuildSectionData(self, required):
        data, input_fname, uniq = self.collect_contents_to_file(self._entries.values(), 'imx9image')

        outfile = f'imx9image-out.{uniq}'
        output_fname = tools.get_output_filename(outfile)

        args = [
            '-d', input_fname, '-n', self.config_filename, '-T', 'imx8image',
            output_fname
        ]

        result = self.mkimage.run_cmd(*args)
        if result is not None and os.path.exists(output_fname):
            return tools.read_file(output_fname)
