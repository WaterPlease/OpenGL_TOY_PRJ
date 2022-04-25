#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include "glm_pre.h"


// RENDERING SETTING
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

extern unsigned int GL_TEXTURE_SKYBOX;

extern bool bPrameterChange;
extern bool bDrawGUI;
extern bool bDrawFireflies;

extern bool bWireMode;

// GRAPHICAL SETTING
extern int TESS_GROUND_LEVEL;
extern int TESS_WATER_LEVEL;
extern int INSTANCE_GRASS_LEVEL;
extern float GAMMA;
extern float SHADOW_BLUR_JITTER;
extern float SHADOW_BLUR_AREA;
extern float SSR_DISTANCE;
extern float SSR_RESOLUTION;
extern int   SSR_LIN_STEPS;
extern int   SSR_BIN_STEPS;
extern float SSR_THICKNESS;

// TERRAIN SETTING
extern float LANDSIZE;
extern float LAND_HEIGHT;

extern float WATER_SIZE;
extern float WATER_TIME_FACTOR;
extern float WATER_LEVEL;
extern float WATER_UV_FACTOR;
extern float WATER_WAVE_LENGTH;
extern float WATER_WAVE_STEEPENSS;
extern float WATER_TRANSPARENCY;

extern float LAND_ROCK_UV_FACTOR;
extern float LAND_GRASS_UV_FACTOR;

extern float GRASS_WAVE_LENGTH;
extern float GRASS_WAVE_STEEPNESS;
extern float GRASS_DENSITY;
extern float GRASS_SIZE;
extern float GRASS_SLOPE_CRITERION;

extern float CAM_MOVE_SPEED;
extern float CAM_ROT_SPEED;
extern float TIME_SPEED;


// BenchMark results

extern bool bBenchmark;
extern float benchStart;
extern float benchTime;
extern int totalFrame;
extern float totalRenderTime;
extern unsigned int queryBuffer;
#endif