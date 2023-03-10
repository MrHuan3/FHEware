# -*- encoding: utf-8 -*-
# File Name: ID
# Author: 瓛
# @Time: 2023/1/11 18:29 1月

import os.path
import time

from ClientSocket import ClientSend, ClientReceive

class ID:

    def __init__(self):
        self.ID = ''

    def readID(self) -> str:
        # get client ID
        if not os.path.exists('../doc/ID.txt'):
            # have not yet access the server
            cs = ClientSend()
            # get ID from server
            msg = '0000000000:gID:Null'
            cs.SendMsg(msg)
            time.sleep(0.05)
            cs = ClientReceive()
            self.ID = cs.ReceiveIndex()
            print("Get ID: " + self.ID)
            if self.ID == 'Error':
                exit(5)
            else:
                # save ID
                IDtxt = open('../doc/ID.txt', 'w', encoding='utf-8')
                IDtxt.write(self.ID)
                # close txt
                IDtxt.close()
                print("ID saved")
        else:
            # already have ID
            IDtxt = open('../doc/ID.txt', 'r', encoding='utf-8')
            self.ID = IDtxt.read()
            # close txt
            IDtxt.close()
            print("ID: " + self.ID)
        return self.ID
