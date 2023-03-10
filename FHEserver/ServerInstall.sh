echo "Welcome to FHEserver"
echo "Begin Installation"
echo "======================"
sudo apt-get install cmake
sudo apt-get install build-essential libgtk2.0-dev libavcodec-dev libavformat-dev libjpeg-dev libswscale-dev libtiff5-dev
sudo apt-get install libgtk2.0-dev
sudo apt-get install pkg-config
sudo add-apt-repository ppa:openjdk-r/ppa
sudo apt-get update
sudo apt-get install libopenni-dev libopenni2-dev
unzip opencv-3.4.16.zip
cd opencv-3.4.16
mkdir build
cd build
cmake -D WITH_TBB=ON -D WITH_EIGEN=ON -D OPENCV_GENERATE_PKGCONFIG=ON  -D BUILD_DOCS=ON -D BUILD_TESTS=OFF -D BUILD_PERF_TESTS=OFF -D BUILD_EXAMPLES=OFF  -D WITH_OPENCL=OFF -D WITH_CUDA=OFF -D BUILD_opencv_gpu=OFF -D BUILD_opencv_gpuarithm=OFF -D BUILD_opencv_gpubgsegm=O -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local ..
make -j8
sudo make install
echo "/usr/local/lib" >> /etc/ld.so.conf.d/opencv.conf
sudo ldconfig 
echo "PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig
	export PKG_CONFIG_PATH" >> /etc/bash.bashrc
sudo apt install mlocate
source /etc/bash.bashrc
sudo updatedb
pkg-config --modversion opencv
sudo apt install python3-pip
pip3 install django
g++ ../../FHEserver/server/Server.cpp -lpthread -o ../../FHEserver/server/Server `pkg-config opencv --cflags --libs`
g++ ../../FHEserver/server/Write.cpp -o ../../FHEserver/server/Write
echo "====================="
echo "Installation done"
echo "====================="
echo "====================="
