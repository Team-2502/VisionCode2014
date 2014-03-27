#ifndef TYPES_H
#define TYPES_H

#define PI 3.1415926535
#define STATUS_INITIALIZING 1
#define STATUS_RUNNING 2
#define STATUS_STOPPED 4
#define STATUS_CAPTURING 8
#define STATUS_CALIBRATING 16

class RaspiVid;

typedef struct {
	int x;
	int y;
} CalibrateTarget;

typedef struct {
	int brightness;
	int threshMin;
	int threshMax;
	float score;
} Calibration;

typedef struct {
	float x;
	float y;
	int width;
	int height;
	float pointsX[4];
	float pointsY[4];
	float ratio;
	float angle;
	float area;
	float dist; // Distance to Target
	float distError; // Estimated error in the distance calculation
	bool hotTarget;
} Target;

typedef struct {
	int brightness;
	int threshMin;
	int threshMax;
	int width;
	int height;
} SaveData;

typedef struct {
	volatile int status;
	RaspiVid * vision;
	SaveData * saveData;
	const char * saveDataFile;
	int width;
	int height;
	bool competitionMode;
	float framerate;
} USERDATA;

#endif
