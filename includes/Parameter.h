#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include "glm_pre.h"

extern bool parameter_changed;

extern bool renderUI;

extern int screen_x;
extern int screen_y;

extern int GROUND_TESS_LEVEL;
extern int GRASS_INST_LEVEL;
extern int WATER_TESS_LEVEL;
extern float gamma;
extern int shadowFactor;
extern float shadowBlurJitter;
extern float shadowBlurArea;
extern float ssr_maxDistance;
extern float ssr_resolution;
extern int   ssr_lin_steps;
extern int   ssr_bin_steps;
extern float ssr_thickness;


extern float landSize;
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
extern float grassProb;
extern float grassSize;
extern float grassCrit;

extern float speed;
extern float rotSpeed;
extern float tSpeed;


extern unsigned int texture_skybox;
#endif