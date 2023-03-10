# -*- encoding: utf-8 -*-
# File Name: MainPanel
# Author: 瓛
# @Time: 2023/1/11 16:56 1月

import os
import tkinter as tk
from tkinter.filedialog import askopenfilename
import tkinter.messagebox
import time
import os.path
import shutil

from ware import Ware
from ID import ID
from ClientSocket import ClientSend, ClientReceive

class MainPanel:
    def __init__(self):
        self.locked = False
        self.loop = True
        self.window = None
        self.video_name = ''
        self.main()

    def quit(self):
        # destroy window and quit
        self.window.destroy()
        self.window.quit()

    def main(self):
        # create main contral panel
        self.window = tk.Tk()
        self.window.title('Main Panel')
        self.window.geometry('300x185')
        # display version info
        tk.Label(self.window, text='FHEware Vision 23.01.12').place(x=15, y=10)

        # describe selected download video name
        tk.Label(self.window, text='Video name').place(x=15, y=80)
        # define video name input frame
        self.video_name = tk.StringVar()
        enter_vn = tk.Entry(self.window, textvariable=self.video_name)
        enter_vn.place(x=15, y=100, width=270, height=30)

        # define tutorial button
        bt_tutorial = tk.Button(self.window, text='  Upload ', command=self.upload)
        bt_tutorial.place(x=10, y=40)
        # define upload video button
        bt_upload = tk.Button(self.window, text='Download', command=self.download)
        bt_upload.place(x=100, y=40)
        # define download button
        bt_download = tk.Button(self.window, text='  Delete  ', command=self.delete)
        bt_download.place(x=200, y=40)
        # define tutorial button
        bt_tutorial = tk.Button(self.window, text=' Tutorial  ', command=self.tutorial)
        bt_tutorial.place(x=10, y=140)
        # define quit button
        bt_quit = tk.Button(self.window, text='    Quit    ', command=self.quit)
        bt_quit.place(x=200, y=140)
        # avoid window quit
        self.window.mainloop()

    def tutorial(self):
        # display user tutorial
        doc = open('../doc/tutorial.txt', 'r', encoding='utf-8')
        tk.messagebox.showinfo(title='TUTORIAL', message=doc.read())

    def upload(self):
        if not self.locked:
            # lock to avoid multi videos
            self.locked = True
            # choose video to upload
            video_path = askopenfilename(title='Choose video', filetypes=[('mp4', '*.mp4')])
            self.trans_video(video_path)
        else:
            # wait for current blind extraction done
            tk.messagebox.showerror(message='Please wait for current blind extraction done')

    def trans_video(self, video_path):
        # get video
        if len(video_path) != 0:
            video_name = video_path.split("/")[-1]
            # copy video to selected fold
            cmd = 'cp ' + video_path + ' ../ClientPool/Videos/' + video_name
            os.system(cmd)
            # encrypt video
            self.encrypt(video_name)

    def encrypt(self, video_name):
        # remark a log
        log = open('../doc/operation.log', 'a', encoding='utf-8')
        # exact time, operation, video name are in log
        log_msg = time.strftime('%Y/%m/%d-%H:%M:%S', time.localtime(time.time())) + '--Upload--' + video_name + '--begin\n'
        log.write(log_msg)
        log.close()
        # define ID class
        id = ID()
        # get local ID
        clientID = id.readID()
        # start encryption
        wa = Ware()
        wa.BeforeClient(video_name)
        self.ClientSend(video_name, clientID)

    def ClientSend(self, video_name, clientID):
        # create ClientSend object
        cs = ClientSend()
        # send clientID, video name in upload mode
        cs.run(video_name, clientID)
        print("Wating for cipher")
        time.sleep((cs.frame_num / 64 + 3))
        # listen and receive reduced frame cipher
        cr = ClientReceive()
        cr.run()
        self.decrypt(video_name, "Upload")

    def decrypt(self, video_name, status):
        # decrypt frame cipher and rebuild reduced video
        wa = Ware()
        wa.AfterClient(video_name)
        # remark a log
        log = open('../doc/operation.log', 'a', encoding='utf-8')
        # exact time, operation, video name are in log
        log_msg = time.strftime('%Y/%m/%d-%H:%M:%S', time.localtime(time.time())) + '--' + status + '--' + video_name + '--end\n'
        log.write(log_msg)
        log.close()
        if status == 'Upload':
            # display video in upload frame
            self.upload_done(video_name)
        else:
            # display video in download frame
            self.download_done(video_name)

    def upload_done(self, video_name):
        # copy key document
        os.system("cp ../ClientPool/ResizedVideos/" + video_name + " ../web/videos/resized_video.mp4")
        os.system("cp ../ClientPool/ReducedVideos/" + video_name + " ../web/videos/extracted_video.mp4")
        # release space
        ori_path = "../ClientPool/Videos/" + video_name
        C2S_path = "../ClientPool/C2S/" + video_name
        S2C_path = "../ClientPool/S2C/" + video_name
        rec_path = "../ClientPool/Recovery/" + video_name
        os.remove(ori_path)
        shutil.rmtree(C2S_path)
        shutil.rmtree(S2C_path)
        shutil.rmtree(rec_path)
        # release lock to get a new operation
        self.locked = False
        os.system("firefox ../web/upload.html")

    def download_done(self, video_name):
        os.system("cp ../ClientPool/ReducedVideos/" + video_name + " ../web/videos/download_video.mp4")
        # release space
        rec_path = "../ClientPool/Recovery/" + video_name
        S2C_path = "../ClientPool/S2C/" + video_name
        shutil.rmtree(rec_path)
        shutil.rmtree(S2C_path)
        # release lock to get a new operation
        self.locked = False
        # display download video
        os.system("firefox ../web/download.html")

    def download(self):
        if not self.locked:
            self.locked = True
            # remark a log
            log = open('../doc/operation.log', 'a', encoding='utf-8')
            # exact time, operation, video name are in log
            log_msg = time.strftime('%Y/%m/%d-%H:%M:%S', time.localtime(time.time())) + '--Download--' + self.video_name.get() + '--begin\n'
            log.write(log_msg)
            log.close()
            # define ID class
            id = ID()
            # get local ID
            clientID = id.readID()
            # define send socket class
            cs = ClientSend()
            # request for download video
            msg = clientID + ':' + 'Dow:' + self.video_name.get()
            cs.SendMsg(msg)
            print("Download: " + self.video_name.get())
            time.sleep(0.05)
            down = ClientReceive()
            result = down.ReceiveIndex()
            print(result)
            if result == 'YetUpload':
                tk.messagebox.showerror(title=result, message='This account has not upload any video yet')
            elif result == 'VideoNotFound':
                msg = "No video named " + self.video_name.get() + " found"
                tk.messagebox.showerror(title=result, message=msg)
            elif result == 'PrepareDownload':
                time.sleep(0.05)
                # define receive socket class
                cr = ClientReceive()
                # judge whether get download video
                isOK = cr.run()
                if isOK:
                    # get the correct video then decrypt it
                    self.decrypt(self.video_name.get(), "Download")
                else:
                    # fail to get the correct then do nothing
                    None
            else:
                tk.messagebox.showerror(title='Error', message="Undefined Error")
            self.locked = False
        else:
            # wait for current download done
            tk.messagebox.showerror(message='Please wait for current download done')

    def delete(self):
        if not self.locked:
            self.locked = True
            # define ID class
            id = ID()
            # get local ID
            clientID = id.readID()
            print("Delete request")
            # define send socket class
            cs = ClientSend()
            # request for delete cloud video
            msg = clientID + ':Del:' + self.video_name.get()
            cs.SendMsg(msg)
            time.sleep(0.05)
            cr = ClientReceive()
            result = cr.ReceiveIndex()
            self.locked = False
            if result == 'Delete':
                # delete selected video from cloud
                msg = self.video_name.get() + " has been removed from cloud"
                tk.messagebox.showinfo(title='Delete Success', message=msg)
            elif result == 'VideoNotFound':
                # there is no selected video found on the cloud
                msg = "No video named " + self.video_name.get() + " found and no video removed"
                tk.messagebox.showerror(title=result, message=msg)
            else:
                tk.messagebox.showerror(title='Error', message="Undefined Error")
        else:
            # wait for current operation done
            tk.messagebox.showerror(message='Please wait for current operation done')
            