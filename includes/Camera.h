#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "glm_pre.h"

class Camera {
public:
	float fovy;
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
};
extern Camera* mainCam;
#endif