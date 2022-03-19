#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "glm_pre.h"

#include "util.h"

#include <iostream>
#include <cmath>

class Camera {
public:
	float fovy;
	float cosHalfDiag;
	float aspect;
	float zNear;
	float zFar;

	glm::vec3 target;
	glm::vec3 pos;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	float rad;
	float theta;
	float phi;

	inline glm::mat4 getPerspectiveMat() {
		return glm::perspective(fovy, aspect, zNear, zFar);
	}
	inline glm::mat4 getViewMat() {
		return glm::lookAt(pos, target, up);
	}
	inline void updatePos() {
		pos = target - rad * front;
	}
	inline void updateRot() {
		float sinTheta, sinPhi, cosTheta, cosPhi;
		theta = clamp(theta,0.0f,180.0f);
		phi = std::fmod(phi, 360.0f);
		sinTheta = glm::sin(glm::radians(theta));
		sinPhi = glm::sin(glm::radians(phi));
		cosTheta = glm::cos(glm::radians(theta));
		cosPhi = glm::cos(glm::radians(phi));

		front = -glm::normalize(glm::vec3( sinTheta*sinPhi, cosTheta, sinTheta*cosPhi));
		up    =  glm::normalize(glm::vec3(-cosTheta*sinPhi, sinTheta,-cosTheta*cosPhi));
		right = glm::normalize(glm::cross(front,up));//glm::normalize(glm::vec3( sinTheta*cosPhi,        0,-sinTheta*sinPhi));
		updatePos();
		std::cout << "theta-phi : " << theta << "-" << phi << "\n";
		std::cout << "front : " << front.x << ", " << front.y << ", " << front.z << "\n";
		std::cout << "up : " << up.x << ", " << up.y << ", " << up.z << "\n";
		std::cout << "right : " << right.x << ", " << right.y << ", " << right.z << "\n";
	}
	inline void calcViewCone() {
		double x = aspect;
		double y = 1.0;
		double h = y / glm::tan(glm::radians(fovy*0.5));

		double tanHalfDiag = glm::sqrt(x * x + y * y)/h;
		double tanHalfFovx = aspect * glm::tan(glm::radians(fovy * 0.5));
		
		cosHalfDiag = (float)(cos(atan(tanHalfDiag)*1.3));
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