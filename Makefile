

all:
	g++ Principal.cpp -I/usr/include/opencv4/ -L/home/usr/include/opencv4/opencv2/ -lopencv_core -lopencv_highgui -lopencv_video -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio -o vision.bin
	
run:
	./vision.bin
