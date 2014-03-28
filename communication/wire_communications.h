#ifndef WIRE_COMMUNICATIONS_H
#define WIRE_COMMUNICATIONS_H
#include <wiringPi.h>

#define GPIO_OUTPUT 1
#define GPIO_INPUT 0

int configureWire() {
	if (wiringPiSetup () == -1)
		return -1;
	pinMode(GPIO_OUTPUT, OUTPUT); // GPIO 8
	pinMode(GPIO_INPUT, INPUT); // GPIO 7
	return 0;
}

void setHotWire(bool hot) {
	digitalWrite(GPIO_OUTPUT, hot ? 1 : 0);
}

bool isProcessingStarted() {
	return digitalRead(GPIO_INPUT) >= 1;
}

#endif // WIRE_COMMUNICATIONS_H
