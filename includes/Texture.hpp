#ifndef _TEXTURE_H_
#define _TEXTURE_H_
/*
#include <memory.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gl/GL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Logger.h"

class Texture {
	GLuint texture;
	char path[128];
	bool isValid;
public:
	Texture(const char* img_path):isValid(false) {
		strcpy(path, img_path);
		texture = 0;
	}
	void genTexture() {
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// load and generate the texture
		int width, height, nrChannels;
		unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			logger.Log("Failed to load texture");
		}
		stbi_image_free(data);
		isValid = true;
	}
};
*/
#endif