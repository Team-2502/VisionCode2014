#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "../misc.h"
#include "httpserver.h"

int answer_to_connection(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
	USERDATA * userdata = (USERDATA*)((HTTP_USERDATA*)cls)->data;
	RaspiVid * vision = userdata->vision;
	
	if (userdata->status & STATUS_INITIALIZING)
		return MHD_HTTP_NO_CONTENT;
	
	URL_COMPONENTS args = getAttributes(url);
	
	string contentType = "text/html; charset=utf-8";
	string page = "";
	vector <unsigned char> data;
	
	if (args.request == "set") {
		page.append("<html><body>");
		cout << "Received set-request with URL: " << url << "\n";
		for (unsigned int i = 0; i < args.attributes.size(); i++) {
			string key = args.attributes[i].key;
			int numAttr = args.attributes[i].values.size();
			if (key == "brightness") {
				if (numAttr >= 1) {
					int brightness = str2int(args.attributes[i].values[0]);
					userdata->saveData->brightness = brightness;
					vision->setBrightness(brightness);
					page.append("Successfully set brightness to ");
					page.append(args.attributes[i].values[0]);
					page.append(".<br />");
				} else {
					page.append("Unable to set brightness! It requires at least 1 argument (Brightness).<br />");
				}
			} else if (key == "threshold") {
				if (numAttr >= 2) {
					int min = str2int(args.attributes[i].values[0].c_str());
					int max = str2int(args.attributes[i].values[1].c_str());
					if (min != -1)
						userdata->saveData->threshMin = min;
					if (max != -1)
						userdata->saveData->threshMax = max;
					page.append("Successfully set threshold to [");
					page.append(args.attributes[i].values[0]);
					page.append(", ");
					page.append(args.attributes[i].values[1]);
					page.append("].<br />");
				} else {
					page.append("Unable to set threshold! It requires at least 2 arguments (Min, Max).<br />");
				}
			} else if (key == "contour_area") {
				if (numAttr >= 2) {
					float min = str2float(args.attributes[i].values[0].c_str());
					float max = str2float(args.attributes[i].values[1].c_str());
					if (min != -1)
						userdata->saveData->contourMin = min / 100;
					if (max != -1)
						userdata->saveData->contourMax = max / 100;
					page.append("Successfully set contour_area to [");
					page.append(args.attributes[i].values[0]);
					page.append(", ");
					page.append(args.attributes[i].values[1]);
					page.append("].<br />");
				} else {
					page.append("Unable to set contour_area! It requires at least 2 arguments (Min%, Max%).<br />");
				}
			} else {
				page.append("Unknown attribute: '");
				page.append(key);
				page.append("'<br />");
			}
		}
		page.append("</body></html>");
	} else if (args.request == "image.bmp") {
		cout << "Received request for image.bmp [" << vision->getWidth() << "x" << vision->getHeight() << "]\n";
		contentType = "image/bmp";
		pthread_mutex_lock(&mutHttpRawImage);
		Mat image = httpRawImage.clone();
		pthread_mutex_unlock(&mutHttpRawImage);
		if (!imencode(".bmp", image, data)) {
			cout << "  Could not encode image!\n";
		}
		page = "";
	} else if (args.request == "threshold.bmp") {
		cout << "Received request for threshold.bmp [" << vision->getWidth() << "x" << vision->getHeight() << "]\n";
		contentType = "image/bmp";
		pthread_mutex_lock(&mutHttpThreshImage);
		Mat image = httpThreshImage.clone();
		pthread_mutex_unlock(&mutHttpThreshImage);
		if (!imencode(".bmp", image, data)) {
			cout << "  Could not encode image!\n";
		}
		page = "";
	} else if (args.request == "targets.bmp") {
		cout << "Received request for targets.bmp [" << vision->getWidth() << "x" << vision->getHeight() << "]\n";
		contentType = "image/bmp";
		pthread_mutex_lock(&mutHttpTargetImage);
		Mat image = httpTargetImage.clone();
		pthread_mutex_unlock(&mutHttpTargetImage);
		if (!imencode(".bmp", image, data)) {
			cout << "  Could not encode image!\n";
		}
		page = "";
	} else if (args.request == "current") {
		cout << "Received request for current calibration values.\n";
		char buf[16];
		page.append("item,value1,value2...\n");
		page.append("brightness,"); page.append(int2str(buf, userdata->saveData->brightness)); page.append("\n");
		page.append("threshold,");
		page.append(int2str(buf, userdata->saveData->threshMin)); page.append(",");
		page.append(int2str(buf, userdata->saveData->threshMax)); page.append("\n");
		page.append("contour_area,");
		page.append(int2str(buf, userdata->saveData->contourMin*100)); page.append(",");
		page.append(int2str(buf, userdata->saveData->contourMax*100)); page.append("\n");
	} else if (args.request == "target_data") {
		cout << "Received request for current target data.\n";
		char buf[16];
		vector <Target> targets = *userdata->targets;
		page.append("count,"); page.append(int2str(buf, targets.size())); page.append("\n");
		for (unsigned int i = 0; i < targets.size(); i++) {
			page.append("["); page.append(int2str(buf, targets[i].x)); page.append(",");
			page.append(int2str(buf, targets[i].y)); page.append("],[");
			page.append(int2str(buf, targets[i].width)); page.append("x");
			page.append(int2str(buf, targets[i].height)); page.append("],");
			page.append(float2str(buf, targets[i].angle)); page.append(",");
			page.append(float2str(buf, targets[i].rect)); page.append(",");
			page.append(float2str(buf, targets[i].dist)); page.append(",");
			page.append(float2str(buf, targets[i].distError));
			page.append("\n");
		}
	} else if (args.request == "calibration_status") {
		cout << "Received request for calibration status.\n";
		char buf[16];
		page.append("brightness,"); page.append(int2str(buf, userdata->calStatus->brightness)); page.append("\n");
		page.append("completion,"); page.append(float2str(buf, userdata->calStatus->percentComplete)); page.append("\n");
	} else if (args.request == "information") {
		char buf[16];
		page.append("framerate,"); page.append(float2str(buf, userdata->framerate)); page.append("\n");
	} else if (args.request == "auto_calibrate") {
		cout << "Starting Auto-calibrate.\n";
		userdata->calTargets->clear();
		for (unsigned int i = 0; i < args.attributes.size(); i++) {
			string key = args.attributes[i].key;
			int numAttr = args.attributes[i].values.size();
			if (key == "target") {
				if (numAttr >= 2) {
					int x = str2int(args.attributes[i].values[0]);
					int y = str2int(args.attributes[i].values[1]);
					userdata->calTargets->push_back((CalibrateTarget){x, y});
					page.append("Successfully remembered target at [");
					page.append(args.attributes[i].values[0]);
					page.append(", ");
					page.append(args.attributes[i].values[1]);
					page.append("]<br />");
				} else {
					page.append("Invalid Target! It requires two points: x,y<br />");
				}
			}
		}
		userdata->status = STATUS_RUNNING | STATUS_CALIBRATING;
	} else {
		page = "<html><body>Nothing to display. Invalid query.</body></html>";
	}
	
	saveData(userdata->saveDataFile, *userdata->saveData);
	
	struct MHD_Response * response;
	
	if (contentType == "text/html; charset=utf-8") {
		response = MHD_create_response_from_buffer(page.length(), (void *)page.c_str(), MHD_RESPMEM_MUST_COPY);
	} else {
		response = MHD_create_response_from_buffer(data.size(), (void *)&data[0], MHD_RESPMEM_MUST_COPY);
	}
	
	MHD_add_response_header(response, "Content-type", contentType.c_str());
	MHD_add_response_header(response, MHD_HTTP_HEADER_CONNECTION, "close");
	
	int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);
	
	return ret;
}

#endif
