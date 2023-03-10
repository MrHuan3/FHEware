# -*- encoding: utf-8 -*-
# File Name: FHEserver
# Author: 瓛
# @Time: 2023/1/12 15:13 1月

import os
import os.path
import socket
import shutil
import time
import ssl

from sm3 import SM3
from server import FHEserver
from delete import Delete


class ServerReceive:
    def __init__(self):
        # define locale IP address
        self.host = "0.0.0.0"
        # define locale port
        self.port = 3389

    def run(self):
        print("Waiting for connection")
        # get message operation head
        get_head = self.read_head()
        if get_head.split(':')[1] == 'gID':
            print(get_head)
            # request for ID
            sm = SM3()
            # get current time
            msg = str(time.time()) + get_head.split(':')[2]
            # generate client ID with SM3
            ID = sm.generate_ID(msg)
            # judge whether new ID existed
            exist = True
            while exist:
                # find whether this ID fold existed
                exist = os.path.exists("..ServerPool/S2C/" + ID)
                if exist:
                    # rehash
                    ID = sm.generate_ID(ID + str(time.time()))
                else:
                    # ID is unique
                    # send client ID back
                    ss = ServerSend()
                    ss.SendMsg(ID)
            # remark a log
            log = open('../doc/operation.log', 'a', encoding='utf-8')
            # exact time, operation, video name are in log
            log_msg = time.strftime('%Y/%m/%d-%H:%M:%S', time.localtime(time.time())) + '--getID--' + ID + '\n'
            log.write(log_msg)
            log.close()
        elif get_head.split(':')[1] == 'Upl':
            print(get_head)
            # request for uploading
            # receive client ID
            clientID = get_head.split(':')[0]
            # receive video name
            video_name = get_head.split(':')[2]
            print("Receive: " + video_name)
            # receive frame number
            frame_num = self.ReceiveIndex()
            print("Receive: " + frame_num)
            # remark a log
            log = open('../doc/operation.log', 'a', encoding='utf-8')
            # exact time, operation, video name are in log
            log_msg = time.strftime('%Y/%m/%d-%H:%M:%S', time.localtime(time.time())) + '--Upload--' + clientID + '--' + video_name + '--' + frame_num + '--begin\n'
            log.write(log_msg)
            log.close()
            fn_path = "../ServerPool/FrameNumber/" + video_name + ".txt"
            # write frame number into txt
            cmd1 = './Write ' + fn_path + '  wb  ' + frame_num
            os.system(cmd1)
            print("Save frame number")
            # receive cipher
            self.ReceiveCipher(video_name, int(frame_num), clientID)
            print("Receiving Done")
            # define extraction class
            sv = FHEserver()
            # blind extract
            sv.server(video_name, clientID)
            # define reply class
            ss = ServerSend()
            # send cipher back
            ss.run(video_name, clientID)
            # remark a log
            log = open('../doc/operation.log', 'a', encoding='utf-8')
            # exact time, operation, video name are in log
            log_msg = time.strftime('%Y/%m/%d-%H:%M:%S', time.localtime(time.time())) + '--Upload--' + clientID + '--' + video_name + '--' + frame_num + '--end\n'
            log.write(log_msg)
            log.close()
        elif get_head.split(':')[1] == 'Dow':
            print(get_head)
            # request for downloading
            video_name = get_head.split(':')[2]
            clientID = get_head.split(':')[0]
            # remark a log
            log = open('../doc/operation.log', 'a', encoding='utf-8')
            # exact time, operation, video name are in log
            log_msg = time.strftime('%Y/%m/%d-%H:%M:%S', time.localtime(time.time())) + '--Download--' + clientID + '--' + video_name + '--begin\n'
            log.write(log_msg)
            log.close()
            if os.path.exists("../ServerPool/S2C/" + clientID):
                # client fold existed
                if os.path.exists("../ServerPool/S2C/" + clientID + '/' + video_name):
                    # selected fold existed
                    # define reply class
                    ss = ServerSend()
                    ss.SendMsg('PrepareDownload')
                    print("PrepareDownload")
                    # send cipher back
                    ss.run(video_name, clientID)
                    # remark a log
                    log = open('../doc/operation.log', 'a', encoding='utf-8')
                    # exact time, operation, video name are in log
                    log_msg = time.strftime('%Y/%m/%d-%H:%M:%S', time.localtime(time.time())) + '--Download--' + clientID + '--' + video_name + '--end\n'
                    log.write(log_msg)
                    log.close()
                else:
                    # selected fold not existed
                    # define reply class
                    ss = ServerSend()
                    ss.SendMsg("VideoNotFound")
                    # remark a log
                    log = open('../doc/operation.log', 'a', encoding='utf-8')
                    # exact time, operation, video name are in log
                    log_msg = time.strftime('%Y/%m/%d-%H:%M:%S', time.localtime(time.time())) + '--Download--' + clientID + '--' + video_name + '--VideoNotFound\n'
                    log.write(log_msg)
                    log.close()
            else:
                # seleted fold not existed
                # define reply class
                ss = ServerSend()
                ss.SendMsg("YetUpload")
                # remark a log
                log = open('../doc/operation.log', 'a', encoding='utf-8')
                # exact time, operation, video name are in log
                log_msg = time.strftime('%Y/%m/%d-%H:%M:%S', time.localtime(time.time())) + '--Download--' + clientID + '--' + video_name + '--YetUpload\n'
                log.write(log_msg)
                log.close()
        elif get_head.split(':')[1] == 'Del':
            print(get_head)
            # request for deleting
            # receive client ID
            clientID = get_head.split(':')[0]
            # receive video name
            video_name = get_head.split(':')[2]
            # remark a log
            log = open('../doc/operation.log', 'a', encoding='utf-8')
            # exact time, operation, video name are in log
            log_msg = time.strftime('%Y/%m/%d-%H:%M:%S', time.localtime(time.time())) + '--Delete--' + clientID + '--' + video_name + '--begin\n'
            log.write(log_msg)
            log.close()
            # define delete class
            de = Delete()
            # get delete status
            isDelete = de.delete(video_name, clientID)
            print(isDelete)
            # define reply class
            ss = ServerSend()
            # send delete status back
            ss.SendMsg(isDelete)
            # remark a log
            log = open('../doc/operation.log', 'a', encoding='utf-8')
            # exact time, operation, video name are in log
            log_msg = time.strftime('%Y/%m/%d-%H:%M:%S', time.localtime(time.time())) + '--Delete--' + clientID + '--' + video_name + '--' + isDelete + '\n'
            log.write(log_msg)
            log.close()
        else:
            None

    def read_head(self) -> str:
        # create ssl context
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        # load private key
        context.load_cert_chain(certfile="../../ssl/server.crt", keyfile="../../ssl/server.key")
        # define server tcp socket
        tcp_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # make port reuse
        tcp_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # listen to all and certain port
        tcp_server.bind((self.host, self.port))
        print("Bind Success")
        # define max connection number 1
        tcp_server.listen(1)
        print("Waiting For Connection")
        rec = ''
        while True:
            # acquire client ip address and port
            tcp_client, addr = tcp_server.accept()
            ssl_connect_sock = context.wrap_socket(tcp_client, server_side=True)
            print("Socket Connected")
            while True:
                # receive data
                rec_data = ssl_connect_sock.recv(1024)
                rec = rec_data.decode("utf-8")
                break
            # close locale TCP connection
            tcp_client.close()
            break
        # close locale server socket
        tcp_server.close()
        if rec.split(':')[1] == 'gID':
            # client ask for ID
            print(rec + str(addr[0]) + str(addr[1]))
            return rec + str(addr[0]) + str(addr[1])
        else:
            # client ask for else
            return rec

    def ReceiveIndex(self) -> str:
        # create ssl context
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        # load private key
        context.load_cert_chain(certfile="../../ssl/server.crt", keyfile="../../ssl/server.key")
        # define server tcp socket
        tcp_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # make port reuse
        tcp_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # listen to all and certain port
        tcp_server.bind((self.host, self.port))
        print("Bind Success")
        # define max connection number 1
        tcp_server.listen(1)
        print("Waiting For Connection")
        rec = ''
        while True:
            # acquire client ip address and port
            tcp_client, addr = tcp_server.accept()
            ssl_connect_sock = context.wrap_socket(tcp_client, server_side=True)
            print("Socket Connected")
            # print("Connection Established")
            while True:
                # receive data
                rec_data = ssl_connect_sock.recv(1024)
                rec = rec_data.decode("utf-8")
                break
            # close locale TCP connection
            tcp_client.close()
            break
        # close locale server socket
        tcp_server.close()
        # return received data
        return rec

    def ReceiveCipher(self, video_name, num, clientID):
        # define work path
        path = "../ServerPool/C2S/" + video_name
        # if fold is existed
        if os.path.exists(path):
            # delete existed fold
            shutil.rmtree(path)
        # recreate fold
        os.mkdir(path)
        # define server TCP socket
        tcp_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # make port reuse
        tcp_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # listen to all and certain port
        tcp_server.bind((self.host, self.port))
        print("Bind Success")
        # define max connection number 1
        tcp_server.listen(1)
        print("Build socket")
        for i in range(0, num):
            while True:
                # acquire client ip address and port
                tcp_client, addr = tcp_server.accept()
                print("Connected")
                while True:
                    # receive frame ser
                    frame_ser_data = tcp_client.recv(1024)
                    frame_ser = frame_ser_data.decode("utf-8")
                    # send a reply
                    tcp_client.send("Copy".encode("utf-8"))
                    # define path to store cipher
                    file_path = path + "/" + frame_ser
                    # open txt to store cipher
                    new_frame = open(file_path, "wb")
                    string = ''
                    while True:
                        # receive cipher data
                        data = tcp_client.recv(1024)
                        if not data:
                            break
                        string = string + data.decode()
                    # write cipher into txt
                    new_frame.write(string.encode())
                    # close txt
                    new_frame.close()
                    # send a reply
                    break
                break
            print("Done: " + str(i))
        # close TCP connection
        tcp_server.close()

class ServerSend:
    def __init__(self):
        # define locale IP address
        self.host = "0.0.0.0"
        # define default port
        self.port = 3389

    def run(self, video_name, clientID):
        self.host = str(self.host)
        # define cipher fold
        fold_path = "../ServerPool/S2C/" + clientID + '/' + video_name
        # get number of frames in the fold
        frame_num = len(os.listdir(fold_path))
        # send video name and frame number
        do_next1 = self.SendMsg(video_name)
        print("Sent: " + video_name)
        do_next2 = self.SendMsg(str(frame_num))
        print("Sent: " + str(frame_num))
        if do_next1 and do_next2:
            done = self.SendTXT(video_name, frame_num, clientID)
            if done:
                print("Sending Done")
            else:
                print("Send Error")
                exit(1)
        else:
            print("Fail To Begin Cipher Sending")
            exit(1)
        # free space
        if os.path.exists("../ServerPool/C2S/" + video_name):
            shutil.rmtree("../ServerPool/C2S/" + video_name)
        if os.path.exists("../ServerPool/FrameNumber/" + video_name + ".txt"):   
            os.remove("../ServerPool/FrameNumber/" + video_name + ".txt")

    def SendMsg(self, msg) -> bool:
        # create ssl context
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        # load private key
        context.load_cert_chain(certfile="../../ssl/server.crt", keyfile="../../ssl/server.key")
        # define server tcp socket
        tcp_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # make port reuse
        tcp_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # listen to all and certain port
        tcp_server.bind((self.host, self.port))
        print("Bind Success")
        # define max connection number 1
        tcp_server.listen(1)
        print("Waiting For Connection")
        while True:
            # acquire client ip address and port
            tcp_client, addr = tcp_server.accept()
            ssl_connect_sock = context.wrap_socket(tcp_client, server_side=True)
            print("Socket Connected")
            # print("Connection Established")
            while True:
                ssl_connect_sock.sendall(msg.encode("utf-8"))
                break
            # close locale TCP connection
            tcp_client.close()
            break
        # close locale server socket
        tcp_server.close()
        # return received data
        return True

    def SendTXT(self, video_name, num, clientID) -> bool:
        # fine default work place path
        file_path = "../ServerPool/S2C/" + clientID + '/' + video_name + "/"
        # define server TCP socket
        tcp_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # make port reuse
        tcp_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # listen to all and certain port
        tcp_server.bind((self.host, self.port))
        print("Bind Success")
        # define max connection number 1
        tcp_server.listen(1)
        print("Build socket")
        for i in range(1, num + 1):
            # acquire client ip address and port
            tcp_client, addr = tcp_server.accept()
            print("Connected")
            while True:
                # open txt and read text
                data = ''
                with open(file_path + str(i) + ".txt", 'rb') as f:
                   data = f.read()
                f.close()
                # send frame cipher name to client
                tcp_client.send((str(i) + ".txt").encode("utf-8"))
                # client receive name correctly
                OK_next_data = tcp_client.recv(1024)
                OK_next = OK_next_data.decode("utf-8")
                if OK_next == "Copy":
                    while True:
                        # send cipher to client
                        tcp_client.send(data)
                        break
                break
            # terminate TCP socket
            tcp_client.close()
            print("Done: " + str(i))
        # close TCP connection
        tcp_server.close()
        return True


if __name__ == '__main__':
    while True:
        sr = ServerReceive()
        sr.run()
