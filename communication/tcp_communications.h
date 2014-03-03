#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#include "../data_storage.h"
#include "../misc.h"
using namespace std;

class TCPServer {
	private:
	int sockfd;
	int forceRebind;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t clientaddrlen;
	int port;
	int connectionId;
	
	public:
	TCPServer(int port) {
		this->port = port;
		this->clientaddrlen = 0;
		this->forceRebind = 1;
		this->connectionId = -1;
	}
	
	int start() {
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
			perror("socket");
			return -1;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &forceRebind, sizeof(int)) == -1) {
			perror("setsockopt");
			return -1;
		}
		memset((void *) &server, 0, sizeof(struct sockaddr_in));
		server.sin_family = AF_INET;
		server.sin_port = htons(1180);
		server.sin_addr.s_addr = INADDR_ANY;
		if (bind(sockfd, (const struct sockaddr *) &server, sizeof(struct sockaddr_in)) == -1) {
			perror("bind");
			return -1;
		}
		if (listen(sockfd, 0) == -1) {
			perror("listen");
			return -1;
		}
		return 0;
	}
	
	int waitForConnection() {
		return (connectionId = accept(sockfd, (struct sockaddr *) &client, &clientaddrlen));
	}
	
	int write(const char * data, int length) {
		write((char*)data, length);
	}
	
	int write(unsigned char * data, int length) {
		write((char*)data, length);
	}
	
	int write(char * data, int length) {
		if (send(connectionId, data, length, 0) == -1) {
			perror("send parent");
			return -1;
		}
		return 0;
	}
	
	int read(const char * bufferData, int length) {
		read((char*)bufferData, length);
	}
	
	int read(unsigned char * bufferData, int length) {
		read((char*)bufferData, length);
	}
	
	int read(char * bufferData, int length) {
		int bytesread = recv(connectionId, bufferData, length, 0);
		if (bytesread == -1) {
			perror("recv");
			return -1;
		}
		return bytesread;
	}
	
	bool isConnected() {
		return connectionId != -1;
	}
	
	int closeConnection() {
		if (connectionId != -1 && close(connectionId) == -1) {
			perror("close(acceptfd)");
			connectionId = -1;
			return -1;
		}
		connectionId = -1;
		return 0;
	}
	
	int closeServer() {
		if (sockfd != -1 && close(sockfd) == -1) {
			perror("close(sockfd)");
			sockfd = -1;
			return -1;
		}
		sockfd = -1;
		return 0;
	}
	
	int stop() {
		if (closeConnection() == -1)
			return -1;
		if (closeServer() == -1)
			return -1;
		return 0;
	}
	
};

struct PROTOCOL_PACKET {
	unsigned char packetType;
	unsigned char length;
	unsigned char * data;
	bool closedConnection;
};

PROTOCOL_PACKET getAndProcessPacket(TCPServer & server) {
	PROTOCOL_PACKET packet;
	memset(&packet, 0, sizeof(packet));
	packet.closedConnection = false;
	unsigned char * readData = new unsigned char[64];
	int bytesRead = server.read(readData, 64);
	if (bytesRead < 2) {
		server.closeConnection();
		return packet;
	}
	int packetType = readData[0];
	int length = readData[1];
	unsigned char * data = new unsigned char[length];
	for (int i = 0; i < length; i++) {
		data[i] = readData[i+2];
	}
	packet.packetType = packetType;
	packet.length = length;
	packet.data = data;
	delete readData;
	return packet;
}

void * tcp_communications(void * arg) {
	cout << "Starting TCP Server...\n";
	TCPServer socket(1180);
	socket.start();
	cout << "Waiting for connection...\n";
	socket.waitForConnection();
	cout << "Connected via TCP.\n";
	while (true) {
		if (!socket.isConnected()) {
			cout << "Connection terminated, waiting for reconnection...\n";
			socket.waitForConnection();
			cout << "Reconnected via TCP.\n";
		}
		PROTOCOL_PACKET packet = getAndProcessPacket(socket);
		switch (packet.packetType) {
			case 0:
				socket.closeConnection();
				continue;
			case 1:
				cout << "Initialized Connection.\n";
				break;
			case 2:
				cout << "Request to start processing.\n";
				break;
			case 3:
				cout << "Request to stop processing.\n";
				break;
			case 4: {
				cout << "Request for target information.\n";
				vector <Target> targets = globalStorage.getTargets();
				const unsigned int TARGET_PACKET_SIZE = 5*8;
				unsigned int length = 2 + 1 + targets.size()*TARGET_PACKET_SIZE;
				unsigned char * sendData = new unsigned char[length];
				sendData[0] = 5;
				sendData[1] = length-2;
				sendData[2] = targets.size();
				cout << "\tTarget Count: " << targets.size() << "\n";
				cout << "\tPacket Length: " << length << "\n";
				for (unsigned int i = 0; i < targets.size(); i++) {
					double * targetData = (double*)(sendData+1+i*TARGET_PACKET_SIZE);
//					targetData[0] = targets[i].x / data->saveData->width - 0.5;
//					targetData[1] = targets[i].y / data->saveData->height - 0.5;
					targetData[0] = 0;
					targetData[1] = 0;
					targetData[2] = targetData[0] * 54;
					targetData[3] = targetData[1] * 41;
					targetData[4] = targets[i].angle;
				}
				socket.write(sendData, length);
				break;
			}
			case 5:
				cout << "I have no idea how I'M THE ONE getting this.. It's the target data\n";
				break;
			case 6: {
				if (packet.length > 0) {
					switch (packet.data[0]) {
						case 1:
							cout << "Request to set value for brightness to " << (int)packet.data[1] << "\n";
							data->vision->setBrightness(packet.data[1]);
							globalStorage.getSaveData()->brightness = packet.data[1];
							globalStorage.writeSaveData();
							break;
						case 2:
							cout << "Request to set thresholds to [" << (int)packet.data[1] << ", " << (int)packet.data[2] << "]\n";
							globalStorage.getSaveData()->threshMin = packet.data[1];
							globalStorage.getSaveData()->threshMax = packet.data[2];
							globalStorage.writeSaveData();
							break;
						case 3:
							globalStorage.setCompetitionMode(packet.data[1] == 1);
							cout << "Request to set competition mode to " << (packet.data[1] == 1 ? "ON" : "OFF") << "\n";
							break;
						case 4:
							globalStorage.setGameRecording(packet.data[1] == 1);
							cout << "Request to set game recording to " << (packet.data[1] == 1 ? "ON" : "OFF") << "\n";
							break;
						case 5: {
							int width = *((int*)&packet.data[1]);
							int height = *((int*)&packet.data[5]);
							globalStorage.getSaveData()->width = width;
							globalStorage.getSaveData()->height = height;
							globalStorage.writeSaveData();
							globalStorage.setVisionRestart(true);
							cout << "Request to set width/height to " << width << "x" << height << "\n";
							break;
						}
						default:
							cout << "Trying to set an unknown value (" << (int)packet.data[0] << ")\n";
							break;
					}
				}
				break;
			}
			case 7: {
				cout << "Received match data of length " << packet.length << "\n";
				globalStorage.writeToMatchFile(packet.data, packet.length);
			}
			default:
				cout << "Unknown packet type received (" << (int)packet.data[1] << ")\n";
				break;
		}
	}
	socket.stop();
	return 0;
}

