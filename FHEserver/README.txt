Welcome to FHEserver
If you use FHEserver for the first time
Please run FullyInstall.sh once to install FHEware as root
After installation completed
To open FHE web
Run "python3 manage.py runserver 0.0.0.0:8000" in ./FHE
To start blind extraction
Run "python3 FHEserver.py" in ./FHEserver/server
Besides
Change client application
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
the client application is in ./FHE/statics/download
After change zip the application to FHEware.tar.xz again
Wish you a pleasure try