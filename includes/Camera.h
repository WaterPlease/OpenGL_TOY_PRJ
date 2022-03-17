#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "glm_pre.h"

#include <iostream>

class Camera {
public:
	float fovy;
	float cosHalfDiag;
	float aspect;
	float zNear;
	float zFar;

	glm::vec3 pos;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;

	inline glm::mat4 getPerspectiveMat() {
		return glm::perspective(fovy, aspect, zNear, zFar);
	}
	inline glm::mat4 getViewMat() {
		return glm::lookAt(pos, pos + front, up);
	}
	inline void updateRot(const glm::vec3& _front,const glm::vec3& refVec, bool useUpVec=true) {
		if (useUpVec) {
			front = glm::normalize(_front);
			up = glm::normalize(refVec);
			right = glm::normalize(glm::cross(front, up));
		}
		else {
			front = glm::normalize(_front);
			right = glm::normalize(refVec);
			up = glm::normalize(glm::cross(right, front));
		}
	}
	inline void calcViewCone() {
		double x = aspect;
		double y = 1.0;
		double h = y / glm::tan(glm::radians(fovy*0.5));

		double tanHalfDiag = glm::sqrt(x * x + y * y)/h;
		double tanHalfFovx = aspect * glm::tan(glm::radians(fovy * 0.5));
		
		cosHalfDiag = (float)(cos(atan(tanHalfDiag)*1.22));
		double cosHalfFovx = (float)(cos(atan(tanHalfFovx)));

		//cosHalfDiag = (float)(1.0/glm::sqrt(1.0+(x*x+y*y)/(h*h)));
		std::cout << "aspect : " << aspect << "\n";
		std::cout << "cosHalfDiag : "<< cosHalfDiag << "\n";
		std::cout << "cosHalfFovy : " << glm::cos(glm::radians(fovy)*0.5)<<"\n";
		std::cout << "cosHalfFovx : " << cosHalfFovx << "\n";
	}
};
extern Camera* mainCam;
#endif