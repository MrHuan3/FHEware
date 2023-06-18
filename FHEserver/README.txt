Welcome to FHEserver
If you use FHEserver for the first time
Please run FullyInstall.sh once to install FHEserver as root
After installation completed
To open FHE web
Run "python3 manage.py runserver 0.0.0.0:8000" in ./FHE
To start blind extraction
Run "python3 FHEserver.py" in ./FHEserver/server

Besides
Change FHEware client application
    ./web/upload.html line 89
    ./web/download.html line 89
    ./bin/ClientSocket.py line 17 and line 117
With your server IP and port

If you are using local server
Change
    0.0.0.0 in line 4
    ./FHEserver/server/FHEserver.py line 21
    ./FHEserver/server/FHEserver.py line 304
    to your private server IP

Download opencv-3.4.16.zip and put it into /FHEware and /FHEserver
URL is https://github.com/opencv/opencv/tree/3.4.16

After change FHEware client application
Tar it into FHEware.tar.xz and put it into ./FHE/statics/download

Wish you a pleasure try