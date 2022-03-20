#include "Parameter.h"

int GROUND_TESS_LEVEL = 24;
int GRASS_TESS_LEVEL = 24;
bool parameter_changed = true;

glm::vec3 sunLightDir(0.0f,1.0f,0.0f);
float waterLambda = 0.1;
float waterLevel = 2.0f;
float uvFactorWater = 10.0f;
float uvFactorRock = 300.0;
float uvFactorGrass = 100.0;
float max_height = 15.0f;
float waveLength = 20.0f;
float steepness = 0.5f;

float speed = 0.01;
float rotSpeed = 0.25f;