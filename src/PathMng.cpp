#include "PathMng.h"

PathMng::PathMng() {
	TCHAR buf[CWD_BUF_SIZE];
	std::wstring tmp;
	GetCurrentDirectory(CWD_BUF_SIZE, buf);
	tmp = buf;
	CWD = std::string(tmp.begin(), tmp.end());
}
void PathMng::PrintCWD() {
	std::cout << CWD << std::endl;
}


std::string PathMng::getAbsPath(const std::string& relPath) {
	std::string _relPath = std::string(relPath);
	std::replace(_relPath.begin(), _relPath.end(), '/', '\\');

	return CWD + DELEMETER + _relPath;
}





PathMng pathMng;