#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "glm_pre.h"

#include "Shader.hpp"

#include "util.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>

#define SQRT2 1.414213

class Camera {
	glm::mat4 prjMat;
	glm::mat4 viewMat;
	glm::mat4 invViewMat;
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
	inline void UpdateMat() {
		prjMat = glm::perspective(fovy, aspect, zNear, zFar);
		viewMat = glm::lookAt(pos, target, up);
		invViewMat = glm::inverse(viewMat);
	}
	inline const glm::mat4& GetPerspectiveMat() {
		return prjMat;
	}
	inline const glm::mat4& GetViewMat() {
		return viewMat;
	}
	inline const glm::mat4& GetInvViewMat() {
		return invViewMat;
	}
	inline void UpdatePos() {
		pos = target - rad * front;
	}
	inline void UpdateRot() {
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
		UpdatePos();
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

class Sun{
	GLuint depthFBO;
	GLuint scrX;
	GLuint scrY;
	double time;
	glm::mat4 transMat;
public:
	float lightStrength = 1.0f;
	glm::vec3 lightDir;
	glm::vec3 color;
	GLuint depthMap;
	GLuint shadow_resolution;
	Sun(const glm::vec3& _lDir, GLuint _resolution, GLuint _scrX, GLuint _scrY);

	void Begin();
	void End();
	inline glm::mat4 GetOrtho(float landSize) {
		landSize *= SQRT2 * 0.5f;
		float wFactor = 1.0f;
		float hFactor = 1.0f;
		return glm::ortho(-landSize*wFactor,landSize*wFactor,-landSize*hFactor,landSize*hFactor,0.0f, landSize*3.0f);
	}
	inline glm::mat4 GetViewMat(float landSize) {
		//lightDir = glm::normalize(-mainCam->front);
		glm::vec3 up(0.0, 1.0, 0.0);
		up = normalize(up);
		return glm::lookAt(glm::normalize(lightDir)*(landSize),
							glm::vec3(0.0f),
							up);
	}
	inline glm::mat4 lightSpaceMat(float landSize) {
		return GetOrtho(landSize) * GetViewMat(landSize);
	}
};
extern Sun* sun;
#endif