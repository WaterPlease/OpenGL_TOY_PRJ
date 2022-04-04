#ifndef __UTIL_H__
#define __UTIL_H__

#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


template<class T>
inline const T& clamp(const T& v, const T& lo, const T& hi) {
	return std::min(std::max(v, lo), hi);
}

class TimeMng {
	double eTime;
	double rTime;
	double eDt;
	double rDt;
public:
	double speed;
	TimeMng(double _speed=1.0):speed(_speed) {
		eTime = glfwGetTime();
		rTime = 0.0;
		eDt = rDt = 0.0;
	}
	inline void updateTime() {
		double old = rTime;
		rTime = glfwGetTime();
		rDt = rTime - old;

		eDt = speed * rDt;
		eTime += eDt;
	}

	inline double getEffectiveTime() { return eTime; }
	inline double getEffectiveDelta() { return eDt;  }
	inline double getRealTime() { return rTime; }
	inline double getRealDelta() { return rDt;  }
	inline double getSpeed() { return speed; }
	inline void   setSpeed(double _speed) { speed = _speed; }
};
extern TimeMng timeMng;


extern GLuint gBuffer_position;
extern GLuint gBuffer_Albedo;
extern GLuint gBuffer_Normal;

extern GLuint gl_SSBO_FLY;
extern GLuint gl_texture_heightmap;
#endif