#ifndef PROCESS_H
#define PROCESS_H

#include "../misc.h"

bool passesFilter(Target t) {
	if (t.ratio <= 5 || t.ratio >= 8 || t.width < 20 || t.height < 5)
		return false;
	return true;
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
	rect.points(t.points);
	t.area = contourArea(points);
	t.ratio = t.width / (float)t.height;
	t.dist = 0;
	t.distError = 0; // Don't worry, I'm always right
	return t;
}

vector <Target> processAndGetTargets(vector <vector <Point> > points) {
	vector <Target> targets;
	for (unsigned int i = 0; i < points.size(); i++) {
		if (points[i].size() < 4)
			continue;
		vector <Point> processed = process(points[i]);
		if (processed.size() == 0)
			continue;
		Target t = getTarget(processed);
		if (passesFilter(t))
			targets.push_back(t);
	}
	return targets;
}

#endif // PROCESS_H
