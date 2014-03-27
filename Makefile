CFLAGS =
CV_CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv` -lwiringPi -luv4lext -lraspivid
OTHER = -L/usr/lib/uv4l/uv4lext/armv6l -L /usr/lib/arm-linux-gnueabihf/ -Wl,-rpath,'/usr/lib/uv4l/uv4lext/armv6l'
OBJS = raspberry_pi_vision.o data_storage.o process.o
DEPS = communication/tcp_communications.h communication/wire_communications.h data_storage.h str_util.h time_util.h vision_processing/process.h

raspberry_pi_vision : $(OBJS)
	g++ $(OTHER) $(LIBS) -g -pthread -o raspberry_pi_vision $(OBJS)

raspberry_pi_vision.o : raspberry_pi_vision.cpp $(DEPS)
	g++ $(CFLAGS) -g -c -o raspberry_pi_vision.o raspberry_pi_vision.cpp

data_storage.o : data_storage.cpp
	g++ $(CFLAGS) -g -c -o data_storage.o data_storage.cpp

process.o : vision_processing/process.cpp
	g++ $(CFLAGS) $(CV_CFLAGS) -g -c -o process.o vision_processing/process.cpp

clean : 
	rm -f raspberry_pi_vision $(OBJS)

