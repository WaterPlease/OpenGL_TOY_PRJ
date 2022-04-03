#include "Camera.h"

Sun::Sun(const glm::vec3& _lDir, GLuint _resolution, GLuint _scrX, GLuint _scrY) :lightDir(_lDir), shadow_resolution(_resolution), scrX(_scrX), scrY(_scrY) {
	glGenFramebuffers(1, &depthFBO);
	glGenTextures(1, &depthMap);

	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, shadow_resolution, shadow_resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	color = glm::vec3(1.0);
}
void Sun::Begin() {
	time = timeMng.getEffectiveTime() * 0.3;
	glm::vec3 lightPos;
	lightPos.x = sin(time) * 3.0f;
	lightPos.z = cos(time) * 2.0f;
	lightPos.y = 1.5f + cos(time) * 1.0f;
	lightDir = glm::normalize(lightPos);
	//float highnoon = clamp(lightDir.y, 0.0f, 1 / 10.0f) * 10.0f;
	//color = glm::mix(glm::vec3(1.0f), glm::vec3(0.99f, 0.80, 0.44f), highnoon);
	//time = 1.750;
	glViewport(0, 0, shadow_resolution, shadow_resolution);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
}

inline void Sun::End() {
	//glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, scrX, scrY);
}


Camera* mainCam;
Sun* sun;