# -*- encoding: utf-8 -*-
# File Name: ClientSocket
# Author: 瓛
# @Time: 2022/1/11 16:52 1月

import os
import time
import os.path
import socket
import shutil
import ssl


class ClientSend:
    def __init__(self):
        # define server IP address
        self.host = "10.211.55.21"
        # define default port
        self.port = 3333
        # define frame number
        self.frame_num = 0

    def run(self, video_name, clientID):
        # define cipher fold
        fold_path = "../ClientPool/C2S/" + video_name
        # get number of frames in the fold
        frame_num = len(os.listdir(fold_path))
        self.frame_num = frame_num
        # send video name and frame number
        msg = clientID + ':Upl:' + video_name
        do_next1 = self.SendMsg(msg)
        print("Sent: " + video_name)
        time.sleep(0.5)
        do_next2 = self.SendMsg(str(frame_num))
        print("Sent: " + str(frame_num))
        time.sleep(0.5)
        if do_next1 and do_next2:
            # send frame cipher to server
            done = self.SendTXT(video_name, frame_num, clientID)
            if done:
                print("Sending Done")
                return
            else:
                print("Send Error")
                exit(1)
        else:
            print("Fail To Begin Cipher Sending")
            exit(1)

    def SendMsg(self, msg) -> bool:
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        context.verify_mode = ssl.CERT_REQUIRED
        context.load_verify_locations("../doc/ca.crt")
        # establish TCP connection
        tcp_send = socket.socket()
        # make port reuse
        tcp_send.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        ssl_sock = context.wrap_socket(tcp_send, server_hostname=self.host)
        try:
            # connect to the server
            ssl_sock.connect((self.host, self.port))
            print("Server Connected")
        except:
            print("Fail To Connect")
            tcp_send.close()
            return False
        while True:
            # send msg to server
            ssl_sock.sendall(msg.encode("utf-8"))
            tcp_send.close()
            return True

    def SendTXT(self, video_name, num, clientID) -> bool:
        # fine default work place path
        file_path = "../ClientPool/C2S/" + video_name + "/"
        # send frame cipher in loop
        for i in range(0, num):
            tcp_send = socket.socket()
            # make port reuse
            tcp_send.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            try:
                sig = 1
                while sig:
                    time.sleep(0.005)
                    # establish TCP connection with server
                    sig = tcp_send.connect((self.host, self.port))
                    print("Server Connected")
            except:
                print("Fail To Connect")
                tcp_send.close()
                return False
            while True:
                # open txt and read text
                data = ''
                with open(file_path + str(i) + ".txt", 'rb') as f:
                    data = f.read()
                f.close()
                # send frame cipher name to server
                tcp_send.send((str(i) + ".txt").encode("utf-8"))
                # server receive name correctly
                OK_next_data = tcp_send.recv(1024)
                OK_next = OK_next_data.decode("utf-8")
                if OK_next == "Copy":
                    while True:
                        # send cipher txt to server
                        tcp_send.send(data)
                        break
                break
            # terminate TCP socket
            tcp_send.close()
            print("Done: " + str(i))
        return True

class ClientReceive:
    def __init__(self):
        # define server IP address
        self.host = "10.211.55.21"
        # define default port
        self.port = 3333

    def run(self) -> bool:
        # receive frame name
        video_name = self.ReceiveIndex()
        print("Receive: " + video_name)
        # receive frame number
        time.sleep(0.5)
        frame_num = int(self.ReceiveIndex())
        print("Receive: " + str(frame_num))
        # receive reduced cipher
        time.sleep(0.5)
        self.ReceiveCipher(video_name, frame_num)
        print("Receiving Done")
        return True

    def ReceiveIndex(self) -> str:
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        context.verify_mode = ssl.CERT_REQUIRED
        context.load_verify_locations("../doc/ca.crt")
        # establish TCP connection
        tcp_send = socket.socket()
        # make port reuse
        tcp_send.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        ssl_sock = context.wrap_socket(tcp_send, server_hostname=self.host)
        try:
            # connect to the server
            ssl_sock.connect((self.host, self.port))
            print("Server Connected")
        except:
            print("Fail To Connect")
            tcp_send.close()
            return "Error"
        while True:
            # receive reply from server
            rec_data = ssl_sock.recv(1024)
            rec = rec_data.decode("utf-8")
            tcp_send.close()
            return rec

    def ReceiveCipher(self, video_name, num):
        # define work path
        path = "../ClientPool/S2C/" + video_name
        # if fold is existed
        if os.path.exists(path):
            # delete existed fold
            shutil.rmtree(path)
        # recreate fold
        os.mkdir(path)
        # send frame cipher in loop
        for i in range(1, num + 1):
            tcp_client = socket.socket()
            # make port reuse
            tcp_client.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            try:
                sig = 1
                while sig:
                    time.sleep(0.005)
                    # establish TCP connection with server
                    sig = tcp_client.connect((self.host, self.port))
                    print("Server Connected")
            except:
                print("Fail To Connect")
                tcp_client.close()
                return False
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
                break
            tcp_client.close()
            print("Done: " + str(i))
