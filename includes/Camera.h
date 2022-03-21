#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "glm_pre.h"

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

class Sun{
	GLuint depthFBO;
	GLuint scrX;
	GLuint scrY;
	double time;
public:
	glm::vec3 lightDir;
	GLuint depthMap;
	GLuint shadow_resolution;
	Sun(const glm::vec3& _lDir,GLuint _resolution,GLuint _scrX, GLuint _scrY):lightDir(_lDir),shadow_resolution(_resolution),scrX(_scrX),scrY(_scrY) {
		glGenFramebuffers(1, &depthFBO);
		glGenTextures(1, &depthMap);
		
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,shadow_resolution,shadow_resolution,0, GL_DEPTH_COMPONENT,GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
		time = glfwGetTime() * 0.5;
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
		float wFactor = 1.3f;
		float hFactor = 1.0f;
		return glm::ortho(-landSize*wFactor,landSize*wFactor,-landSize*hFactor,landSize*hFactor,0.0f, landSize*3.0f);
	}
	inline glm::mat4 getViewMat(float landSize) {
		glm::vec3 lightPos;
		lightPos.x = sin(time) * 3.0f;
		lightPos.z = cos(time) * 2.0f;
		lightPos.y = 1.5f + cos(time) * 1.0f;
		lightDir = glm::normalize(lightPos);
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