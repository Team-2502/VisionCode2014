#include "communication/tcp_communications.h"
#include "communication/wire_communications.h"
#include "data_storage.h"
#include "str_util.h"
#include "time_util.h"
#include "vision_processing/process.h"

// RaspiVid
#include <raspicam/raspivid.h>

// I/O
#include <iostream>
#include <iomanip>
#include <sys/stat.h>
using namespace cv;

string FILEPATH = "/home/josh/devel/RPi2014/match_data/";
RaspiVid * v = NULL;

int initialize() {
	// Global Initialization
	DataStorage::Get().getUserdata()->status = STATUS_INITIALIZING;
	
	// Load Vision Data
	DataStorage::Get().getUserdata()->width = DataStorage::Get().getSaveData()->width;
	DataStorage::Get().getUserdata()->height = DataStorage::Get().getSaveData()->height;
	
	// Create /dev/video0
	struct stat buffer;
	if (stat("/dev/video0", &buffer) != 0)
		system("sudo uv4l --sched-rr --driver raspicam --device-name=video0 --width 640 --height 480 --encoding yuv420 --nopreview --imgfx blur --awb off --framerate 30");
	if (configureWire())
		return -1;
	return 0;
}

int initializeVision(int width, int height) {
	if (v != NULL) {
		v->destroy();
		delete v;
	}
	// Initializes Camera
	v = new RaspiVid("/dev/video0", width, height);
	DataStorage::Get().getUserdata()->vision = v;
	cout << "Initializing...\n";
	if (!v->initialize(RaspiVid::METHOD_MMAP))
		return -1;
	v->setBrightness(4);
	if (DataStorage::Get().getSaveData()->threshMin == 0 || DataStorage::Get().getSaveData()->threshMin > 50)
		DataStorage::Get().getSaveData()->threshMin = 1;
	if (DataStorage::Get().getSaveData()->threshMax < 200)
		DataStorage::Get().getSaveData()->threshMax = 255;
	v->startCapturing();
}

void iterativeVisionLoop(int width, int height, long frame, RaspiVid * v) {
	VideoBuffer buffer = v->grabFrame();
	if (buffer.length() == 0 || buffer.data() == NULL) {
		cout << "Invalid buffer length or invalid buffer data pointer\n";
		return;
	}
	
	if (isProcessingStarted()) {
		v->setBrightness(4);
		vector <Target> targets = processAndGetTargets(width, height, buffer.data());
		DataStorage::Get().setTargets(targets);
		bool hot = false;
		for (int i = 0; i < targets.size() && !hot; i++) {
			if (targets[i].hotTarget)
				hot = true;
		}
		setHotWire(hot);
	} else {
		Mat image(height, width, CV_8UC1, buffer.data(), false);
		DataStorage::Get().setBrightnessImage(image);
		v->setBrightness(50);
	}
	if (!DataStorage::Get().isCompetitionMode()) {
		if (frame % 15 == 0) {
			showClientImage();
		}
	} else {
		if (DataStorage::Get().isGameRecording()) {
			if (!DataStorage::Get().isVideoFileOpened()) {
				string videoOutputPath = FILEPATH.c_str();
				videoOutputPath.append("video_output.bin");
				system(string("echo mv -f ").append(videoOutputPath).append(" ").append(FILEPATH.c_str()).append("video_output_bak.bin").c_str());
				system(string("mv -f ").append(videoOutputPath).append(" ").append(FILEPATH.c_str()).append("video_output_bak.bin").c_str());
				DataStorage::Get().openVideoFile(videoOutputPath.c_str());
				system(string("chmod 777 ").append(videoOutputPath).c_str());
			}
			if (DataStorage::Get().getVideoFileSize() < 3.5 * 1024 * 1024 * 1024) {
				unsigned char time[8];
				*((long*)&time[0]) = getmsofday();
				DataStorage::Get().writeToVideoFile(time, 8);
				DataStorage::Get().writeToVideoFile(buffer.data(), buffer.length());
			}
		}
	}
}

int main(int argc, char *argv[]) {
	DataStorage::Get().setCompetitionMode(true);
	DataStorage::Get().setGameRecording(true);
	string saveDataPath = FILEPATH.c_str();
	string matchOutputPath = FILEPATH.c_str();
	saveDataPath.append("server_data.bin");
	matchOutputPath.append("match_output.bin");
	DataStorage::Get().openSaveData(saveDataPath.c_str());
	DataStorage::Get().openMatchFile(matchOutputPath.c_str());
	system(string("chmod 777 ").append(saveDataPath).c_str());
	system(string("chmod 777 ").append(matchOutputPath).c_str());
	DataStorage::Get().writeSaveData();
	
	if (initialize() != 0)
		return -1;
	
	double averageFrame = -1;
	long startFrame = getmsofday();
	bool running = true;
	
	pthread_t pCommunicationThread;
	pthread_create(&pCommunicationThread, NULL, &tcp_communications, NULL);
	DataStorage::Get().getSaveData()->width = 640;
	DataStorage::Get().getSaveData()->height = 480;
	do {
		int width = 640;//DataStorage::Get().getSaveData()->width;
		int height = 480;//DataStorage::Get().getSaveData()->height;
		if (initializeVision(width, height) != 0) {
			cout << "Failed to initialize. I give up.\n";
			running = false;
			continue;
		}
		cout << "Capturing at " << width << "x" << height << "\n";
		DataStorage::Get().setVisionRestart(false);
		bool lastStartProcessingValue = false;
		for (unsigned int frame = 0; !DataStorage::Get().isVisionRestarting(); frame++) {
			iterativeVisionLoop(width, height, frame, v);
			long endFrame = getmsofday();
			long diffFrame = endFrame - startFrame;
			startFrame = endFrame;
			if (averageFrame == -1) averageFrame = diffFrame; else averageFrame = averageFrame*.9 + diffFrame*.1;
			DataStorage::Get().getUserdata()->framerate = 1000.0 / averageFrame;
			if (frame % 10 == 0)
				cout << setprecision(2) << "\rFrame: #" << frame << "\t" << averageFrame << "ms    " << (1000.0 / averageFrame) << " FPS          ";
		}
	} while (running);
	cout << "Closing...\n";
	delete v;
	DataStorage::Get().closeVideoFile();
	DataStorage::Get().closeMatchFile();
	return 0;
}

