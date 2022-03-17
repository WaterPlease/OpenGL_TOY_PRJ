#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <string>
#include <iostream>

class Logger{
public:
	void Log(const char* src, const int line, const char* msg) {
		std::cout << "[" << src << "::" << line << "] : ";
		Log(msg);
	}
	void Log(std::string& msg) {
		std::cout << msg << std::endl;
	}
	void Log(const char* msg) {
		std::cout << msg << std::endl;
	}
};

extern Logger logger;

#endif