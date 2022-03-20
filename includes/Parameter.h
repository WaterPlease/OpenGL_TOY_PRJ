#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include "glm_pre.h"

extern int GROUND_TESS_LEVEL;
extern int GRASS_TESS_LEVEL;
extern bool parameter_changed;

extern glm::vec3 sunLightDir;
extern float waterLambda;
extern float waterLevel;
extern float uvFactorWater;
extern float uvFactorRock;
extern float uvFactorGrass;
extern float max_height;

extern float waveLength;
extern float steepness;

extern float speed;
extern float rotSpeed;
#endif