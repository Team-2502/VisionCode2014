CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`
OTHER = -L/usr/lib/uv4l/uv4lext/armv6l -L /usr/lib/arm-linux-gnueabihf/ -luv4lext -lv4l2 -lraspivid -Wl,-rpath,'/usr/lib/uv4l/uv4lext/armv6l'

raspberry_pi_vision : raspberry_pi_vision.cpp
	g++ $(OTHER) $(CFLAGS) $(LIBS) -g -std=c++0x -pthread -o raspberry_pi_vision raspberry_pi_vision.cpp

