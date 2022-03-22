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
#include "GUI.h"

class RenderPIPE {
	GLuint fbo;
public:
	RenderPIPE() {
		glGenFramebuffers(1, &fbo);
	}

	void begin() {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}

	void end() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	~RenderPIPE() {
		glDeleteFramebuffers(1, &fbo);
	}
};
class HDRPIPE :public RenderPIPE {
	GLuint rbo;
	GLuint nextFBO;
	GLuint quadVAO;
	GLuint quadVBO;
	Shader* hdrShader;
public:
	GLuint colorTexture;
	HDRPIPE(const glm::uvec2& screenRes, GLuint nextPIPEFBO = 0) :RenderPIPE(), nextFBO(nextPIPEFBO) {

		begin();

		glGenTextures(1, &colorTexture);
		glBindTexture(GL_TEXTURE_2D, colorTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenRes.x, screenRes.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			exit(1);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//end();
		float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
		};

		glGenVertexArrays(1, &quadVAO);
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		std::cout << "HDR SHADER\n";
		hdrShader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\postProcess.vs",
			"C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\hdrShader.fs");
		std::cout << "HDR SHADER DONE\n";
	}

	void begin() {
		RenderPIPE::begin();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void end() {
		glBindFramebuffer(GL_FRAMEBUFFER, nextFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		hdrShader->use();
		if (parameter_changed) {
			hdrShader->setFloat("gamma", gamma);
		}

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(hdrShader->ID, "screenTexture"), 0);
		glBindTexture(GL_TEXTURE_2D, colorTexture);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
};
class Renderer {
	GLuint skyboxTexture;
	GLuint skyboxVAO, skyboxVBO;
	Shader* skyboxShader;
	GLFWwindow* window;

	
public:
	HDRPIPE* hdrPIPE;
	glm::uvec2 screenRes = glm::uvec2(800, 600);
	Renderer(const char* title,int width, int height);
	glm::vec4 clearColor;

	inline void startFrameRender(){
		hdrPIPE->begin();
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	}
	inline void postProcess() {
		hdrPIPE->end();
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

	void drawSkybox() {
		glDepthFunc(GL_LEQUAL);
		skyboxShader->use();

		skyboxShader->setVec3("camPos", mainCam->pos);
		skyboxShader->setMat4("projection", mainCam->getPerspectiveMat());
		skyboxShader->setMat4("view", glm::mat4(glm::mat3(mainCam->getViewMat())));

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(skyboxShader->ID, "skybox"), 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		
		glDepthFunc(GL_LESS);
	}

	inline GLuint getSkyboxTexture() { return skyboxTexture; }
};

extern bool wireMode;


#endif