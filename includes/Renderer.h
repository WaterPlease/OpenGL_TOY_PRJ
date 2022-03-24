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
	Shader* extractShader;
	Shader* blurShader;
	glm::uvec2 screenRes;
public:
	GLuint colorTexture;
	GLuint extractTexture;
	GLuint blurredTexture;
	HDRPIPE(const glm::uvec2& _screenRes, GLuint nextPIPEFBO = 0) :RenderPIPE(), nextFBO(nextPIPEFBO),screenRes(_screenRes) {

		begin();

		glGenTextures(1, &colorTexture);
		glBindTexture(GL_TEXTURE_2D, colorTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

		// bloom init
		glGenTextures(1, &extractTexture);
		glBindTexture(GL_TEXTURE_2D, extractTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glGenTextures(1, &blurredTexture);
		glBindTexture(GL_TEXTURE_2D, blurredTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		extractShader = new ComputeShader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\extract.cs");
		blurShader = new ComputeShader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\blur.cs");


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

		extractShader->use();
		glBindImageTexture(0, colorTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
		glBindImageTexture(1, extractTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glDispatchCompute(screenRes.x / 32, screenRes.y / 18,1);
		
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		blurShader->use();
		glBindImageTexture(0, extractTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
		glBindImageTexture(1, blurredTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glDispatchCompute(screenRes.x / 32, screenRes.y / 18, 1);

		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		hdrShader->use();
		if (parameter_changed) {
			hdrShader->setFloat("gamma", gamma);
		}

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(hdrShader->ID, "screenTexture"), 0);
		glBindTexture(GL_TEXTURE_2D, colorTexture);

		glActiveTexture(GL_TEXTURE1);
		glUniform1i(glGetUniformLocation(hdrShader->ID, "bloomTexture"), 1);
		glBindTexture(GL_TEXTURE_2D, blurredTexture);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
};
class DEFFEREDPIPE :public RenderPIPE {
	GLuint rbo;
	GLuint nextFBO;
	GLuint quadVAO;
	GLuint quadVBO;
	Shader* lightingShader;
public:
	GLuint gPosition,gNormal,gAlbedoSpec;//alpha value of albedo buffer is specular value.
	DEFFEREDPIPE(const glm::uvec2& screenRes, GLuint nextPIPEFBO = 0) :RenderPIPE(), nextFBO(nextPIPEFBO) {

		begin();

		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

		glGenTextures(1, &gNormal);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

		glGenTextures(1, &gAlbedoSpec);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

		unsigned int attachments[] = {GL_COLOR_ATTACHMENT0 ,GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
		glDrawBuffers(3, attachments);

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
		lightingShader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\postProcess.vs",
			"C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\deffered.fs");
	}

	void begin() {
		RenderPIPE::begin();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void end() {
		glBindFramebuffer(GL_FRAMEBUFFER, nextFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lightingShader->use();
		if (parameter_changed) {
			lightingShader->setFloat("shadowBlurJitter", shadowBlurJitter);
			lightingShader->setFloat("shadowBlurArea", shadowBlurArea);
		}
		lightingShader->setVec3("sunDir", glm::vec3(mainCam->getViewMat()*glm::vec4(sun->lightDir,0.0)));
		lightingShader->setVec3("lightColor", sun->color);
		lightingShader->setVec3("viewPos", mainCam->pos);
		lightingShader->setMat4("inverseViewMat", glm::inverse(mainCam->getViewMat()));
		lightingShader->setMat4("lightSpaceMat", sun->lightSpaceMat(landSize));;

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(lightingShader->ID, "gPosition"), 0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glUniform1i(glGetUniformLocation(lightingShader->ID, "gNormal"), 1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glUniform1i(glGetUniformLocation(lightingShader->ID, "gAlbedoSpec"), 2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glActiveTexture(GL_TEXTURE3);
		glUniform1i(glGetUniformLocation(lightingShader->ID, "texture_shadow"), 3);
		glBindTexture(GL_TEXTURE_2D, sun->depthMap);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
};

class SSRPIPE :public RenderPIPE {
	GLuint rbo;
	GLuint nextFBO;
	GLuint quadVAO;
	GLuint quadVBO;
	glm::uvec2 gBufferImages;
	Shader* ssrShader;
	float RES_X, RES_Y;
public:
	GLuint texture_color;
	SSRPIPE(const glm::uvec2& screenRes,GLuint nextPIPEFBO = 0) :RenderPIPE(), nextFBO(nextPIPEFBO) {
		
		begin();

		glGenTextures(1, &texture_color);
		glBindTexture(GL_TEXTURE_2D, texture_color);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color, 0);

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
		ssrShader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\postProcess.vs",
			"C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\ssr.fs");

		RES_X = screenRes.x;
		RES_Y = screenRes.y;
	}

	void gBufferBind(GLuint gPos, GLuint gNormal) {
		gBufferImages = glm::uvec2(gPos,gNormal);
	}

	void begin() {
		RenderPIPE::begin();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void end() {
		glBindFramebuffer(GL_FRAMEBUFFER, nextFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ssrShader->use();
		if (parameter_changed) {
			ssrShader->setFloat("RES_X", RES_X);
			ssrShader->setFloat("RES_Y", RES_Y);
			ssrShader->setFloat("maxDistance", ssr_maxDistance);
			ssrShader->setFloat("resolution", ssr_resolution);
			ssrShader->setFloat("thickness", ssr_thickness);
			ssrShader->setInt("steps", ssr_steps);
		}
		ssrShader->setVec3("camPos", mainCam->pos);
		ssrShader->setVec3("camFront", glm::vec3(mainCam->getViewMat()*glm::vec4(mainCam->front,0.0)));
		ssrShader->setMat4("Prjmat", mainCam->getPerspectiveMat());

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(ssrShader->ID, "image_position"), 0);
		glBindTexture(GL_TEXTURE_2D, gBufferImages[0]);
		glActiveTexture(GL_TEXTURE1);
		glUniform1i(glGetUniformLocation(ssrShader->ID, "image_normal"), 1);
		glBindTexture(GL_TEXTURE_2D, gBufferImages[1]);
		glActiveTexture(GL_TEXTURE2);
		glUniform1i(glGetUniformLocation(ssrShader->ID, "image_color"), 2);
		glBindTexture(GL_TEXTURE_2D, texture_color);

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
	DEFFEREDPIPE* defferedPIPE;
	SSRPIPE* ssrPIPE;
	HDRPIPE* hdrPIPE;
	glm::uvec2 screenRes;
	Renderer(const char* title,int width, int height);
	glm::vec4 clearColor;

	inline void startFrameRender(){
		defferedPIPE->begin();
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	}
	inline void postProcess() {
		defferedPIPE->end();
		ssrPIPE->end();
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