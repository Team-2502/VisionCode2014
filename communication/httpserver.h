#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <microhttpd.h>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

typedef struct {
	string key;
	vector <string> values;
} ATTRIBUTE;

typedef struct {
	string request;
	vector <ATTRIBUTE> attributes;
} URL_COMPONENTS;

typedef struct {
	void * data;
} HTTP_USERDATA;

URL_COMPONENTS getAttributes(const char * url) {
		URL_COMPONENTS urlComponents;
		vector <ATTRIBUTE> attributes;
		string request = "";
		string attr = "";
		string value = "";
		vector <string> values;
		int mode = 0;
		for (int i = 1; url[i] != 0; i++) {
			if (url[i] == ':') {
				mode = 1;
			} else if (url[i] == '&') {
				mode = 1;
				values.push_back(value);
				attributes.push_back((ATTRIBUTE){attr, values});
				attr = "";
				value = "";
				values.clear();
			} else if (url[i] == '=') {
				mode = 2;
			} else if (url[i] == ',') {
				values.push_back(value);
				value = "";
			} else {
				switch (mode) {
					case 0:
						request.push_back(url[i]);
						break;
					case 1:
						attr.push_back(url[i]);
						break;
					case 2:
						value.push_back(url[i]);
						break;
						
					default:
						break;
				}
			}
		}
		if (value.length() > 0) {
			values.push_back(value);
		}
		if (attr.length() > 0)
			attributes.push_back((ATTRIBUTE){attr, values});
		else if (value.length() > 0)
			attributes.push_back((ATTRIBUTE){"", values});
		
		urlComponents.request = request;
		urlComponents.attributes = attributes;
		return urlComponents;
	}

int answer_to_connection (	void *cls, struct MHD_Connection *connection,
							const char *url, const char *method,
							const char *version, const char *upload_data,
							size_t *upload_data_size, void **con_cls);

class HttpServer {
	private:
	struct MHD_Daemon *daemon;
	HTTP_USERDATA * userdata;
	int port;
	bool initialized;
	
	public:
	HttpServer(int port, HTTP_USERDATA * data) {
		this->port = port;
		this->initialized = false;
		this->userdata = data;
		initialize();
	}
	
	bool initialize() {
		if (initialized)
			return true;
		daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, port, NULL, NULL, &answer_to_connection, (void*)userdata, MHD_OPTION_END);
		if (NULL == daemon) {
			cout << "[ERROR] Daemon is null!\n";
			return (initialized = false);
		}
		return (initialized = true);
	}
	
	void close() {
		if (!initialized)
			return;
		MHD_stop_daemon(daemon);
		initialized = false;
	}
	
	~HttpServer() {
		close();
	}
	
};

#endif // HTTPSERVER_H
