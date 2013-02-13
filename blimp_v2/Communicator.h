#pragma once
#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include "commonIncludes.h"
#include <queue>

class Communicator
{
private:
	Tserial *com;
	char buffer[1024];
	std::queue<char *> messages;
	std::queue<int> messageLengths;
	int jpegImagesInQueue;
    int maxJpegImagesInQueue;
	int voidedFunctionCalls;
	bool locked;
	bool sendLocked;
	void findCorrectComPort();
	bool commsEstablished;
	std::string comPort;
public:
	Communicator();
	std::string listen();
	void enqueueMessage(char *data, int len);
	void sendData();
	~Communicator();
	void lock();
	void unlock();
	void sendGeneralMessage(std::string &inString);
	bool establishCommunications();
};

#endif