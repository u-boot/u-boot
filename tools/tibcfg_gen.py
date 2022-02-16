# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
#
# TI Board Configuration Class for Schema Validation and Binary Generation
#

from jsonschema import validate

import yaml
import os
import sys


class TIBoardConfig:
    file_yaml = {}
    schema_yaml = {}
    data_rules = {}

    def __init__(self):
        pass

    def Load(self, file, schema, data_rules=""):
        with open(file, 'r') as f:
            self.file_yaml = yaml.safe_load(f)
        with open(schema, 'r') as sch:
            self.schema_yaml = yaml.safe_load(sch)
        self.data_rules = data_rules

    def CheckValidity(self):
        try:
            validate(self.file_yaml, self.schema_yaml)
            return True
        except Exception as e:
            print(e)
            return False

    def __ConvertToByteChunk(self, val, data_type):
        br = []
        size = 0
        if(data_type == "#/definitions/u8"):
            size = 1
        elif(data_type == "#/definitions/u16"):
            size = 2
        elif(data_type == "#/definitions/u32"):
            size = 4
        else:
            return -1
        if(type(val) == int):
            while(val != 0):
                br = br + [(val & 0xFF)]
                val = val >> 8
        while(len(br) < size):
            br = br + [0]
        return br

    def __CompileYaml(self, schema_yaml, file_yaml):
        br = []
        for key in file_yaml.keys():
            if not 'type' in schema_yaml['properties'][key]:
                br = br + \
                    self.__ConvertToByteChunk(
                        file_yaml[key], schema_yaml['properties'][key]["$ref"])
            elif schema_yaml['properties'][key]['type'] == 'object':
                br = br + \
                    self.__CompileYaml(
                        schema_yaml['properties'][key], file_yaml[key])
            elif schema_yaml['properties'][key]['type'] == 'array':
                for item in file_yaml[key]:
                    if not isinstance(item, dict):
                        br = br + \
                            self.__ConvertToByteChunk(
                                item, schema_yaml['properties'][key]['items']["$ref"])
                    else:
                        br = br + \
                            self.__CompileYaml(
                                schema_yaml['properties'][key]['items'], item)
        return br

    def GenerateBinaries(self, out_path=""):
        if not os.path.isdir(out_path):
            os.mkdir(out_path)
        if(self.CheckValidity()):
            for key in self.file_yaml.keys():
                br = []
                br = self.__CompileYaml(
                    self.schema_yaml['properties'][key], self.file_yaml[key])
                with open(out_path + "/" + key + ".bin", 'wb') as cfg:
                    cfg.write(bytearray(br))
        else:
            raise ValueError("Config YAML Validation failed!")

    def DeleteBinaries(self, out_path=""):
        if os.path.isdir(out_path):
            for key in self.file_yaml.keys():
                if os.path.isfile(out_path + "/" + key + ".bin"):
                    os.remove(out_path + "/" + key + ".bin")


def cfgBinaryGen():
    """Generate config binaries from YAML config file and YAML schema
        Arguments:
            - config_yaml: board config file in YAML
            - schema_yaml: schema file in YAML to validate config_yaml against
    Pass the arguments along with the filename in the Makefile.
    """
    tibcfg = TIBoardConfig()
    config_yaml = sys.argv[1]
    schema_yaml = sys.argv[2]
    try:
        tibcfg.Load(config_yaml, schema_yaml)
    except:
        raise ValueError("Could not find config files!")
    tibcfg.GenerateBinaries(os.environ['O'])


cfgBinaryGen()
