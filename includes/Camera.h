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
	glm::vec3 lightDir;
	glm::vec3 color;
	GLuint depthMap;
	GLuint shadow_resolution;
	Sun(const glm::vec3& _lDir,GLuint _resolution,GLuint _scrX, GLuint _scrY):lightDir(_lDir),shadow_resolution(_resolution),scrX(_scrX),scrY(_scrY) {
		glGenFramebuffers(1, &depthFBO);
		glGenTextures(1, &depthMap);
		
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,shadow_resolution,shadow_resolution,0, GL_DEPTH_COMPONENT,GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,depthMap,0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	inline void begin() {
		time = timeMng.getEffectiveTime() * 0.3;
		glm::vec3 lightPos;
		lightPos.x = sin(time) * 3.0f;
		lightPos.z = cos(time) * 2.0f;
		lightPos.y = 1.5f + cos(time) * 1.0f;
		lightDir = glm::normalize(lightPos);
		float highnoon = clamp(lightDir.y, 0.0f, 1 / 10.0f) * 10.0f;
		color = glm::mix(glm::vec3(1.0f), glm::vec3(0.99f, 0.80, 0.44f), highnoon);
		//time = 1.750;
		glViewport(0, 0, shadow_resolution, shadow_resolution);
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		//glCullFace(GL_FRONT);
	}
	inline void end() {
		//glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, scrX, scrY);
	}
	inline glm::mat4 getOrtho(float landSize) {
		landSize *= SQRT2 * 0.5f;
		float wFactor = 1.0f;
		float hFactor = 1.0f;
		return glm::ortho(-landSize*wFactor,landSize*wFactor,-landSize*hFactor,landSize*hFactor,0.0f, landSize*3.0f);
	}
	inline glm::mat4 getViewMat(float landSize) {
		//lightDir = glm::normalize(-mainCam->front);
		glm::vec3 up(0.0, 1.0, 0.0);
		up = normalize(up);
		return glm::lookAt(glm::normalize(lightDir)*(landSize),
							glm::vec3(0.0f),
							up);
	}
	inline glm::mat4 lightSpaceMat(float landSize) {
		return getOrtho(landSize) * getViewMat(landSize);
	}
};
extern Sun* sun;
#endif