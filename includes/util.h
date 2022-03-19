#ifndef __UTIL_H__
#define __UTIL_H__

#include <algorithm>

template<class T>
inline const T& clamp(const T& v, const T& lo, const T& hi) {
	return std::min(std::max(v, lo), hi);
}

#endif