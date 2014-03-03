#include "data_storage.h"
#include <iostream>
#include <fstream>
using namespace std;

DataStorage::DataStorage() {
	pthread_mutex_init(&brightnessImageMutex, NULL);
	pthread_mutex_init(&redImageMutex, NULL);
	pthread_mutex_init(&blueImageMutex, NULL);
	pthread_mutex_init(&brightnessImageMutex, NULL);
	pthread_mutex_init(&thresholdImageMutex, NULL);
	pthread_mutex_init(&targetImageMutex, NULL);
	videoFile = -1;
	matchFile = -1;
	visionRestart = false;
}

DataStorage::~DataStorage() {
	if (videoFile != -1)
		close(videoFile);
	if (matchFile != -1)
		close(matchFile);
}

void DataStorage::openSaveData(const char * saveFile) {
	saveFilename = saveFile;
	if (fileExists(saveFile)) {
		cout << "Save file exists, loading from file '" << saveFile << "'\n";
		readSaveData();
	} else {
		cout << "Save file does not exist at '" << saveFile << "', creating!\n";
		saveData.brightness = 50;
		saveData.threshMin = 1;
		saveData.threshMax = 255;
		saveData.width = 640;
		saveData.height = 480;
		writeSaveData();
	}
}

bool DataStorage::writeToFile(int filePtr, unsigned char * data, unsigned int length) {
	int bytesWritten = 0;
	while (length > 0) {
		bytesWritten = write(videoFile, data, length);
		length -= bytesWritten;
		data += bytesWritten;
		if (bytesWritten == -1)
			return false;
	}
	return true;
}

void DataStorage::openVideoFile(const char * videoFilename) {
	videoFile = open(videoFilename, O_WRONLY|O_CREAT|O_TRUNC);
}

bool DataStorage::writeToVideoFile(unsigned char * data, unsigned int length) {
	if (videoFile == -1)
		return false;
	return writeToFile(videoFile, data, length);
}

void DataStorage::closeVideoFile() {
	if (videoFile != -1)
		close(videoFile);
	videoFile = -1;
}

void DataStorage::openMatchFile(const char * matchFilename) {
	matchFile = open(matchFilename, O_WRONLY|O_CREAT|O_TRUNC);
}

bool DataStorage::writeToMatchFile(unsigned char * data, unsigned int length) {
	if (videoFile == -1)
		return false;
	return writeToFile(matchFile, data, length);
}

void DataStorage::closeMatchFile() {
	if (matchFile == -1)
		close(matchFile);
	matchFile = -1;
}

bool DataStorage::fileExists(const char * filename) {
	ifstream fileTest(filename);
	bool ret = false;
	if (fileTest)
		ret = true;
	fileTest.close();
	return ret;
}

void DataStorage::writeSaveData() {
	if (saveFilename == "")
		return;
	ofstream OUTPUT;
	OUTPUT.open(saveFilename.c_str(), ios::out | ios::binary);
	OUTPUT.write((const char *)&saveData, sizeof(&saveData));
	OUTPUT.close();
}

void DataStorage::readSaveData() {
	ifstream INPUT;
	INPUT.open(saveFilename, ios::in | ios::binary);
	INPUT.read((char *)&saveData, sizeof(&saveData));
	INPUT.close();
}

SaveData * DataStorage::getSaveData() {
	return &saveData;
}

void DataStorage::setCompetitionMode(bool mode) {
	competitionMode = mode;
}

volatile bool DataStorage::isCompetitionMode() {
	return competitionMode;
}

void DataStorage::setGameRecording(bool recording) {
	gameRecording = recording;
}

volatile bool DataStorage::isGameRecording() {
	return gameRecording;
}

void DataStorage::setVisionRestart(bool restart) {
	visionRestart = restart;
}

volatile bool DataStorage::isVisionRestarting() {
	return visionRestart;
}

void DataStorage::setTargets(vector <Target> targets) {
	this->targets = targets;
}

vector <Target> DataStorage::getTargets() {
	return targets;
}

void DataStorage::setCalibrationTargets(vector <CalibrateTarget> targets) {
	this->calTargets = targets;
}

vector <CalibrateTarget> DataStorage::getCalibrationTargets() {
	return calTargets;
}

void DataStorage::setBrightnessImage(cv::Mat & newImage) {
	pthread_mutex_lock(&brightnessImageMutex);
	newImage.copyTo(brightnessImage);
	pthread_mutex_unlock(&brightnessImageMutex);
}

void DataStorage::setThresholdImage(cv::Mat & threshImage) {
	pthread_mutex_lock(&thresholdImageMutex);
	threshImage.copyTo(thresholdImage);
	pthread_mutex_unlock(&thresholdImageMutex);
}

void DataStorage::setTargetImage(cv::Mat & targetImage) {
	pthread_mutex_lock(&targetImageMutex);
	targetImage.copyTo(targetImage);
	pthread_mutex_unlock(&targetImageMutex);
}

void DataStorage::setRedImage(cv::Mat & newImage) {
	pthread_mutex_lock(&redImageMutex);
	newImage.copyTo(redImage);
	pthread_mutex_unlock(&redImageMutex);
}

void DataStorage::setBlueImage(cv::Mat & newImage) {
	pthread_mutex_lock(&blueImageMutex);
	newImage.copyTo(blueImage);
	pthread_mutex_lock(&blueImageMutex);
}

cv::Mat DataStorage::copyBrightnessImage() {
	pthread_mutex_lock(&brightnessImageMutex);
	cv::Mat copy = brightnessImage.clone();
	pthread_mutex_unlock(&brightnessImageMutex);
	return copy;
}

cv::Mat DataStorage::copyThresholdImage() {
	pthread_mutex_lock(&thresholdImageMutex);
	cv::Mat copy = thresholdImage.clone();
	pthread_mutex_unlock(&thresholdImageMutex);
	return copy;
}

cv::Mat DataStorage::copyTargetImage() {
	pthread_mutex_lock(&targetImageMutex);
	cv::Mat copy = targetImage.clone();
	pthread_mutex_unlock(&targetImageMutex);
	return copy;
}

cv::Mat DataStorage::copyRedImage() {
	pthread_mutex_lock(&redImageMutex);
	cv::Mat copy = redImage.clone();
	pthread_mutex_unlock(&redImageMutex);
	return copy;
}

cv::Mat DataStorage::copyBlueImage() {
	pthread_mutex_lock(&blueImageMutex);
	cv::Mat copy = blueImage.clone();
	pthread_mutex_lock(&blueImageMutex);
	return copy;
}

