# -*- encoding: utf-8 -*-
# File Name: delete
# Author: 瓛
# @Time: 2023/1/12 11:01 1月

import os
import shutil

class Delete:

    def delete(self, video_name, clientID) -> str:
        # define client path
        path = "../ServerPool/S2C/" + clientID
        if os.path.exists(path):
            # client fold existed
            fn_path = path + '/' + video_name
            if os.path.exists(fn_path):
                # delete selected fold
                shutil.rmtree(fn_path)
                return "Delete"
            else:
                return "VideoNotFound"
        else:
            # client fold not existed
            return "VideoNotFound"
        return "UndefinedError"
        

