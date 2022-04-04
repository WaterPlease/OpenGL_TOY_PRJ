#include "Object.h"

void SimpleObject::draw() {
	if (model == nullptr || shader == nullptr) return;


	shader->use();

	// view/projection transformations
	glm::mat4 projection = mainCam->GetPerspectiveMat();
	glm::mat4 view = mainCam->GetViewMat();
	shader->setMat4("projection", projection);
	shader->setMat4("view", view);
	if (bPrameterChange) {
		glm::mat4 modelMat = glm::mat4(1.0f);
		modelMat = glm::rotate(modelMat, glm::radians(rot[0]), glm::vec3(rot[1], rot[2], rot[3]));
		modelMat = glm::scale(modelMat, glm::vec3(scale));
		shader->setMat4("model", modelMat);

		shader->setFloat("landSize", LANDSIZE);
		shader->setVec3("pos", pos);
		shader->setFloat("max_height", LAND_HEIGHT);
	}

	glActiveTexture(GL_TEXTURE4);
	glUniform1i(glGetUniformLocation(shader->ID, "texture_height"), 4);
	glBindTexture(GL_TEXTURE_2D, gl_texture_heightmap);
	// render the loaded model

	model->Draw(*shader);
}

void SimpleObject::shadowDraw() {
	if (model == nullptr || shader == nullptr) return;

	shadow_shader->use();

	// view/projection transformations
	glm::mat4 projection = mainCam->GetPerspectiveMat();
	glm::mat4 view = mainCam->GetViewMat();
	shadow_shader->setMat4("lightSpaceMat", sun->lightSpaceMat(LANDSIZE));
	if (bPrameterChange) {
		// render the loaded model
		glm::mat4 modelMat = glm::mat4(1.0f);
		modelMat = glm::rotate(modelMat, glm::radians(rot[0]), glm::vec3(rot[1], rot[2], rot[3]));
		modelMat = glm::scale(modelMat, glm::vec3(scale));
		shadow_shader->setMat4("model", modelMat);

		shadow_shader->setFloat("landSize", LANDSIZE);
		shadow_shader->setVec3("pos", pos);
		shadow_shader->setFloat("max_height", LAND_HEIGHT);
	}

	glActiveTexture(GL_TEXTURE4);
	glUniform1i(glGetUniformLocation(shader->ID, "texture_height"), 4);
	glBindTexture(GL_TEXTURE_2D, gl_texture_heightmap);

	model->Draw(*shadow_shader);
}

FireFliesObject::FireFliesObject() :BaseObject(ObjClass::FireFlies) {
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
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);

	glGenBuffers(1, &SSBO_FLY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_FLY);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 32 * 32 * 5 * sizeof(float), flyInfo, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, SSBO_FLY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	gl_SSBO_FLY = SSBO_FLY;

	fliesUpdate = new ComputeShader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\flyUpdate.cs");
	flyShader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\flyShader.vs",
		"C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\flyShader.fs");
}

void FireFliesObject::draw() {
	fliesUpdate->use();
	fliesUpdate->setFloat("t", timeMng.getEffectiveTime());
	if (bPrameterChange) {
		fliesUpdate->setFloat("landSize", LANDSIZE);
		fliesUpdate->setFloat("max_height", LAND_HEIGHT);
		fliesUpdate->setFloat("waterLevel", WATER_LEVEL);
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
	if (bDrawFireflies)
		glDispatchCompute(32, 32, 1);

	glMemoryBarrier(GL_ALL_BARRIER_BITS | GL_SHADER_STORAGE_BARRIER_BIT);

	flyShader->use();
	glm::mat4 projection = mainCam->GetPerspectiveMat();
	glm::mat4 view = mainCam->GetViewMat();
	flyShader->setMat4("projection", projection);
	flyShader->setMat4("view", view);
	if (bPrameterChange) {
		flyShader->setFloat("landSize", LANDSIZE);
		flyShader->setFloat("max_height", LAND_HEIGHT);
		flyShader->setFloat("waterLevel", WATER_LEVEL);
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
	if (bDrawFireflies)
		glDrawArraysInstanced(GL_POINTS, 0, 1, 32 * 32);
	glBindVertexArray(0);
	glEnable(GL_CULL_FACE);
}

void TerrainObject::LoadTexture() {
	textureAlbedoGrass = TextureFromFile("Ground037_2K_Color.png",
		pathMng.getAbsPath("model\\terrain_texture"));
	textureNormalGrass = TextureFromFile("Ground037_2K_NormalGL.png",
		pathMng.getAbsPath("model\\terrain_texture"));
	textureRoughnessGrass = TextureFromFile("Ground037_2K_Roughness.png",
		pathMng.getAbsPath("model\\terrain_texture"));
	textureAoGrass = TextureFromFile("Ground037_2K_AmbientOcclusion.png",
		pathMng.getAbsPath("model\\terrain_texture"));

	/*
	textureAlbedoRock = TextureFromFile("Ground049A_2K_Color.png",
		std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\terrain_texture"));
	*/
	textureAlbedoRock = TextureFromFile("Ground049A_2K_Color.png",
		pathMng.getAbsPath("model\\terrain_texture"));
	textureNormalRock = TextureFromFile("Ground049A_2K_NormalGL.png",
		pathMng.getAbsPath("model\\terrain_texture"));
	textureRoughnessRock = TextureFromFile("Ground049A_2K_Roughness.png",
		pathMng.getAbsPath("model\\terrain_texture"));
	textureAoRock = TextureFromFile("Ground049A_2K_AmbientOcclusion.png",
		pathMng.getAbsPath("model\\terrain_texture"));

	textureHeight = TextureFromFile_ForHeight("terrain_height.png",
		pathMng.getAbsPath("model\\terrain_texture"));
	textureGrass = TextureFromFile_NEAREST("grass.png",
		pathMng.getAbsPath("model\\terrain_texture"));

	gl_texture_heightmap = textureHeight;

	std::vector<string> pathLst;
	for (int i = 1; i <= 120; i++) {
		std::string tmp = to_string(i);
		int zeroCount = (4 - (int)tmp.length());
		for (int j = 0; j < zeroCount; j++) tmp = to_string(0) + tmp;
		pathLst.push_back(tmp + string(".png"));
	}
	textureWaterNormal = Texture3DFromFile(pathLst,
		pathMng.getAbsPath("model\\water_normal_map"));
}

TerrainObject::TerrainObject(GLfloat _patchSize, Shader* _t_shader, Shader* _t_shadow_shader, Shader* _g_shader, Shader* _w_shader) :BaseObject(ObjClass::Terrain)
, terrain_shader(_t_shader), terrain_shadow_shader(_t_shadow_shader), grass_shader(_g_shader), water_shader(_w_shader) {
	patchSize = _patchSize;
	float vertices[] = { -patchSize,0.0f,-patchSize,
		-patchSize,0.0f,+patchSize,
		+patchSize,0.0f,+patchSize,
		+patchSize,0.0f,-patchSize, };
	numAxisPatch = static_cast<int>(LANDSIZE / (patchSize * 2.0f));
	std::cout << "Total # patch : " << numAxisPatch * numAxisPatch << "\n";

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
		+1.0 * 0.5f, 0.0f, -1.0 * 0.5f, };
	glGenVertexArrays(1, &water_VAO);
	glGenBuffers(1, &water_VBO);

	glBindVertexArray(water_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, water_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(water_vertices), &water_vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);

	LoadTexture();
}

void TerrainObject::draw() {

	glm::mat4 projection = mainCam->GetPerspectiveMat();
	glm::mat4 view = mainCam->GetViewMat();
	glm::mat4 lightSpaceMat = sun->lightSpaceMat(LANDSIZE);

	terrain_shader->use();

	if (bPrameterChange) {
		terrain_shader->setInt("tessLevel", TESS_GROUND_LEVEL);
		terrain_shader->setInt("numAxisPatches", numAxisPatch);
		terrain_shader->setFloat("patchSize", patchSize);
		terrain_shader->setFloat("landSize", LANDSIZE);
		terrain_shader->setFloat("max_height", LAND_HEIGHT);
		terrain_shader->setFloat("cosHalfDiag", mainCam->cosHalfDiag);
		terrain_shader->setFloat("uvFactorRock", LAND_ROCK_UV_FACTOR);
		terrain_shader->setFloat("uvFactorGrass", LAND_GRASS_UV_FACTOR);
		terrain_shader->setFloat("grassCrit", GRASS_SLOPE_CRITERION);
	}
	terrain_shader->setMat4("projection", projection);
	terrain_shader->setMat4("view", view);
	terrain_shader->setVec3("camPos", mainCam->pos);
	terrain_shader->setVec3("camFront", mainCam->front);

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

	glBindVertexArray(VAO);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawArraysInstanced(GL_PATCHES, 0, 4, numAxisPatch * numAxisPatch);
	glBindVertexArray(0);



	grass_shader->use();
	if (bPrameterChange) {
		grass_shader->setInt("axisCount", INSTANCE_GRASS_LEVEL);
		grass_shader->setFloat("landSize", LANDSIZE);
		grass_shader->setFloat("grassSize", GRASS_SIZE);
		grass_shader->setFloat("grassProb", GRASS_DENSITY);
		grass_shader->setFloat("grassCrit", GRASS_SLOPE_CRITERION);
		grass_shader->setFloat("max_height", LAND_HEIGHT);
		grass_shader->setFloat("cosHalfDiag", mainCam->cosHalfDiag);
		grass_shader->setFloat("waterLevel", WATER_LEVEL);
		grass_shader->setFloat("waveLength", GRASS_WAVE_LENGTH);
		grass_shader->setFloat("steepness", GRASS_WAVE_STEEPNESS);
	}
	grass_shader->setMat4("projection", projection);
	grass_shader->setMat4("view", view);
	grass_shader->setVec3("camPos", mainCam->pos);
	grass_shader->setVec3("camFront", mainCam->front);
	grass_shader->setFloat("time", timeMng.getEffectiveTime());
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(grass_shader->ID, "texture_height"), 0);
	glBindTexture(GL_TEXTURE_2D, textureHeight);

	glActiveTexture(GL_TEXTURE2);
	glUniform1i(glGetUniformLocation(grass_shader->ID, "texture_grass"), 2);
	glBindTexture(GL_TEXTURE_2D, textureGrass);


	glDisable(GL_CULL_FACE);

	glBindVertexArray(grass_VAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 18, INSTANCE_GRASS_LEVEL * INSTANCE_GRASS_LEVEL);
	glBindVertexArray(0);

	glEnable(GL_CULL_FACE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	water_shader->use();
	if (bPrameterChange) {
		water_shader->setInt("tessLevel", TESS_WATER_LEVEL);
		water_shader->setFloat("waterLevel", WATER_LEVEL);
		water_shader->setFloat("uvFactorWater", WATER_UV_FACTOR);
		water_shader->setFloat("waterTimeFactor", WATER_TIME_FACTOR);
		water_shader->setFloat("landSize", LANDSIZE * WATER_SIZE);
		water_shader->setFloat("waveLength", WATER_WAVE_LENGTH);
		water_shader->setFloat("steepness", WATER_WAVE_STEEPENSS);
		water_shader->setFloat("transparency", WATER_TRANSPARENCY);
		water_shader->setFloat("res_x", SCREEN_WIDTH);
		water_shader->setFloat("res_y", SCREEN_HEIGHT);
	}
	water_shader->setMat4("projection", projection);
	water_shader->setMat4("view", view);
	water_shader->setMat4("invViewMat", mainCam->GetInvViewMat());
	water_shader->setFloat("time", timeMng.getEffectiveTime());
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(water_shader->ID, "texture_normal"), 0);
	glBindTexture(GL_TEXTURE_3D, textureWaterNormal);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(glGetUniformLocation(water_shader->ID, "texture_position"), 1);
	glBindTexture(GL_TEXTURE_2D, gBuffer_position);

	glActiveTexture(GL_TEXTURE2);
	glUniform1i(glGetUniformLocation(water_shader->ID, "texture_color"), 2);
	glBindTexture(GL_TEXTURE_2D, gBuffer_Albedo);

	glBindVertexArray(water_VAO);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawArrays(GL_PATCHES, 0, 4);
	glBindVertexArray(0);
	glDisable(GL_BLEND);

}

void TerrainObject::shadowDraw() {
	terrain_shadow_shader->use();

	terrain_shadow_shader->setInt("tessLevel", 8);//
	terrain_shadow_shader->setInt("numAxisPatches", numAxisPatch);//
	terrain_shadow_shader->setFloat("patchSize", patchSize);//
	terrain_shadow_shader->setFloat("landSize", LANDSIZE);//
	terrain_shadow_shader->setFloat("max_height", LAND_HEIGHT);//
	terrain_shadow_shader->setMat4("lightSpaceMat", sun->lightSpaceMat(LANDSIZE));;

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(terrain_shadow_shader->ID, "texture_height"), 0);
	glBindTexture(GL_TEXTURE_2D, textureHeight);

	glBindVertexArray(VAO);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawArraysInstanced(GL_PATCHES, 0, 4, numAxisPatch * numAxisPatch);
	glBindVertexArray(0);
}

