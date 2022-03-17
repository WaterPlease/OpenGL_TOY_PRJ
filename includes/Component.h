#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#include <vector>
#include <string>

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include "glm_pre.h"

class Component {
public:
	virtual std::string getName() = 0;
	virtual void OnUpdate() = 0;
};

class CompTransform : public Component {
public:
	glm::vec3 pos;
	glm::vec3 rot;
	void OnUpdate(){}
	std::string getName() { return std::string("Transform"); }
};
#endif