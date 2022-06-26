# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
#
# TI Board Configuration Class for Schema Validation and Binary Generation
#

import os
import getopt
import sys

import yaml

from jsonschema import validate


class TIBoardConfig:

    """ Texas Instruments Board Configuration File"""

    def __init__(self, file, schema, data_rules=""):
        """Load a YAML configuration file and YAML schema

        Validation of the config file against the schema is also done."""
        with open(file, 'r') as f:
            self.file_yaml = yaml.safe_load(f)
        with open(schema, 'r') as sch:
            self.schema_yaml = yaml.safe_load(sch)
        self.data_rules = data_rules
        try:
            validate(self.file_yaml, self.schema_yaml)
        except Exception as e:
            print(e)

    def _convert_to_byte_chunk(self, val, data_type):
        """Convert value into byte array"""
        size = 0
        if(data_type == "#/definitions/u8"):
            size = 1
        elif(data_type == "#/definitions/u16"):
            size = 2
        elif(data_type == "#/definitions/u32"):
            size = 4
        else:
            raise Exception("Data type not present in definitions")
        if type(val) == int:
            br = val.to_bytes(size, byteorder="little")
        return br

    def _compile_yaml(self, schema_yaml, file_yaml):
        """Convert YAML file into byte array based on YAML schema"""
        br = bytearray()
        for key in file_yaml.keys():
            node = file_yaml[key]
            node_schema = schema_yaml['properties'][key]
            node_type = node_schema.get('type')
            if not 'type' in node_schema:
                br += self._convert_to_byte_chunk(node,
                                                  node_schema.get('$ref'))
            elif node_type == 'object':
                br += self._compile_yaml(node_schema, node)
            elif node_type == 'array':
                for item in node:
                    if not isinstance(item, dict):
                        br += self._convert_to_byte_chunk(
                            item, schema_yaml['properties'][key]['items']["$ref"])
                    else:
                        br += self._compile_yaml(node_schema.get('items'), item)
        return br

    def generate_binaries(self, out_path=""):
        """Generate config binary artifacts from the loaded YAML configuration file"""
        if not os.path.isdir(out_path):
            os.mkdir(out_path)
        for key in self.file_yaml.keys():
            node = self.file_yaml[key]
            node_schema = self.schema_yaml['properties'][key]
            br = self._compile_yaml(node_schema, node)
            path = os.path.join(out_path, key + ".bin")
            with open(path, 'wb') as cfg:
                cfg.write(br)

    def delete_binaries(self, out_path=""):
        """Delete generated binaries"""
        if os.path.isdir(out_path):
            for key in self.file_yaml.keys():
                path = os.path.join(out_path, key + ".bin")
                if os.path.isfile(path):
                    os.remove(path)


def cfgBinaryGen():
    """Generate config binaries from YAML config file and YAML schema
        Arguments:
            - config_yaml: board config file in YAML
            - schema_yaml: schema file in YAML to validate config_yaml against
            - output_dir: output directory where generated binaries can be populated
    Pass the arguments along with the filename in the Makefile.
    """
    opts, args = getopt.getopt(sys.argv[1:], "c:s:o")
    for opt, val in opts:
        if opt == "-c":
            config_yaml = val
        elif opt == "-s":
            schema_yaml = val
        elif opt == "-o":
            output_dir = os.path.abspath(val)
    try:
        tibcfg = TIBoardConfig(config_yaml, schema_yaml)
        tibcfg.generate_binaries(output_dir)
    except:
        raise ValueError("Could not find config files!")


cfgBinaryGen()
