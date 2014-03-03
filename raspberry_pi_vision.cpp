#include "communication/tcp_communications.h"
#include "data_storage.cpp"
#include "misc.h"
#include "vision_processing/process.h"

RaspiVid * v = NULL;

int initialize() {
	// Global Initialization
	data = new USERDATA;
	data->status = STATUS_INITIALIZING;
	
	// Load Vision Data
	data->width = globalStorage.getSaveData()->width;
	data->height = globalStorage.getSaveData()->height;
	return 0;
}

int initializeVision(int width, int height) {
	if (v != NULL) {
		v->destroy();
		delete v;
	}
	// Initializes Camera
	v = new RaspiVid("/dev/video0", width, height);
	data->vision = v;
	cout << "Initializing...\n";
	if (!v->initialize(RaspiVid::METHOD_MMAP)) {
		cout << "Could not initialize, attempting to create a new /dev/video\n";
		system("sudo uv4l --sched-rr --driver raspicam --auto-video_nr --width 640 --height 480 --encoding yuv420 --nopreview --imgfx blur --awb off --framerate 30");
		if (!v->initialize(RaspiVid::METHOD_MMAP)) {
			cout << "Yeah, didn't work. Failed!\n";
			return -1;
		} else {
			cout << "AND I RECOVER!\n";
		}
		return 0;
	}
	v->setBrightness(globalStorage.getSaveData()->brightness);
	v->startCapturing();
}

int main(int argc, char *argv[]) {
	globalStorage.setCompetitionMode(false);
	globalStorage.setGameRecording(false);
	globalStorage.openSaveData("server_data.bin");
	globalStorage.writeSaveData();
	globalStorage.openVideoFile("video_output.bin");
	globalStorage.openMatchFile("match_output.bin");
	
	if (initialize() != 0)
		return -1;
	
	double averageFrame = -1;
	long startFrame = getmsofday();
	bool running = true;
	
	cout << "Capturing...\n";
	
	pthread_t pCommunicationThread;
	pthread_create(&pCommunicationThread, NULL, &tcp_communications, NULL);
	globalStorage.getSaveData()->width = 640;
	globalStorage.getSaveData()->height = 480;
	do {
		int width = globalStorage.getSaveData()->width;
		int height = globalStorage.getSaveData()->height;
		if (initializeVision(width, height) != 0) {
			cout << "Failed to initialize. I give up.\n";
			running = false;
			continue;
		}
		globalStorage.setVisionRestart(false);
		for (unsigned int frame = 0; !globalStorage.isVisionRestarting(); frame++) {
			VideoBuffer buffer = v->grabFrame();
			if (buffer.length() == 0 || buffer.data() == NULL) {
				cout << "Invalid buffer length or invalid buffer data pointer\n";
				continue;
			}
			
			Mat image(height, width, CV_8UC1, buffer.data(), false);
			Mat thresh;
			inRange(image, Scalar(globalStorage.getSaveData()->threshMin), Scalar(globalStorage.getSaveData()->threshMax), thresh);
			vector <vector<Point> > contours;
			findContours(thresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
			vector <Target> targets = processAndGetTargets(contours);
			globalStorage.setTargets(targets);
			
			long endFrame = getmsofday();
			long diffFrame = endFrame - startFrame;
			startFrame = endFrame;
			if (averageFrame == -1) averageFrame = diffFrame; else averageFrame = averageFrame*.9 + diffFrame*.1;
			data->framerate = 1000.0 / averageFrame;
			if (frame % 10 == 0) {
				cout << setprecision(2) << "\rFrame: #" << frame << "\t" << averageFrame << "ms    " << (1000.0 / averageFrame) << " FPS          ";
				cout.flush();
				if (globalStorage.isGameRecording()) {
					globalStorage.writeToVideoFile(buffer.data(), buffer.length());
				}
			}
			if (!globalStorage.isCompetitionMode()) {
				if (frame % 15 == 0) {
					waitKey(1);
					Mat img = image.clone();
					for (unsigned int i = 0; i < targets.size(); i++) {
						for (unsigned int j = 0; j < 4; j++)
							line(img, targets[i].points[j], targets[i].points[(j+1)%4], Scalar(255), 1, 8);
					}
					imshow("Frame", img);
				}
			}
		}
	} while (running);
	delete v;
	globalStorage.closeVideoFile();
	globalStorage.closeMatchFile();
	return 0;
}

