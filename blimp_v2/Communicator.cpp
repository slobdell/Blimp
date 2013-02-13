#include "Communicator.h"

Communicator::Communicator()
{
	com=new Tserial();

	jpegImagesInQueue=0;
	voidedFunctionCalls=0;
	maxJpegImagesInQueue=1;
	locked=false;
	sendLocked=false;
	commsEstablished=false;
	findCorrectComPort();
	
	
	
	/*
	if(com!=0)
	{
		com->connect("COM6", 57600, spNONE, true);
		//com->connect("COM6", 19200, spNONE);
	}
	*/
}
bool Communicator::establishCommunications()
{
	if(commsEstablished)return true;
	findCorrectComPort();
	return commsEstablished;
}
void Communicator::findCorrectComPort()
{
	int numTries=15;
	for(int tries=0;tries<numTries;tries++)
		for(int j=1;j<=16;j++)
		{
			comPort="COM"+to_string(j);
			if(com!=0)
			{
				
				com->disconnect();
				char *toSend=new char[5];
				memcpy(toSend, comPort.c_str()+0 /*terminates a C-style string*/, comPort.size()+1);


				com->connect(toSend, 57600, spNONE, true);
				wait_ms(100);
				//std::cout<<com->getChar()<<"<<<\n";
			}
			int bytesInBuffer=com->getNbrOfBytes();
			
			if(bytesInBuffer>0)
			{
				char *buf=new char[128];
				com->getArray(buf, 128);
				std::string output="";
				for( int j=0;j<128;j++)
				{
					output+=(char)buf[j];
				}
				int findIndex=output.find("xbee");
				
				if( findIndex>=0)
				{
					//WPRLED
					commsEstablished=true;
					this->enqueueMessage("found\n",6);
					this->sendData();
					return;
				}
				
				
		
			}
			

		}
		
		commsEstablished=false;
}

void Communicator::sendGeneralMessage(std::string &inString)
{
	char *toSend=new char[inString.size()];
	memcpy(toSend, inString.c_str(), inString.size());
	this->enqueueMessage("msg\n", 4);
	this->enqueueMessage(toSend, inString.size());
	this->sendData();
}

std::string Communicator::listen()
{
		
		//com->sendArray("this is a test", 14);
		//char myChar=com->getChar();
	

		int numBytes=com->getNbrOfBytes();
		std::string ret="";
		if(numBytes>0)
		{
			int bytesRead=com->getArray(buffer, numBytes);
			for( int j=0;j<bytesRead;j++)
			{
				//printf("%c",buffer[j]);
				try{
				ret+=buffer[j];
				}
				catch(int e)
				{
					std::cout<<"Error SBL xxxxxx\n";
				}

			}
			//printf("Number of bytes = %d\n", bytesRead);
		}
		return ret;
		
		
		
}

void Communicator::lock()
{
	if(locked)std::cout<<"my mutex works\n";
	while(locked)wait_ms(50);
	locked=true;
}
void Communicator::unlock()
{
	locked=false;
}

Communicator::~Communicator()
{
	
	com->disconnect();
}

void Communicator::sendData()
{
	if(sendLocked)return;//another thread is accessing this
	sendLocked=true;
	while(!messages.empty())
	{
		//Edit and Continue : warning 2004 : Cannot handle adding or removing a local 
		//variable with a name that is already defined more than once: this 
		//(Communicator::sendData)
		try{
		char *data=messages.front();
		int len=messageLengths.front();
		try
		{
			messages.pop();
		}
		catch(int e)
		{
			std::cout<<"Got an error popping a message at cp1\n";
		}
		try
		{
			messageLengths.pop();
		}
		catch(int e)
		{
			std::cout<<"Got an error popping a message at cp2\n";
		}

		if(data=="jpg\n")jpegImagesInQueue--;
		com->sendArray(data, len);
		}
		catch(int e)
		{
			std::cout<<"SBL OH SHIT!  Error at send data\n";
		}
	}
	sendLocked=false;
}

void Communicator::enqueueMessage(char *data, int len)
{
	
	if(data=="jpg\n")
	{
		if(jpegImagesInQueue<maxJpegImagesInQueue)
		{
			jpegImagesInQueue++;
		}
		else
		{
			voidedFunctionCalls++;
			return;
		}
	}
	else if(voidedFunctionCalls>=1 && voidedFunctionCalls<=3)
	{
			voidedFunctionCalls++;
			return;
	}
	else
	{
		voidedFunctionCalls=0;//I need to change this
	}
	
	//limit this to 3 jpeg frames at a time
	messages.push(data);
	messageLengths.push(len);
}
