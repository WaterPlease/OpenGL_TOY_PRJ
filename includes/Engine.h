#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <winerror.h>
#include <ctime>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glm_pre.h"
#include "Logger.h"
#include "Renderer.h"

#include "Object.h"
#include "GUI.h"

class Engine {
	std::string title;
	Renderer renderer;
	GLFWwindow* window;
	vector<BaseObject*> objs;
public:
	Engine(const char* title, int width, int height);
	void Loop();
};

void engine_resoultion_callback(GLFWwindow* window, int width, int height);
void engine_input_handler(GLFWwindow* window);

#endif