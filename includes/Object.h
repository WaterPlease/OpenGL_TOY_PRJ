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


		shader->use();

		// view/projection transformations
		glm::mat4 projection = mainCam->getPerspectiveMat();
		glm::mat4 view = mainCam->getViewMat();
		shader->setMat4("projection", projection);
		shader->setMat4("view", view);
		if (parameter_changed) {
			glm::mat4 modelMat = glm::mat4(1.0f);
			modelMat = glm::rotate(modelMat, glm::radians(rot[0]), glm::vec3(rot[1], rot[2], rot[3]));
			modelMat = glm::scale(modelMat, glm::vec3(scale));
			shader->setMat4("model", modelMat);

			shader->setFloat("landSize", landSize);
			shader->setVec3("pos", pos);
			shader->setFloat("max_height", max_height);
		}
		
		glActiveTexture(GL_TEXTURE4);
		glUniform1i(glGetUniformLocation(shader->ID, "texture_height"), 4);
		glBindTexture(GL_TEXTURE_2D, gl_texture_heightmap);
		// render the loaded model

		model->Draw(*shader);
	}
	inline void shadowDraw() {
		if (model == nullptr || shader == nullptr) return;

		shadow_shader->use();

		// view/projection transformations
		glm::mat4 projection = mainCam->getPerspectiveMat();
		glm::mat4 view = mainCam->getViewMat();
		shadow_shader->setMat4("lightSpaceMat", sun->lightSpaceMat(landSize));
		if (parameter_changed) {
			// render the loaded model
			glm::mat4 modelMat = glm::mat4(1.0f);
			modelMat = glm::rotate(modelMat, glm::radians(rot[0]), glm::vec3(rot[1], rot[2], rot[3]));
			modelMat = glm::scale(modelMat, glm::vec3(scale));
			shadow_shader->setMat4("model", modelMat);

			shadow_shader->setFloat("landSize", landSize);
			shadow_shader->setVec3("pos", pos);
			shadow_shader->setFloat("max_height", max_height);
		}

		glActiveTexture(GL_TEXTURE4);
		glUniform1i(glGetUniformLocation(shader->ID, "texture_height"), 4);
		glBindTexture(GL_TEXTURE_2D, gl_texture_heightmap);

		model->Draw(*shadow_shader);
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
	GLfloat pos[32*32*5];
};

class FireFliesObject : public BaseObject {
	ComputeShader* fliesUpdate;
	Shader* flyShader;
	const int flyDensity=32;
public:
	GLuint VAO, VBO, SSBO_FLY;
	FlyInfo* flyInfo;
	FireFliesObject() :BaseObject(ObjClass::FireFlies){
		flyInfo = new FlyInfo();
		for (int i = 0; i < flyDensity * flyDensity; i++) {
			flyInfo->pos[5 * i + 0] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2.0f - 1.0f;
			flyInfo->pos[5 * i + 1] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2.0f - 1.0f;
			flyInfo->pos[5 * i + 2] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2.0f - 1.0f;
			flyInfo->pos[5 * i + 3] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2.0f - 1.0f;
			flyInfo->pos[5 * i + 4] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2.0f - 1.0f;

			for (int s = 0; s < 4; s++) {
				for (int t = 0; t < 4; t++) {
					//flyInfo->rotMat[16*i + s*4 + t] = (s==t)? 1.0f:0.0f;
				}
			}
		}

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		//float vertices[] = { -0.5,0.0,0.0,
		//					  0.5,0.0,0.0,
		//					  0.0,1.0,0.0};
		float vertices[] = { 0.0,0.0,0.0, };

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER,VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glBindVertexArray(0);

		glGenBuffers(1, &SSBO_FLY); 
		glBindBuffer(GL_SHADER_STORAGE_BUFFER,SSBO_FLY);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 32*32*5*sizeof(float), flyInfo, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, SSBO_FLY);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		gl_SSBO_FLY = SSBO_FLY;

		fliesUpdate = new ComputeShader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\flyUpdate.cs");
		flyShader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\flyShader.vs",
			"C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\flyShader.fs");
	}

	void draw() {
		fliesUpdate->use();
		fliesUpdate->setFloat("t", timeMng.getEffectiveTime());
		if (parameter_changed) {
			fliesUpdate->setFloat("landSize", landSize);
			fliesUpdate->setFloat("max_height", max_height);
			fliesUpdate->setFloat("waterLevel", waterLevel);
		}
		glActiveTexture(GL_TEXTURE4);
		glUniform1i(glGetUniformLocation(fliesUpdate->ID, "texture_height"), 4);
		glBindTexture(GL_TEXTURE_2D, gl_texture_heightmap);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_FLY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO_FLY);
		GLuint block_index0 = 0;
		block_index0 = glGetProgramResourceIndex(fliesUpdate->ID, GL_SHADER_STORAGE_BLOCK, "flyInfo");
		GLuint ssbo_binding_point_index = 3;
		glShaderStorageBlockBinding(fliesUpdate->ID, block_index0, ssbo_binding_point_index);
		if(drawFireflies)
			glDispatchCompute(32 , 32, 1);

		glMemoryBarrier(GL_ALL_BARRIER_BITS|GL_SHADER_STORAGE_BARRIER_BIT);

		flyShader->use();
		glm::mat4 projection = mainCam->getPerspectiveMat();
		glm::mat4 view = mainCam->getViewMat();
		flyShader->setMat4("projection", projection);
		flyShader->setMat4("view", view);
		if (parameter_changed) {
			flyShader->setFloat("landSize", landSize);
			flyShader->setFloat("max_height", max_height);
			flyShader->setFloat("waterLevel", waterLevel);
		}

		glActiveTexture(GL_TEXTURE4);
		glUniform1i(glGetUniformLocation(flyShader->ID, "texture_height"), 4);
		glBindTexture(GL_TEXTURE_2D, gl_texture_heightmap);

		glDisable(GL_CULL_FACE);
		glBindVertexArray(VAO);


		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_FLY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO_FLY);
		block_index0 = 0;
		block_index0 = glGetProgramResourceIndex(flyShader->ID, GL_SHADER_STORAGE_BLOCK, "flyInfo");
		ssbo_binding_point_index = 3;
		glShaderStorageBlockBinding(flyShader->ID, block_index0, ssbo_binding_point_index);

		glPointSize(3.0f);
		if(drawFireflies)
			glDrawArraysInstanced(GL_POINTS, 0, 1, 32 * 32);
		glBindVertexArray(0);
		glEnable(GL_CULL_FACE);
	}
};
class TerrainObject : public BaseObject {
	GLfloat patchSize;
	GLfloat grass_patchSize;
	GLfloat landSize;
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

	void LoadTexture() {
		textureAlbedoGrass = TextureFromFile("Ground037_2K_Color.png",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\terrain_texture"));
		textureNormalGrass = TextureFromFile("Ground037_2K_NormalGL.png",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\terrain_texture"));
		textureRoughnessGrass = TextureFromFile("Ground037_2K_Roughness.png",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\terrain_texture"));
		textureAoGrass = TextureFromFile("Ground037_2K_AmbientOcclusion.png",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\terrain_texture"));


		textureAlbedoRock = TextureFromFile("Ground049A_2K_Color.png",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\terrain_texture"));
		textureNormalRock = TextureFromFile("Ground049A_2K_NormalGL.png",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\terrain_texture"));
		textureRoughnessRock = TextureFromFile("Ground049A_2K_Roughness.png",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\terrain_texture"));
		textureAoRock = TextureFromFile("Ground049A_2K_AmbientOcclusion.png",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\terrain_texture"));

		textureHeight  = TextureFromFile_ForHeight("terrain_height.png",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\terrain_texture"));
		textureGrass = TextureFromFile_NEAREST("grass.png",
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\terrain_texture"));

		gl_texture_heightmap = textureHeight;

		std::vector<string> pathLst;
		for (int i = 1; i <= 120; i++) {
			std::string tmp = to_string(i);
			int zeroCount = (4 - (int)tmp.length());
			for (int j = 0; j < zeroCount; j++) tmp = to_string(0) + tmp;
			pathLst.push_back(tmp+string(".png"));
		}
		textureWaterNormal = Texture3DFromFile(pathLst,
			std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\water_normal_map"));
	}
public:
	GLuint VAO;
	GLuint grass_VAO;
	GLuint water_VAO;
	GLuint textureHeight;
	GLfloat grass_factor = 2.0f;
	TerrainObject(GLfloat _patchSize, GLfloat _landSize, Shader* _t_shader,Shader* _t_shadow_shader,Shader* _g_shader,Shader* _w_shader) :BaseObject(ObjClass::Terrain)
		,terrain_shader(_t_shader), terrain_shadow_shader(_t_shadow_shader),grass_shader(_g_shader),water_shader(_w_shader) {
		patchSize = _patchSize;
		landSize = _landSize;
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
		float grass_vertices[] = {
			-0.5,0.0,0.0,0.0,1.0,0.0,-0.707107,0.707107,
			0.5,0.0,0.0,1.0,1.0,0.0,-0.707107,0.707107,
			-0.5,0.707107,0.707107,0.0,0.0,0.0,-0.707107,0.707107,
			0.5,0.0,0.0,1.0,1.0,0.0,-0.707107,0.707107,
			0.5,0.707107,0.707107,1.0,0.0,0.0,-0.707107,0.707107,
			-0.5,0.707107,0.707107,0.0,0.0,0.0,-0.707107,0.707107,
			0.25,0.0,-0.433013,0.0,1.0,-0.612373,-0.707107,-0.353553,
			-0.25,0.0,0.433013,1.0,1.0,-0.612373,-0.707107,-0.353553,
			-0.362373,0.707107,-0.786566,0.0,0.0,-0.612373,-0.707107,-0.353553,
			-0.25,0.0,0.433013,1.0,1.0,-0.612373,-0.707107,-0.353553,
			-0.862372,0.707107,0.07946,1.0,0.0,-0.612373,-0.707107,-0.353553,
			-0.362373,0.707107,-0.786566,0.0,0.0,-0.612373,-0.707107,-0.353553,
			0.25,0.0,0.433012,0.0,1.0,0.612372,-0.707107,-0.353554,
			-0.25,0.0,-0.433012,1.0,1.0,0.612372,-0.707107,-0.353554,
			0.862372,0.707107,0.079459,0.0,0.0,0.612372,-0.707107,-0.353554,
			-0.25,0.0,-0.433012,1.0,1.0,0.612372,-0.707107,-0.353554,
			0.362372,0.707107,-0.786566,1.0,0.0,0.612372,-0.707107,-0.353554,
			0.862372,0.707107,0.079459,0.0,0.0,0.612372,-0.707107,-0.353554,
		};
			
			/*{
			-0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,0.0f,1.0f,
			0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,0.0f,1.0f,
			-0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,0.0f,1.0f,

			0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,0.0f,1.0f,
			0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f,1.0f,
			-0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,0.0f,1.0f,

			0.0f, 0.0f, -0.5f, 0.0f, 1.0f, 1.0f,0.0f,0.0f,
			0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f,0.0f,0.0f,
			0.0f, 1.0f, -0.5f, 0.0f, 0.0f, 1.0f,0.0f,0.0f,

			0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f,0.0f,0.0f,
			0.0f, 1.0f, 0.5f, 1.0f, 0.0f, 1.0f,0.0f,0.0f,
			0.0f, 1.0f, -0.5f, 0.0f, 0.0f, 1.0f,0.0f,0.0f,
			};*/
		std::cout << "Total # patch of grass : " << grass_numAxisPatch * grass_numAxisPatch << "\n";

		glGenVertexArrays(1, &grass_VAO);
		glGenBuffers(1, &grass_VBO);

		glBindVertexArray(grass_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, grass_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(grass_vertices), &grass_vertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		glBindVertexArray(0);

		// water patches
		float water_vertices[] = {
			-1.0 * 0.5f, 0.0f, -1.0 * 0.5f,
			-1.0 * 0.5f, 0.0f, +1.0 * 0.5f,
			+1.0 * 0.5f, 0.0f, +1.0 * 0.5f,
			+1.0 * 0.5f, 0.0f, -1.0 * 0.5f,};
		glGenVertexArrays(1, &water_VAO);
		glGenBuffers(1, &water_VBO);

		glBindVertexArray(water_VAO);
		glBindBuffer(GL_ARRAY_BUFFER,water_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(water_vertices), &water_vertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glBindVertexArray(0);

		LoadTexture();
	}
	void draw() {

		glm::mat4 projection = mainCam->getPerspectiveMat();
		glm::mat4 view = mainCam->getViewMat();
		glm::mat4 lightSpaceMat = sun->lightSpaceMat(landSize);
		
		terrain_shader->use();

		if (parameter_changed) {
			terrain_shader->setInt("tessLevel", GROUND_TESS_LEVEL);
			terrain_shader->setInt("numAxisPatches", numAxisPatch);
			terrain_shader->setFloat("patchSize", patchSize);
			terrain_shader->setFloat("landSize", landSize);
			terrain_shader->setFloat("max_height", max_height);
			terrain_shader->setFloat("cosHalfDiag", mainCam->cosHalfDiag);
			terrain_shader->setFloat("uvFactorRock", uvFactorRock);
			terrain_shader->setFloat("uvFactorGrass", uvFactorGrass);
			terrain_shader->setInt("shadowFactor", shadowFactor);
			terrain_shader->setFloat("grassCrit", grassCrit);
		}
		terrain_shader->setMat4("projection", projection);
		terrain_shader->setMat4("view", view);
		terrain_shader->setVec3("camPos", mainCam->pos);
		terrain_shader->setVec3("camFront", mainCam->front);
		terrain_shader->setVec3("lightDir", glm::normalize(sun->lightDir));
		terrain_shader->setMat4("lightSpaceMat", lightSpaceMat);

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_albedo_rock"), 0);
		glBindTexture(GL_TEXTURE_2D, textureAlbedoRock);

		glActiveTexture(GL_TEXTURE1);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_albedo_grass"), 1);
		glBindTexture(GL_TEXTURE_2D, textureAlbedoGrass);

		glActiveTexture(GL_TEXTURE2);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_normal_rock"), 2);
		glBindTexture(GL_TEXTURE_2D, textureNormalRock);

		glActiveTexture(GL_TEXTURE3);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_normal_grass"), 3);
		glBindTexture(GL_TEXTURE_2D, textureNormalGrass);

		glActiveTexture(GL_TEXTURE4);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_rough_rock"), 4);
		glBindTexture(GL_TEXTURE_2D, textureRoughnessRock);

		glActiveTexture(GL_TEXTURE5);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_rough_grass"), 5);
		glBindTexture(GL_TEXTURE_2D, textureRoughnessGrass);

		glActiveTexture(GL_TEXTURE6);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_ao_rock"), 6);
		glBindTexture(GL_TEXTURE_2D, textureAoRock);

		glActiveTexture(GL_TEXTURE7);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_ao_grass"), 7);
		glBindTexture(GL_TEXTURE_2D, textureAoGrass);
		
		glActiveTexture(GL_TEXTURE8);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_height"), 8);
		glBindTexture(GL_TEXTURE_2D, textureHeight);

		glActiveTexture(GL_TEXTURE9);
		glUniform1i(glGetUniformLocation(terrain_shader->ID, "texture_shadow"), 9);
		glBindTexture(GL_TEXTURE_2D, sun->depthMap);

		glBindVertexArray(VAO);
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawArraysInstanced(GL_PATCHES, 0, 4, numAxisPatch * numAxisPatch);
		glBindVertexArray(0);



		grass_shader->use();
		if (parameter_changed) {
			grass_shader->setInt("axisCount", GRASS_INST_LEVEL);
			grass_shader->setFloat("patchSize", grass_patchSize);
			grass_shader->setFloat("landSize", landSize);
			grass_shader->setFloat("grassSize", grassSize);
			grass_shader->setFloat("grassProb", grassProb);
			grass_shader->setFloat("grassCrit", grassCrit);
			grass_shader->setFloat("max_height", max_height);
			grass_shader->setFloat("cosHalfDiag", mainCam->cosHalfDiag);
			grass_shader->setFloat("waterLevel", waterLevel);
			grass_shader->setFloat("waveLength", grass_waveLength);
			grass_shader->setFloat("steepness", grass_steepness);
			grass_shader->setInt("shadowFactor", shadowFactor);
			grass_shader->setFloat("shadowBlurJitter", shadowBlurJitter);
			grass_shader->setFloat("shadowBlurArea", shadowBlurArea);
		}
		grass_shader->setMat4("projection", projection);
		grass_shader->setMat4("view", view);
		grass_shader->setVec3("camPos", mainCam->pos);
		grass_shader->setVec3("camFront", mainCam->front);
		grass_shader->setVec3("lightDir", glm::normalize(sun->lightDir));
		grass_shader->setMat4("lightSpaceMat", lightSpaceMat);
		grass_shader->setFloat("time", timeMng.getEffectiveTime());
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(grass_shader->ID, "texture_height"), 0);
		glBindTexture(GL_TEXTURE_2D, textureHeight);

		glActiveTexture(GL_TEXTURE1);
		glUniform1i(glGetUniformLocation(grass_shader->ID, "texture_shadow"), 1);
		glBindTexture(GL_TEXTURE_2D, sun->depthMap);

		glActiveTexture(GL_TEXTURE2);
		glUniform1i(glGetUniformLocation(grass_shader->ID, "texture_grass"), 2);
		glBindTexture(GL_TEXTURE_2D, textureGrass);


		glDisable(GL_CULL_FACE);

		glBindVertexArray(grass_VAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 18, GRASS_INST_LEVEL * GRASS_INST_LEVEL);
		glBindVertexArray(0);

		glEnable(GL_CULL_FACE);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		water_shader->use();
		if (parameter_changed) {
			water_shader->setInt("tessLevel", WATER_TESS_LEVEL);
			water_shader->setFloat("waterLevel", waterLevel);
			water_shader->setFloat("uvFactorWater", uvFactorWater);
			water_shader->setFloat("waterLevel", waterLevel);
			water_shader->setFloat("cosHalfDiag", mainCam->cosHalfDiag);
			water_shader->setFloat("landSize", landSize*waterSize);
			water_shader->setFloat("waveLength", water_waveLength);
			water_shader->setFloat("steepness", water_steepness);
			water_shader->setInt("shadowFactor", shadowFactor);
			water_shader->setFloat("transparency", water_transparency);
			water_shader->setFloat("res_x", screen_x);
			water_shader->setFloat("res_y", screen_y);
		}
		water_shader->setMat4("projection", projection);
		water_shader->setMat4("view", view);
		water_shader->setVec3("camPos", mainCam->pos);
		water_shader->setVec3("camFront", mainCam->front);
		water_shader->setVec3("lightDir", glm::normalize(sun->lightDir));
		water_shader->setFloat("waterTimeFactor", waterLambda);
		water_shader->setMat4("lightSpaceMat", lightSpaceMat);
		water_shader->setFloat("time", timeMng.getEffectiveTime());
		water_shader->setMat4("invViewMat", mainCam->getInvViewMat());
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(water_shader->ID, "texture_normal"), 0);
		glBindTexture(GL_TEXTURE_3D, textureWaterNormal);

		glActiveTexture(GL_TEXTURE1);
		glUniform1i(glGetUniformLocation(water_shader->ID, "texture_skybox"), 1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_skybox);

		glActiveTexture(GL_TEXTURE2);
		glUniform1i(glGetUniformLocation(water_shader->ID, "texture_shadow"), 2);
		glBindTexture(GL_TEXTURE_2D, sun->depthMap);

		glActiveTexture(GL_TEXTURE3);
		glUniform1i(glGetUniformLocation(water_shader->ID, "texture_position"), 3);
		glBindTexture(GL_TEXTURE_2D, gBuffer_position);

		glActiveTexture(GL_TEXTURE4);
		glUniform1i(glGetUniformLocation(water_shader->ID, "texture_color"), 4);
		glBindTexture(GL_TEXTURE_2D, gBuffer_Albedo);

		glBindVertexArray(water_VAO);
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawArrays(GL_PATCHES, 0, 4);
		glBindVertexArray(0);
		glDisable(GL_BLEND);
		
	}
	void shadowDraw() {
		terrain_shadow_shader->use();

		terrain_shadow_shader->setInt("tessLevel", 8);//
		terrain_shadow_shader->setInt("numAxisPatches", numAxisPatch);//
		terrain_shadow_shader->setFloat("patchSize", patchSize);//
		terrain_shadow_shader->setFloat("landSize", landSize);//
		terrain_shadow_shader->setFloat("max_height", max_height);//
		terrain_shadow_shader->setMat4("lightSpaceMat", sun->lightSpaceMat(landSize));;

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(terrain_shadow_shader->ID, "texture_height"), 0);
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