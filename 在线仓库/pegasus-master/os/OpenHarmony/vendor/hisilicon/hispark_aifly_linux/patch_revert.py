#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys
import re
import subprocess
import importlib

def read_yaml_file(input_file: str):
    if not os.path.isfile(input_file):
        print("[ERROR]: {} file not exist".format(input_file))

    yaml = importlib.import_module('yaml')
    with open(input_file, 'rt', encoding='utf-8') as yaml_file:
        try:
            return yaml.safe_load(yaml_file)
        except yaml.YAMLError as exc:
            if hasattr(exc, 'problem_mark'):
                mark = exc.problem_mark
                print(f'{input_file} load failed, error line ' f' {mark.line + 1}:{mark.column + 1}')
            else:
                print(f'{input_file} load failed')

def exec_command(cmd: list, log_path='patch_revert.log', exec_env=None, log_mode='normal', **kwargs):
    useful_info_pattern = re.compile(r'\[\d+/\d+\].+')
    is_log_filter = kwargs.pop('log_filter', False)
    if log_mode == 'silent':
        is_log_filter = True
    # if '' in cmd:
    #     cmd.remove('')
    if not os.path.exists(os.path.dirname(log_path)):
        os.makedirs(os.path.dirname(log_path), exist_ok=True)

    with open(log_path, 'at', encoding='utf-8') as log_file:
        print("start run hpm command ---", exec_env)
        try:
            process = subprocess.Popen(cmd,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.STDOUT,
                                        encoding='utf-8',
                                        env=exec_env,
                                        **kwargs)

            for line in iter(process.stdout.readline, ''):
                log_file.write(line)
                if is_log_filter:
                    info = re.findall(useful_info_pattern, line)
                    if len(info):
                        print("{} mode={}".format(info[0], log_mode))
                else:
                    print(line)
        except Exception as e:
            print(f"执行命令失败: {e}")

    process.wait()
    print("end hpm command")
    ret_code = process.returncode

    if ret_code != 0:
        print("err: ret_code = {}".format(ret_code))

class Patch():
    """Patch class for hb_internal.build --patch parameter
    Install the patch based on the configuration file.
    """
    def __init__(self):
        self.cur_dir = os.getcwd()
        self.root_path = os.path.join(self.cur_dir, '../../../')
        self.log_path = os.path.join(self.cur_dir, 'patch_revert.log')
        # Patch configuration file path
        self.patch_cfg = os.path.join(self.cur_dir, 'patch.yml')

    def modify_systemUtil(self):
        file_path = os.path.join(self.root_path, 'build/hb/util/system_util.py')
        with open(file_path, 'r') as file:
            lines = file.readlines()
        
        # 修改指定行
        print('line 54 ----- {}'.format(lines[54]))
        lines[54] = f'        while \"\" in cmd and type(cmd) == list:\n'
        
        # 写回文件
        with open(file_path, 'w') as file:
            file.writelines(lines)

    def modify_patch_process(self):
        file_path = os.path.join(self.root_path, 'build/hb/util/prebuild/patch_process.py')
        with open(file_path, 'r') as file:
            lines = file.readlines()

        lines[46] = f'            # self.patch_cache_update()\n'
         # 写回文件
        with open(file_path, 'w') as file:
            file.writelines(lines)                   

    def patch_make(self, cmd_args=None, reverse=True):
        """Patch installation entry function

        Args:
            reverse (bool, optional): True if the patch is rolled back,
                                      else False. Defaults to False.

        Raises:
            OHOSException: if configuration file does not exist.
        """

        if not os.path.isfile(self.patch_cfg):
            print("[ERROR]: {} patch_cfg not exist".format(self.patch_cfg))

        print("read yml file ----", self.patch_cfg)
        patch_cfg_dict = read_yaml_file(self.patch_cfg)
        for src_path, patch_list in patch_cfg_dict.items():
            self.patch_revert(src_path, patch_list, reverse)

        self.modify_systemUtil()
        print("modify_systemUtil ----")
        self.modify_patch_process()

    def patch_revert(self, src_path, patch_list, reverse=False):
        """Run the patch installation command.

        Args:
            src_path (string): Path to which the patch needs to be installed
            patch_list (list): List of paths for storing patches
            reverse (bool, optional): True if the patch is rolled back,
                                      else False. Defaults to False.

        Raises:
            OHOSException: src_path or patch_path is invalid,
                           or the command fails to be executed.
        """
        src_path = os.path.join(self.root_path, src_path)
        if not os.path.exists(src_path):
            print("[ERROR]: {} src_path not exist".format(src_path))

        cmd = ''
        if reverse:
            cmd = f'git reset --hard && git clean -df'
        # else:
        #     cmd = f'patch -p1 < {patch_path}'
        try:
            exec_command(cmd, log_path=self.log_path,
                            cwd=src_path, shell=True)
        except Exception as exception:
            # Failed to roll back the patch, clear patch cache
            print('[ERROR]: exec_command err.')
            raise exception
                
        # for patch_item in patch_list:
        #     patch_path = os.path.join(self.root_path, patch_item)
        #     if not os.path.isfile(patch_path):
        #         patch_path = os.path.join(self.root_path,
        #                                   src_path, patch_item)
        #         if not os.path.isfile(patch_path):
        #             print(f'patch {patch_item} not exist for {src_path}, stop applying patch')
                    
        #     if reverse:
        #         cmd = f'git apply -R {patch_path}'
        #     # else:
        #     #     cmd = f'patch -p1 < {patch_path}'
        #     try:
        #         exec_command(cmd, log_path=self.log_path,
        #                      cwd=src_path, shell=True)
        #     except Exception as exception:
        #         # Failed to roll back the patch, clear patch cache
        #         print('[ERROR]: exec_command err.')
        #         raise exception


if __name__ == "__main__":
    patch = Patch()
    patch.patch_make()
