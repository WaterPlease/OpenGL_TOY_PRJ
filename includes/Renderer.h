#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <winerror.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glm_pre.h"
#include "Logger.h"
#include "Object.h"
#include "Camera.h"
#include "Parameter.h"

class Renderer {
	glm::uvec2 screenRes = glm::uvec2(800, 600);
	GLFWwindow* window;
public:
	Renderer(const char* title,int width, int height);

	inline void startFrameRender(){
		glClearColor(0.29f, 0.66f, 0.85f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	inline void endFrameRender(){
		glfwSwapBuffers(window);

		parameter_changed = false;
	}
	inline void draw(BaseObject& obj) { obj.draw(); }

	// get-set
	inline glm::uvec2 getScreenRes() { return screenRes; }
	inline void setScreenRes(glm::uvec2& res) { screenRes = res; }

	inline GLFWwindow* getWindow() { return window; };
};

extern bool wireMode;
#endif