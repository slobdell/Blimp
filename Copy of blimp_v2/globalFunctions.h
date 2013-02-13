#pragma once
#ifndef test
#define test
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <winio.h>
#include <vector>
#include <queue>
#include <string>
#include <sstream>




void wait_ms(unsigned long ms);

std::vector<std::string> split(const std::string &s, char delim, std::vector<std::string> &elems);

std::vector<std::string> split(const std::string &s, char delim);
std::string to_string(double x);

#endif