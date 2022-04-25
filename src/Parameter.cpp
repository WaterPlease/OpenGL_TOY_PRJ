#include "Parameter.h"

bool bDrawGUI = true;

int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;

int TESS_GROUND_LEVEL = 8;
int TESS_WATER_LEVEL = 64;
int INSTANCE_GRASS_LEVEL = 256;
float GAMMA = 1.6f;
bool bPrameterChange = true;
bool bWireMode = false;

glm::vec3 sunLightDir(0.0f,1.0f,0.0f);

float LANDSIZE = 100.0f;
float WATER_SIZE = 8.0f;
float WATER_TIME_FACTOR = 0.1;
float WATER_LEVEL = 2.0f;
float WATER_UV_FACTOR = 80.0f;
float LAND_ROCK_UV_FACTOR = 100.0;
float LAND_GRASS_UV_FACTOR = 100.0;
float LAND_HEIGHT = 15.0f;
float WATER_WAVE_LENGTH = 20.0f;
float WATER_WAVE_STEEPENSS = 0.5f;
float WATER_TRANSPARENCY = 0.3;
float GRASS_WAVE_LENGTH = 10.5f;
float GRASS_WAVE_STEEPNESS = 0.02f;
float GRASS_DENSITY = 1.0;
float GRASS_SIZE = 1.0f;
float GRASS_SLOPE_CRITERION = 0.941f;

float CAM_MOVE_SPEED = 10;
float CAM_ROT_SPEED = 0.25f;
float TIME_SPEED = 1.0f;

float SHADOW_BLUR_JITTER = 1.256;
float SHADOW_BLUR_AREA = 2.6232f;

bool bDrawFireflies = true;

unsigned int GL_TEXTURE_SKYBOX;

float SSR_DISTANCE = 30.0f;
float SSR_RESOLUTION = 0.5f;
int   SSR_LIN_STEPS = 75;
int   SSR_BIN_STEPS = 5;
float SSR_THICKNESS = 1.0f;


bool bBenchmark = false;
float benchStart;
float benchTime = 5.0f;
int totalFrame = 1.0;
float totalRenderTime = -1.0;
unsigned int queryBuffer;