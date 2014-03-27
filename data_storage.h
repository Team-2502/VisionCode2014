#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H
#include <opencv2/core/core.hpp>
#include <vector>

#include "types.h"
using namespace std;

class DataStorage {
	private:
	static DataStorage * DataStorageInstance;
	
	DataStorage();
	USERDATA * data;
	volatile bool visionRestart;
	volatile bool competitionMode;
	volatile bool gameRecording;
	int videoFile;
	int matchFile;
	string saveFilename;
	SaveData * saveData;
	vector <Target> targets;
	vector <CalibrateTarget> calTargets;
	cv::Mat brightnessImage;
	cv::Mat thresholdImage;
	cv::Mat targetImage;
	cv::Mat redImage;
	cv::Mat blueImage;
	pthread_mutex_t brightnessImageMutex;
	pthread_mutex_t thresholdImageMutex;
	pthread_mutex_t targetImageMutex;
	pthread_mutex_t redImageMutex;
	pthread_mutex_t blueImageMutex;
	
	bool fileExists(const char * filename);
	void readSaveData();
	
	bool writeToFile(int filePtr, unsigned char * data, unsigned int length);
	
	public:
	~DataStorage();
	
	static DataStorage & Get();
	
	void openSaveData(const char * saveFile);
	void writeSaveData();
	SaveData * getSaveData();
	
	USERDATA * getUserdata();
	
	void openVideoFile(const char * videoFilename);
	bool writeToVideoFile(unsigned char * data, unsigned int length);
	void closeVideoFile();
	bool isVideoFileOpened();
	
	void openMatchFile(const char * matchFilename);
	bool writeToMatchFile(unsigned char * data, unsigned int length);
	void closeMatchFile();
	bool isMatchFileOpened();
	
	void setVisionRestart(bool restart);
	volatile bool isVisionRestarting();
	
	void setCompetitionMode(bool mode);
	volatile bool isCompetitionMode();
	
	void setGameRecording(bool recording);
	volatile bool isGameRecording();
	
	void setTargets(vector <Target> targets);
	vector <Target> getTargets();
	
	void setCalibrationTargets(vector <CalibrateTarget> targets);
	vector <CalibrateTarget> getCalibrationTargets();
	
	void setBrightnessImage(cv::Mat & newImage);
	void setThresholdImage(cv::Mat & threshImage);
	void setTargetImage(cv::Mat & targetImage);
	void setRedImage(cv::Mat & newImage);
	void setBlueImage(cv::Mat & newImage);
	
	cv::Mat copyBrightnessImage();
	cv::Mat copyThresholdImage();
	cv::Mat copyTargetImage();
	cv::Mat copyRedImage();
	cv::Mat copyBlueImage();
	
};

#endif // DATA_STORAGE_H
