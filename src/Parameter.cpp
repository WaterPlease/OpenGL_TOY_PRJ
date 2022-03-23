#include "Parameter.h"

int GROUND_TESS_LEVEL = 24;
int GRASS_INST_LEVEL = 256;
int WATER_TESS_LEVEL = 32;
float gamma = 2.2f;
bool parameter_changed = true;

glm::vec3 sunLightDir(0.0f,1.0f,0.0f);

float landSize = 100.0f;
float waterSize = 1.0f;
float waterLambda = 0.1;
float waterLevel = 2.0f;
float uvFactorWater = 10.0f;
float uvFactorRock = 300.0;
float uvFactorGrass = 100.0;
float max_height = 15.0f;
float water_waveLength = 20.0f;
float water_steepness = 0.5f;
float grass_waveLength = 10.5f;
float grass_steepness = 0.02f;
float grassProb = 0.5;
float grassSize = 0.5f;
float grassCrit = 0.995f;

float speed = 10;
float rotSpeed = 0.25f;
float tSpeed = 1.0f;

int shadowFactor = 1;
float shadowBlurJitter = 1.256;
float shadowBlurArea = 2.6232f;

unsigned int texture_skybox;