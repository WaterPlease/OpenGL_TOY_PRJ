#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "Component.h"
#include <map>
#include <string>
#include "Model.hpp"
#include "Camera.h"
#include "Parameter.h"
#include "util.h"
#include "PathMng.h"

#define FLYDENSITY 128

enum ObjClass {
	Base,
	Simple,
	Terrain,
	Cloud,
	FireFlies,
};

typedef  struct {
	GLuint  count;
	GLuint  instanceCount;
	GLuint  first;
	GLuint  baseInstance;
} DrawArraysIndirectCommand;

class BaseObject {
	ObjClass classID = ObjClass::Base;
public:
	BaseObject(ObjClass cID):classID(cID) { }
	virtual void update() { return; } // do nothing
	virtual void draw() { return; }   // do nothing
	virtual void shadowDraw() { return; }   // do nothing
	inline ObjClass getClassID() { return classID; }
};

class SimpleObject:public BaseObject{
	std::map<std::string,Component*> logicComp;
	std::map<std::string,Component*> dataComp;
	Model* model;
	Shader* shader;
	Shader* shadow_shader;

	glm::vec4 rot = glm::vec4(0.0f,0.0f,1.0f,0.0f);
public:
	bool make_shadow = false;
	glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
	float scale = 1.0;
	const std::string name;
	SimpleObject(std::string _name):name(_name), model(nullptr), shader(nullptr), BaseObject(ObjClass::Simple) { }
	inline void update() {
		for (auto iter = logicComp.begin(); iter != logicComp.end(); iter++) {
			iter->second->OnUpdate();
		}
	}
	inline void addComp(Component* comp, bool isLogic) {
		if (isLogic) logicComp[comp->getName()] = comp;
		else dataComp[comp->getName()] = comp;
	}
	void draw();
	void shadowDraw();
	inline Model* getModel() {
		return model;
	}
	inline void setModel(Model* _model) {
		model = _model;
	}
	inline Shader* getShader() {
		return shader;
	}
	inline Shader* getShadowShader() {
		return shadow_shader;
	}
	inline void setShader(Shader* _shader) {
		shader = _shader;
	}
	inline void setShadowShader(Shader* _shader) {
		shadow_shader = _shader;
	}
};

struct FlyInfo {
	GLfloat pos[FLYDENSITY * FLYDENSITY *5];
};

class FireFliesObject : public BaseObject {
	ComputeShader* fliesUpdate;
	Shader* flyShader;
	const int flyDensity= FLYDENSITY;
public:
	GLuint VAO, VBO, SSBO_FLY;
	FlyInfo* flyInfo;
	FireFliesObject();

	void draw();
};

class TerrainObject : public BaseObject {
	GLfloat patchSize;
	int numAxisPatch;
	int grass_numAxisPatch;

	GLuint VBO,grass_VBO,water_VBO;

	GLuint textureAlbedoRock;
	GLuint textureNormalRock;
	GLuint textureRoughnessRock;
	GLuint textureAoRock;

	GLuint textureAlbedoGrass;
	GLuint textureNormalGrass;
	GLuint textureRoughnessGrass;
	GLuint textureAoGrass;

	GLuint textureWaterNormal;
	GLuint textureGrass;

	Shader* terrain_shader;
	Shader* terrain_shadow_shader;
	Shader* grass_shader;
	Shader* water_shader;

	void LoadTexture();
public:
	GLuint VAO;
	GLuint grass_VAO;
	GLuint water_VAO;
	GLuint textureHeight;
	GLfloat grass_factor = 2.0f;
	TerrainObject(GLfloat _patchSize, Shader* _t_shader, Shader* _t_shadow_shader, Shader* _g_shader, Shader* _w_shader);
	void draw();
	void shadowDraw();
	
	inline void parameter_update() { bPrameterChange = true; }
};

// Later...
class CloudObject : public BaseObject {
	GLfloat seedX;
	GLfloat seedY;
	GLfloat seedZ;
	GLfloat cellHalfSize;
	
	GLuint VBO,EBO;
	Shader* shader;
public:
	GLuint VAO;
	CloudObject(GLfloat X, GLfloat Y, GLfloat Z,GLfloat _cellHalfSize):BaseObject(ObjClass::Cloud) {
		cellHalfSize = _cellHalfSize;
		float vertices[] = { X - cellHalfSize,Y - cellHalfSize,Z - cellHalfSize,
							 X + cellHalfSize,Y - cellHalfSize,Z - cellHalfSize,
							 X + cellHalfSize,Y - cellHalfSize,Z + cellHalfSize,
							 X - cellHalfSize,Y - cellHalfSize,Z + cellHalfSize,
							 X - cellHalfSize,Y + cellHalfSize,Z - cellHalfSize,
							 X + cellHalfSize,Y + cellHalfSize,Z - cellHalfSize,
							 X + cellHalfSize,Y + cellHalfSize,Z + cellHalfSize,
							 X - cellHalfSize,Y + cellHalfSize,Z + cellHalfSize};
		GLuint indices[] = { 0,3,1, 1,3,2,
							4,5,6 ,6,7,4,
							4,0,5, 5,0,1,
							5,1,2, 2,6,5,
							7,6,2, 2,3,7,
							4,7,3, 3,0,4};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glBindVertexArray(0);

		shader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain.vs",
			"C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain.fs", nullptr, nullptr);
	}
	void draw() {
		shader->use();
		glm::mat4 projection = mainCam->GetPerspectiveMat();
		glm::mat4 view = mainCam->GetViewMat();
		shader->setMat4("projection", projection);
		shader->setMat4("view", view);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
	}
};
#endif