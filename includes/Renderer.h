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
public:
	GLuint fbo;
	RenderPIPE() {
		glGenFramebuffers(1, &fbo);
	}

	void Begin() {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}

	void End() {
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
	Shader* extractShader;
	Shader* blurShader;
	glm::uvec2 screenRes;
public:
	GLuint colorTexture;
	GLuint extractTexture;
	GLuint blurredTexture;
	HDRPIPE(const glm::uvec2& _screenRes, GLuint nextPIPEFBO = 0);

	void Begin();

	void End();
};

struct lightIndex{
	GLuint indexLstSize;
	GLuint indexLst[48 * 30 * 8192];
	glm::uvec2 gridCell[48 * 30];   // offset, size
};
struct Plane {
	GLfloat x;
	GLfloat y;
	GLfloat z;
};
class DEFFEREDPIPE :public RenderPIPE {
	GLuint rbo;
	GLuint nextFBO;
	GLuint quadVAO;
	GLuint quadVBO;
	Shader* lightingShader;
	ComputeShader* lightCullShader;
public:
	GLuint gPositionMetal,gNormalRough,gAlbedoAO;//alpha value of albedo buffer is specular value.
	GLuint SSBO_lightIndex;
	Plane xPlanes[48+1]; // 40 pixels
	Plane yPlanes[30+1]; // 36 pixels
	const GLuint ZERO=0;
	DEFFEREDPIPE(const glm::uvec2& screenRes, GLuint nextPIPEFBO = 0);

	void Begin();

	void End();
};

class SSRPIPE :public RenderPIPE {
	GLuint rbo;
	GLuint nextFBO;
	GLuint quadVAO;
	GLuint quadVBO;
	glm::uvec2 gBufferImages;
	GLuint texture_skybox;
	Shader* ssrShader;
	float RES_X, RES_Y;
public:
	GLuint texture_color;
	SSRPIPE(const glm::uvec2& screenRes, GLuint nextPIPEFBO = 0);

	inline void gBufferBind(GLuint gPosMetla, GLuint gNormalRough) {
		gBufferImages = glm::uvec2(gPosMetla,gNormalRough);
	}
	inline void skyboxBind(GLuint skybox) {
		texture_skybox = skybox;
	}

	void Begin();

	void End();
};

class Renderer {
	GLuint skyboxTexture;
	GLuint skyboxVAO, skyboxVBO;
	Shader* skyboxShader;
	GLFWwindow* window;

	
public:
	DEFFEREDPIPE* defferedPIPE;
	SSRPIPE* ssrPIPE;
	HDRPIPE* hdrPIPE;
	glm::uvec2 screenRes;
	Renderer(const char* title,int width, int height);
	glm::vec4 clearColor;

	inline void StartFrameRender(){
		defferedPIPE->Begin();
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	}
	inline void postProcess() {
		defferedPIPE->End();
		ssrPIPE->End();
		hdrPIPE->End();
	}
	inline void endFrameRender(){
		glfwSwapBuffers(window);

		bPrameterChange = false;
	}
	inline void draw(BaseObject& obj) { obj.draw(); }

	// get-set
	inline glm::uvec2 getScreenRes() { return screenRes; }
	inline void setScreenRes(glm::uvec2& res) { screenRes = res; }

	inline GLFWwindow* getWindow() { return window; };

	void drawSkybox();

	inline GLuint getSkyboxTexture() { return skyboxTexture; }
};

#endif