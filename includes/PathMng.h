#ifndef __PATH_MNG_H__
#define __PATH_MNG_H__

#include <windows.h>
#include <string>
#include <iostream>
#include <algorithm>

const int CWD_BUF_SIZE = 256;

class PathMng {
	std::string CWD;
	const std::string DELEMETER = std::string("\\");
public:
	PathMng();
	void PrintCWD();
	std::string getAbsPath(const std::string& relPath);
};

extern PathMng pathMng;

#endif