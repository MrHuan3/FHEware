# -*- encoding: utf-8 -*-
# File Name: server
# Author: 瓛
# @Time: 2023/1/12 09:11 1月

import os

class FHEserver:
    def server(self, video_name, clientID):
        # blind extract frame cipher
        cmd = './Server ' + video_name + ' ' + clientID
        os.system(cmd)