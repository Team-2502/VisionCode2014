#include "process.h"

// OpenCV
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Math
#include <cmath>

// Vector
#include <vector>

#include <iostream>

// Types and Data Storage
#include "../types.h"
#include "../data_storage.h"

using namespace std;
using namespace cv;

bool passesFilter(Target t) {
	if (t.ratio <= 4 || t.ratio >= 9 || t.width < 20 || t.height < 5)
		return false;
	return true;
}

float targetDistance(Target a, Target b) {
	return sqrt(pow(a.x-b.x, 2) + pow(a.y-b.y, 2));
}

bool isHotTarget(Target a, Target b) {
	double angDiff = abs(a.angle - b.angle);
	int width = 640;//DataStorage::Get().getSaveData()->width;
	int height = 480;//DataStorage::Get().getSaveData()->height;
	double dist = targetDistance(a, b) / ((width > height) ? width : height);
	if (angDiff >= 60 && dist >= .15) {
		if (b.x > a.x && a.y > b.y) {
			return true;
		} else if (b.x < a.x && a.y > b.y) {
			return true;
		}
	}
	return false;
}

float angle(Point a, Point b, Point c) {
	Point ab;
    Point ac;
	
    ab.x = b.x - a.x;
    ab.y = b.y - a.y;
	
    ac.x = b.x - c.x;
    ac.y = b.y - c.y;
	
    float dotabac = (ab.x * ab.y + ac.x * ac.y);
    float lenab = sqrt(ab.x * ab.x + ab.y * ab.y);
    float lenac = sqrt(ac.x * ac.x + ac.y * ac.y);
	
    float dacos = dotabac / lenab / lenac;
	
	float ang = abs(acos(dacos));
	if (ang != ang)
		return 0;
	while (ang >= 2 * PI) {
		ang -= 2 * PI;
	}
	if (ang > PI)
		ang = PI - ang;
	return ang;
}

vector <Point> process(vector <Point> points) {
	const float ANGLE_THRESHOLD = 5;
	vector <Point> processed;
	for (int i = 0; i < points.size(); i++) {
		Point lastPoint = points[i==0 ? points.size()-1 : i-1];
		Point point = points[i];
		Point nextPoint = points[(i+1)%points.size()];
		float ang = angle(lastPoint, point, nextPoint);
		if (ang >= (90-ANGLE_THRESHOLD)*PI/180 && ang <= (90+ANGLE_THRESHOLD)*PI/180) {
			processed.push_back(points[i]);
		}
	}
	if (processed.size() < 4)
		processed.clear();
	return processed;
}

Target getTarget(vector <Point> points) {
	convexHull(Mat(points), points);
	RotatedRect rect = minAreaRect(Mat(points));
	Target t;
	t.x = rect.center.x;
	t.y = rect.center.y;
	t.width = ((rect.size.width>rect.size.height) ? rect.size.width : rect.size.height);
	t.height = ((rect.size.width>rect.size.height) ? rect.size.height : rect.size.width);
	t.angle = ((rect.size.width>rect.size.height) ? rect.angle : (rect.angle+90));
	Point2f rectPoints[4];
	rect.points(rectPoints);
	for (int i = 0; i < 4; i++) {
		t.pointsX[i] = rectPoints[i].x;
		t.pointsY[i] = rectPoints[i].y;
	}
	t.area = contourArea(points);
	t.ratio = t.width / (float)t.height;
	t.dist = 0;
	t.distError = 0; // Don't worry, I'm always right
	return t;
}

vector <Target> processAndGetTargets(int width, int height, unsigned char * data) {
	Mat image(height, width, CV_8UC1, data, false);
	Mat thresh;
	inRange(image, Scalar(DataStorage::Get().getSaveData()->threshMin), Scalar(DataStorage::Get().getSaveData()->threshMax), thresh);
	vector <vector<Point> > contours;
	findContours(thresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	vector <Target> targets;
	for (unsigned int i = 0; i < contours.size(); i++) {
		vector <Point> points = contours[i];
		if (points.size() < 4)
			continue;
		vector <Point> processed = process(points);
		if (processed.size() == 0)
			continue;
		Target t = getTarget(processed);
		if (passesFilter(t)) {
			t.hotTarget = false;
			for (int j = 0; j < int(targets.size()); j++) {
				if (isHotTarget(t, targets[j])) {
					t.hotTarget = true;
				} else if (isHotTarget(targets[j], t)) {
					targets[j].hotTarget = true;
				}
			}
			targets.push_back(t);
		}
	}
	DataStorage::Get().setBrightnessImage(image);
	DataStorage::Get().setThresholdImage(thresh);
	DataStorage::Get().setTargets(targets);
	return targets;
}

void showClientImage() {
	waitKey(1);
	vector <Target> targets = DataStorage::Get().getTargets();
	Mat img = DataStorage::Get().copyBrightnessImage();
	for (unsigned int i = 0; i < targets.size(); i++) {
		for (unsigned int j = 0; j < 4; j++) {
			Point2f cur = cvPoint(targets[i].pointsX[j], targets[i].pointsY[j]);
			Point2f next = cvPoint(targets[i].pointsX[(j+1)%4], targets[i].pointsY[(j+1)%4]);
			line(img, cur, next, Scalar(255), 1, 8);
		}
	}
	imshow("Frame", img);
}

