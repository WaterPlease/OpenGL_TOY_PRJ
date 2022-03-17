#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "Component.h"
#include <map>
#include <string>
#include "Model.hpp"
#include "Camera.h"
#include "Parameter.h"

enum ObjClass {
	Base,
	Simple,
	Terrain,
	Cloud,
};

class BaseObject {
	ObjClass classID = ObjClass::Base;
public:
	BaseObject(ObjClass cID):classID(cID) { }
	virtual void update() { return; } // do nothing
	virtual void draw() { return; }   // do nothing
	inline ObjClass getClassID() { return classID; }
};

class SimpleObject:public BaseObject{
	std::map<std::string,Component*> logicComp;
	std::map<std::string,Component*> dataComp;
	Model* model;
	Shader* shader;
public:
	SimpleObject():model(nullptr),shader(nullptr), BaseObject(ObjClass::Simple) { }
	void update() {
		for (auto iter = logicComp.begin(); iter != logicComp.end(); iter++) {
			iter->second->OnUpdate();
		}
	}
	inline void addComp(Component* comp, bool isLogic) {
		if (isLogic) logicComp[comp->getName()] = comp;
		else dataComp[comp->getName()] = comp;
	}
	inline void draw() {
		if (model == nullptr || shader == nullptr) return;
		model->Draw(*shader);
	}
	inline Model* getModel() {
		return model;
	}
	inline void setModel(Model* _model) {
		model = _model;
	}
	inline Shader* getShader() {
		return shader;
	}
	inline void setShader(Shader* _shader) {
		shader = _shader;
	}
};

class TerrainObject : public BaseObject {
	GLfloat patchSize;
	GLfloat grass_patchSize;
	int numAxisPatch;
	int grass_numAxisPatch;
	//float vertices[12];

	GLuint VBO,grass_VBO;// , EBO;
	GLuint textureDiffuseRock;
	GLuint textureDiffuseGrass;
	GLuint textureNormalRock;
	GLuint textureNormalGrass;
	GLuint textureSpecRock;
	GLuint textureSpecGrass;
	GLuint textureAoRock;
	GLuint textureAoGrass;
	GLuint textureHeight;

	Shader* terrain_shader;
	Shader* grass_shader;

	void LoadTexture() {
		textureDiffuseGrass = TextureFromFile("Ground_Grass_001_COLOR.jpg",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model"));
		textureDiffuseRock = TextureFromFile("Rock_039_baseColor.jpg",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model"));
		textureNormalGrass = TextureFromFile("Ground_Grass_001_NORM.jpg",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model"));
		textureNormalRock = TextureFromFile("Rock_039_normal.jpg",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model"));
		textureSpecGrass = TextureFromFile("Ground_Grass_001_ROUGH.jpg",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model"));
		textureSpecRock = TextureFromFile("Rock_039_roughness.jpg",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model"));
		textureAoGrass = TextureFromFile("Ground_Grass_001_OCC.jpg",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model"));
		textureAoRock = TextureFromFile("Rock_039_ambientOcclusion.jpg",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model"));
		textureHeight  = TextureFromFile_ForHeight("_terrain_height.png",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model"));
	}
public:
	// temporary. THIS field must be replaced by some other object, like renederer or something else.
	glm::vec3 lightDir;

	GLuint VAO;
	GLuint grass_VAO;
	GLfloat grass_factor = 1.5f;
	GLfloat max_height = 15.0;
	TerrainObject(GLfloat _patchSize, GLfloat landSize, Shader* _t_shader,Shader* _g_shader) :BaseObject(ObjClass::Terrain)
		,terrain_shader(_t_shader), grass_shader(_g_shader) {
		patchSize = _patchSize;
		float vertices[] = { - patchSize,0.0f,- patchSize,
							 -patchSize,0.0f,+patchSize,
							 +patchSize,0.0f,+patchSize,
							 + patchSize,0.0f,- patchSize, };
		numAxisPatch = static_cast<int>(landSize / (patchSize*2.0f));
		std::cout << "Total # patch : " << numAxisPatch * numAxisPatch << "\n";
		//memcpy(vertices, _vertices, sizeof(vertices));
		
		//GLuint indices[] = { 0,1,3, 1,2,3};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glBindVertexArray(0);

		// grass patches
		grass_patchSize = patchSize * grass_factor;
		float grass_vertices[] = { -grass_patchSize,0.0f,-grass_patchSize,
							 -grass_patchSize,0.0f,+grass_patchSize,
							 +grass_patchSize,0.0f,+grass_patchSize,
							 +grass_patchSize,0.0f,-grass_patchSize, };
		grass_numAxisPatch = static_cast<int>(landSize / (grass_patchSize*2.0f));
		std::cout << "Total # patch of grass : " << grass_numAxisPatch * grass_numAxisPatch << "\n";

		glGenVertexArrays(1, &grass_VAO);
		glGenBuffers(1, &grass_VBO);

		glBindVertexArray(grass_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, grass_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(grass_vertices), &grass_vertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glBindVertexArray(0);



		LoadTexture();
	}
	void draw() {
		glm::mat4 projection = mainCam->getPerspectiveMat();
		glm::mat4 view = mainCam->getViewMat();

		grass_shader->use();
		if (parameter_changed) {
			grass_shader->setInt("tessLevel", TESS_LEVEL + 2);
			grass_shader->setInt("numAxisPatches", grass_numAxisPatch);
			grass_shader->setFloat("patchSize", grass_patchSize);
			grass_shader->setFloat("grassWidth", 0.05f);
			grass_shader->setFloat("grassLean", 0.8f);
			grass_shader->setFloat("grassHeight", 0.25f);
			grass_shader->setFloat("max_height", max_height);
			grass_shader->setFloat("cosHalfDiag", mainCam->cosHalfDiag);
		}
		grass_shader->setMat4("projection", projection);
		grass_shader->setMat4("view", view);
		grass_shader->setVec3("camPos", mainCam->pos);
		grass_shader->setVec3("camFront", mainCam->front);
		grass_shader->setVec3("lightDir", lightDir);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(grass_shader->ID, "texture_height"), 0);
		glBindTexture(GL_TEXTURE_2D, textureHeight);

		glDisable(GL_CULL_FACE);

		glBindVertexArray(grass_VAO);
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawArraysInstanced(GL_PATCHES, 0, 4, grass_numAxisPatch* grass_numAxisPatch);
		glBindVertexArray(0);

		glEnable(GL_CULL_FACE);


		terrain_shader->use();

		if (parameter_changed) {
			terrain_shader->setInt("tessLevel", TESS_LEVEL);
			terrain_shader->setInt("numAxisPatches", numAxisPatch);
			terrain_shader->setFloat("patchSize", patchSize);
			terrain_shader->setFloat("max_height", max_height);
			terrain_shader->setFloat("Kd", 0.5f);
			terrain_shader->setFloat("Ks", 0.5f);
			terrain_shader->setFloat("Ns", 20.0f);
			terrain_shader->setFloat("cosHalfDiag", mainCam->cosHalfDiag);
		}
		terrain_shader->setMat4("projection", projection);
		terrain_shader->setMat4("view", view);
		terrain_shader->setVec3("camPos", mainCam->pos);
		terrain_shader->setVec3("camFront", mainCam->front);
		terrain_shader->setVec3("lightDir", lightDir);

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_diffuse_rock"), 0);
		glBindTexture(GL_TEXTURE_2D, textureDiffuseRock);

		glActiveTexture(GL_TEXTURE1);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_diffuse_grass"), 1);
		glBindTexture(GL_TEXTURE_2D, textureDiffuseGrass);

		glActiveTexture(GL_TEXTURE2);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_normal_rock"), 2);
		glBindTexture(GL_TEXTURE_2D, textureNormalRock);

		glActiveTexture(GL_TEXTURE3);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_normal_grass"), 3);
		glBindTexture(GL_TEXTURE_2D, textureNormalGrass);

		glActiveTexture(GL_TEXTURE4);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_spec_rock"), 4);
		glBindTexture(GL_TEXTURE_2D, textureSpecRock);

		glActiveTexture(GL_TEXTURE5);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_spec_grass"), 5);
		glBindTexture(GL_TEXTURE_2D, textureSpecGrass);

		glActiveTexture(GL_TEXTURE6);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_ao_rock"), 6);
		glBindTexture(GL_TEXTURE_2D, textureAoRock);

		glActiveTexture(GL_TEXTURE7);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_ao_grass"), 7);
		glBindTexture(GL_TEXTURE_2D, textureAoGrass);
		
		glActiveTexture(GL_TEXTURE8);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_height"), 8);
		glBindTexture(GL_TEXTURE_2D, textureHeight);

		glBindVertexArray(VAO);
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawArraysInstanced(GL_PATCHES, 0, 4, numAxisPatch * numAxisPatch);
		glBindVertexArray(0);
	}
	
	inline void parameter_update() { parameter_changed = true; }
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
		glm::mat4 projection = mainCam->getPerspectiveMat();
		glm::mat4 view = mainCam->getViewMat();
		shader->setMat4("projection", projection);
		shader->setMat4("view", view);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
	}
};
#endif