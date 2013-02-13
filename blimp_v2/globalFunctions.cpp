#include "globalFunctions.h"

void wait_ms(unsigned long ms)
{
	unsigned long nowtime=GetTickCount();
	while ((GetTickCount()-nowtime)<=ms);
}

std::vector<std::string> split(const std::string &s, char delim, std::vector<std::string> &elems)
{
	std::stringstream ss(s);
	
	std::string item;
	while(std::getline(ss, item, delim))
	{
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim) 
{
	std::vector<std::string> elems;
	return split(s, delim, elems);
}

std::string to_string(double x)
{
	std::ostringstream ss;
	ss<<x;
	return ss.str();
}