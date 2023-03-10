# -*- encoding: utf-8 -*-
# File Name: ware
# Author: 瓛
# @Time: 2023/1/12 15:45 1月

import os


class Ware:

    def BeforeClient(self, video_name):
        # start encryption
        cmd = './BeforeClient ' + video_name
        os.system(cmd)

    def AfterClient(self, video_name):
        # start decryption
        cmd = './AfterClient ' + video_name
        os.system(cmd)