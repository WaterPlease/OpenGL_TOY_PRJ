#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include "glm_pre.h"

extern int GROUND_TESS_LEVEL;
extern int GRASS_TESS_LEVEL;
extern int WATER_TESS_LEVEL;
extern float gamma;
extern bool parameter_changed;

extern float waterSize;
extern float waterLambda;
extern float waterLevel;
extern float uvFactorWater;
extern float uvFactorRock;
extern float uvFactorGrass;
extern float max_height;

extern float water_waveLength;
extern float water_steepness;
extern float grass_waveLength;
extern float grass_steepness;

extern float speed;
extern float rotSpeed;

extern int shadowFactor;
extern float shadowBlurJitter;
extern float shadowBlurArea;

extern unsigned int texture_skybox;
#endif