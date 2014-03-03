#ifndef CALIBRATION_H
#define CALIBRATION_H
#include "../misc.h"
#include <cmath>

double getCalibrationScore(int brightness) {
	
}

bool sortCalibrations(Calibration a, Calibration b) {
	return a.score < b.score;
}

void * calibrationThread(void * arg) {
	USERDATA * userdata = (USERDATA *) arg;
	RaspiVid * vision = userdata->vision;
	userdata->calStatus = new CalibrationStatus;
	CalibrationStatus * calStatus = userdata->calStatus;
	cout << "Spawned Calibration Thread.\n";
	while (userdata->status & STATUS_INITIALIZING) {
		usleep(10);
	}
	cout << "Main Thread Initialized.\n";
	// Calibration variables
	const int START_BRIGHTNESS = 50;
	const int THRESH_INC = 3;
	int calibrationLoop = 0;
	int brightness = 50;
	calStatus->brightness = brightness;
	vector <Calibration> calibrations;
	while (userdata->status & STATUS_RUNNING) {
		if (!(userdata->status & STATUS_CALIBRATING)) {
			calibrationLoop = 0;
			usleep(50000);
			continue;
		}
		if (calibrationLoop == 0) {
			cout << "Calibrating...\n";
			calibrations.clear();
			calStatus->brightness = brightness;
			calStatus->percentComplete = 0;
		} else if (50 - calibrationLoop*2 < -2) {
			sort(calibrations.begin(), calibrations.end(), sortCalibrations);
			cout << "Top 3 of " << calibrations.size() << " calibrations:\n";
			for (int i = 0; i < calibrations.size() && i < 3; i++) {
				cout << "  Brightness: " << calibrations[i].brightness << "\tThreshold: " << calibrations[i].threshMin << ", ";
				cout << calibrations[i].threshMax << "\n";
			}
			if (calibrations.size() >= 1) {
				calStatus->brightness = calibrations[0].brightness;
				vision->setBrightness(calibrations[0].brightness);
				userdata->saveData->brightness = calibrations[0].brightness;
				userdata->saveData->threshMin = calibrations[0].threshMin;
				userdata->saveData->threshMax = calibrations[0].threshMax;
				saveData(userdata->saveDataFile, *userdata->saveData);
			}
			userdata->status = STATUS_RUNNING | STATUS_CAPTURING;
			calStatus->percentComplete = 0;
		} else {
			brightness = 50 - calibrationLoop*2;
			if (brightness == 0) brightness = 1;
			else if (brightness < 0) brightness = 0;
			cout << "Testing Brightness: " << brightness << "\n";
			calStatus->brightness = brightness;
			calStatus->percentComplete = (1 - (50 - calibrationLoop*2 + 4)/52.0)*100.0;
			vision->setBrightness(brightness);
			vision->grabFrame();
			vision->grabFrame();
			VideoBuffer buffer = vision->grabFrame();
			Mat image = Mat(userdata->height, userdata->width, CV_8UC1, buffer.data(), true);
			Mat thresh;
			Calibration cal;
			Calibration bestCal; bestCal.score = -1;
			for (int min = 1; min < 255 && (userdata->status & STATUS_RUNNING); min+=THRESH_INC) {
				threshold(min, 255, image, thresh);
				vector <vector<Point> > contours;
				{ // To reduce the scope of threshClone
					Mat threshClone = thresh.clone();
					findContours(threshClone, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
				}
				vector <vector <Point> > finalContours = process(contours, userdata->width, userdata->height, 0, userdata->width*userdata->height);
				vector <Target> vTargets = getTargets(finalContours, userdata->width, userdata->height);
				int targetsFound = 0;
				float score = 0;
				for (unsigned int i = 0; i < userdata->calTargets->size(); i++) {
					CalibrateTarget calTarget = (*userdata->calTargets)[i];
					for (unsigned int t = 0; t < vTargets.size(); t++) {
						if (sqrt(pow(vTargets[t].x-calTarget.x, 2) + pow(vTargets[t].y-calTarget.y, 2)) <= userdata->width/32) {
							targetsFound++;
							score += pow(contourArea(finalContours[i])/double(userdata->width*userdata->height), 5/8.0) * 10;
						}
					}
				}
				score += vTargets.size() - targetsFound;
				if (targetsFound == userdata->calTargets->size()) {
					cal.brightness = brightness;
					cal.threshMin = min;
					cal.threshMax = 255;
					cal.score = score;
					if (bestCal.score == -1 || cal.score < bestCal.score) {
						bestCal = cal;
					}
				} else if (bestCal.score != -1 || (bestCal.score != -1 && cal.score > bestCal.score)) {
					break;
				}
			}
			calibrations.push_back(bestCal);
		}
		calibrationLoop++;
	}
	return NULL;
}

#endif // CALIBRATION_H
