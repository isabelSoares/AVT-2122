//
// 
// AVT demo light 
// based on demos from GLSL Core Tutorial in Lighthouse3D.com   
//
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
//

#pragma comment(lib, "DevIL.lib")
#pragma comment(lib, "ILU.lib")

#include <math.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>
// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"
#include "Texture_Loader.h"

#include "avtFreeType.h"

#include "camera.h"

#include "spotlight.h"
#include "directionalLight.h"
#include "pointlight.h"
#include "flare.h"

#include "objects_geometry.h"
#include "game.h"

#define frand()			((float)rand()/RAND_MAX)

using namespace std;

#define CAPTION "AVT Per Fragment Phong Lightning Demo"
int WindowHandle = 0;
int WinX = 640, WinY = 480;

unsigned int FrameCount = 0;

VSShaderLib shader;
VSShaderLib shaderText;  //render bitmap text

// =================================== RENDER OBJECTS ===================================

MySkyBox skyBox;
MyCubeReflector cubeReflector;

MyTable table;
MyRoad road;
MyCar car;
std::vector<MyOrange> oranges;
std::vector<MyPacketButter> butters;
std::vector<MyCandle> candles;
std::vector<MyBillboardTree> trees;
std::vector<MyWaterParticle> particles;

// ======================================================================================

// =================================== CAMERA OBJECTS ===================================

float ratio, window_width, window_height;

const int ORTHO_CAMERA_ACTIVE = 0;
const int TOP_PERSPECTIVE_CAMERA_ACTIVE = 1;
const int CAR_PERSPECTIVE_CAMERA_ACTIVE = 2;
const int CAR_INSIDE_PERSPECTIVE_CAMERA_ACTIVE = 3;

int activeCamera = CAR_PERSPECTIVE_CAMERA_ACTIVE;

MyCamera orthoCamera = MyCamera(MyCameraType::Ortho, 0, 90, 5.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0 });
MyCamera topPerspectiveCamera = MyCamera(MyCameraType::Perspective, 0, 90, 130.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0 });
MyCamera carCamera = MyCamera(MyCameraType::Perspective, 0, 15, 8.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0});
MyCamera carInsideCamera = MyCamera(MyCameraType::Perspective, 0, 15, 0.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0 });
MyCamera carBehindCamera = MyCamera(MyCameraType::Perspective, 0, 15, 0.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0 });

std::vector<MyCamera*> cameras = {
	&orthoCamera ,
	&topPerspectiveCamera,
	&carCamera,
	&carInsideCamera
};

// ======================================================================================

// =================================== LIGHT OBJECTS ===================================

const int NUMBER_SPOTLIGHTS = 2;

MySpotlight spotlights[NUMBER_SPOTLIGHTS] = {
	MySpotlight(MyVec3{0, 0, 0}, {}, {}, 20, MySpotlightState::Off),
	MySpotlight(MyVec3{0, 0, 0}, {}, {}, 20, MySpotlightState::Off),
};

const int NUMBER_DIRECTIONAL_LIGHTS = 1;

MyDirectionalLight directionalLights[NUMBER_DIRECTIONAL_LIGHTS] = {
	MyDirectionalLight(MyVec3{0, -1, 1}, MyDirectionalLightState::Off),
};

const int NUMBER_POINTLIGHTS = 6;

MyPointlight pointlights[NUMBER_POINTLIGHTS] = {
	MyPointlight(MyVec3{0, 0, 0}, {}, {}, MyPointlightState::On),
	MyPointlight(MyVec3{0, 0, 0}, {}, {}, MyPointlightState::On),
	MyPointlight(MyVec3{0, 0, 0}, {}, {}, MyPointlightState::On),
	MyPointlight(MyVec3{0, 0, 0}, {}, {}, MyPointlightState::On),
	MyPointlight(MyVec3{0, 0, 0}, {}, {}, MyPointlightState::On),
	MyPointlight(MyVec3{0, 0, 0}, {}, {}, MyPointlightState::On),
};

// =====================================================================================

// =================================== OTHER OBJECTS ===================================

MyGame game = MyGame();

bool fogActivated = false;
bool flareEffect = false;
bool bumpMapping = false;

MyVec3 START_POSITION = MyVec3{ 0, 0, 0 };

const int FLARE_POINTLIGHT = 3;
std::vector<char*> flareTextureNames =  {"crcl", "flar", "hxgn", "ring", "sun"};

int timesToGenerateParticles = 0;

// =====================================================================================

// =================================== OTHER CONSTANTS ==================================
const int FPS = 60;

const int TABLE_SIZE = 250;
const int NUMBER_ORANGES = 0;
const int NUMBER_PARTICLES = 2;
const int TIME_PARTICLES = 200;
const int CHECKER_LENGTH = 8;

const int TREES_PER_MINIMUM = 3;
const int TREES_PER_MAXIMUM = 9;

const float ORTHO_FRUSTUM_HEIGHT = (TABLE_SIZE / 2) * 1.05;

const string font_name = "fonts/Acme-Regular.ttf";
// ======================================================================================

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices5
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint model_uniformId;
GLint view_uniformId;
GLint normal_uniformId;

GLint reflect_perFragment_uniformId;
GLint fogActivated_uniformId;
GLint bumpMode_uniformId;
GLint shadowMode_uniformId;

// PointLight UniformID
GLint lpPos_uniformId;
GLint lpState_uniformId;

// DirectionalLight UniformID
GLint ldDirection_uniformId;
GLint ldState_uniformId;

// Spotlight UniformID
GLint lsPos_uniformId;
GLint lsDirection_uniformId;
GLint lsAngle_uniformId;
GLint lsState_uniformId;

// Textures UniformID
GLint tex_loc0, tex_loc1, tex_loc2, tex_loc3, tex_loc4, tex_loc5, tex_cube_loc, texbump_loc0;
GLint texMode_uniformId;
GLuint TextureArray[8];
GLuint FlareTextureArray[5];

// Assimp UniformID
GLint normalMap_loc;
GLint specularMap_loc;
GLint diffMapCount_loc;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Frame counting and FPS computation
long myTime,timebase = 0,frame = 0;
char s[32];

void initGameObjects() {

	skyBox = MySkyBox(MyVec3{ 0, 0, 0 }, MyVec3{ 500, 500, 500 });
	cubeReflector = MyCubeReflector(MyVec3{ 0, 0, 0 }, MyVec3{ 15, 15, 15 });

	table = MyTable(MyVec3{ 0, -0.1, 0 }, MyVec3{ TABLE_SIZE, 0.2, TABLE_SIZE });
	road = MyRoad(MyVec3{ 0, -0.2, 0 }, 20, 0.5 * TABLE_SIZE, 0.33 * TABLE_SIZE, TABLE_SIZE, TABLE_SIZE, 3.5, 0.4, 0.8);
	std::vector<MySpotlight*> carSpotlights = { &spotlights[0], &spotlights[1] };
	car = MyCar(MyVec3{ 0, 0, 0 }, carSpotlights);
	oranges = {};
	for (int i = 0; i < NUMBER_ORANGES; i++) {
		float orangeX = rand() % TABLE_SIZE - TABLE_SIZE / 2;
		float orangeY = rand() % TABLE_SIZE - TABLE_SIZE / 2;
		oranges.push_back(MyOrange(MyVec3{ orangeX, 2.0, orangeY }, MyVec3{ 1, 1, 1 }, float(game.gameTime / 300 + 1)));
	}
	butters = { MyPacketButter(MyVec3{ -0.25 * TABLE_SIZE + 5, 0.6, -0.3 * TABLE_SIZE + 30 }, MyVec3{3.0f, 1.2f, 1.5f}),
				MyPacketButter(MyVec3{ -0.25 * TABLE_SIZE - 5, 0.6, 0.3 * TABLE_SIZE - 30 }, MyVec3{3.0f, 1.2f, 1.5f}),
				MyPacketButter(MyVec3{ 0.20 * TABLE_SIZE, 0.6, 0.33 * TABLE_SIZE }, MyVec3{3.0f, 1.2f, 1.5f}),
				MyPacketButter(MyVec3{ 0.20 * TABLE_SIZE, 0.6, -0.33 * TABLE_SIZE }, MyVec3{3.0f, 1.2f, 1.5f}) };
	candles = { MyCandle(MyVec3{-0.34 * TABLE_SIZE, 0, 0.22 * TABLE_SIZE}, 2, 0.4, &pointlights[0]), MyCandle(MyVec3{0.34 * TABLE_SIZE, 0, 0.22 * TABLE_SIZE}, 2, 0.4, &pointlights[1]),
				MyCandle(MyVec3{ 0, 0, -0.48 * TABLE_SIZE}, 2, -0.4, &pointlights[3]), MyCandle(MyVec3{ 0, 0, 0.32 * TABLE_SIZE}, 2, 0.4, &pointlights[2]),
				MyCandle(MyVec3{-0.16 * TABLE_SIZE, 0, -0.22 * TABLE_SIZE}, 2, 0.4, &pointlights[4]), MyCandle(MyVec3{0.16 * TABLE_SIZE, 0, -0.22 * TABLE_SIZE}, 2, 0.4, &pointlights[5]) };
	
	float square_size = TABLE_SIZE / CHECKER_LENGTH;
	trees = {};
	std::vector<int> blackList = {3, 5, 10, 12, 14, 17, 21, 26, 30, 33, 37, 42, 46, 49, 51, 53, 58, 60};
	for (int i = 1; i < CHECKER_LENGTH * CHECKER_LENGTH; i++) {

		if ((i / CHECKER_LENGTH) % 2 == (i % CHECKER_LENGTH) % 2) continue;
		if (std::find(blackList.begin(), blackList.end(), i) != blackList.end()) continue;


		// Starting at top left of table
		float startPosX = square_size * (i % CHECKER_LENGTH);
		float startPosY = square_size * (i / CHECKER_LENGTH);

		int numberTrees = rand() % (TREES_PER_MAXIMUM - TREES_PER_MINIMUM + 1) + TREES_PER_MINIMUM;

		for (int j = 0; j < numberTrees; j++) {

			float randomFloatX = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float posX = randomFloatX * square_size + startPosX;
			float randomFloatY = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float posY = randomFloatY * square_size + startPosY;

			trees.push_back({ MyBillboardTree(MyVec3{ -0.5f * TABLE_SIZE + posX, 0, -0.5f * TABLE_SIZE + posY}, 6) });
		}
		
	};

	particles = {};
	timesToGenerateParticles = 0;

	START_POSITION = MyVec3{ -0.25 * TABLE_SIZE, 0, 0 };
	car.position = START_POSITION;
}

void generateNeededParticles() {

	if (timesToGenerateParticles <= 0) return;

	timesToGenerateParticles = timesToGenerateParticles - 1;

	for (int side = 0; side < 2; side++) {

		int sideNormalized = side * 2 - 1;

		for (int i = 0; i < NUMBER_PARTICLES; i++) {

			float posX = 0.8 * frand() - 0.4;
			float posZ = 0.8 * frand() - 0.4;

			MyVec3 velocity = MyVec3{ posX / 100, 0.12, posZ / 100 };
			MyVec3 accelaration = MyVec3{ 0, -0.0008, 0 };
			MyWaterParticle particle = MyWaterParticle(MyVec3{ -0.25f * TABLE_SIZE + sideNormalized * 10 + posX, 0, posZ }, velocity, accelaration, 0.0036f + 0.0004 * frand() - 0.00039, 1.0f);

			particles.push_back(particle);

		}
	}
}

void timer(int value) {
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
    FrameCount = 0;
    glutTimerFunc(1000, timer, 0);

	// Update GameTime
	if (game.state == MyGameState::Running) game.gameTime = game.gameTime + 1;
}

void refresh(int value)
{

	glutPostRedisplay();
	glutTimerFunc(1000 / FPS, refresh, 0);
}

void drawStencilBackMirror() {

	pushMatrix(PROJECTION);
	pushMatrix(VIEW);
	pushMatrix(MODEL);

	/* create a diamond shaped stencil area */
	loadIdentity(PROJECTION);
	loadIdentity(VIEW);

	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	ortho(-2, 2, -2, 2, -1, 1);

	glUseProgram(shader.getProgramIndex());

	translate(MODEL, 0, 1.74, 0);
	scale(MODEL, 0.831 * (1.887906f / ratio), 0.5, 1);
	translate(MODEL, 0.045, 0, 0);
	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	//glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	glStencilFunc(GL_NEVER, 0x1, 0x1);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

	MyMesh mesh = createCube();

	glBindVertexArray(mesh.vao);
	glDrawElements(mesh.type, mesh.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);
}

void changeCameraSize() {

	loadIdentity(PROJECTION);

	MyCamera* currentCamera = cameras[activeCamera];
	if (currentCamera->type == MyCameraType::Perspective) { perspective(53.13f, ratio, 0.1f, 1000.0f); }
	else { ortho(-ORTHO_FRUSTUM_HEIGHT * ratio, ORTHO_FRUSTUM_HEIGHT * ratio, -ORTHO_FRUSTUM_HEIGHT, ORTHO_FRUSTUM_HEIGHT, -20, 2000); }
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	if(h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
	window_width = w;
	window_height = h;
	changeCameraSize();
}

// ------------------------------------------------------------
//
// Dealing with lights
//

void dealWithLights() {

	// Pointlights Load Info
	float* resp_pos = (float*)malloc(NUMBER_POINTLIGHTS * 4 * sizeof(float));
	int* resp_state = (int*)malloc(NUMBER_POINTLIGHTS * sizeof(int));
	for (int lightIndex = 0; lightIndex < NUMBER_POINTLIGHTS; lightIndex++) {

		MyPointlight * currentPointlight = &pointlights[lightIndex];
		currentPointlight->computeEyeStuff();

		MyVec3 lightPos = currentPointlight->getPosition();
		float lightOnePos[4] = { lightPos.x, lightPos.y, lightPos.z, 1.0 };
		multMatrixPoint(VIEW, lightOnePos, resp_pos + lightIndex * 4);

		int state = currentPointlight->getState();
		memcpy(resp_state + lightIndex, &state, sizeof(int));
	}
	glUniform4fv(lpPos_uniformId, NUMBER_POINTLIGHTS, resp_pos);
	glUniform1iv(lpState_uniformId, NUMBER_POINTLIGHTS, resp_state);

	// DirectionalLight Load Info
	float* resd_dir = (float*)malloc(NUMBER_DIRECTIONAL_LIGHTS * 4 * sizeof(float));
	int* resd_state = (int*)malloc(NUMBER_DIRECTIONAL_LIGHTS * sizeof(int));
	for (int lightIndex = 0; lightIndex < NUMBER_DIRECTIONAL_LIGHTS; lightIndex++) {

		MyDirectionalLight * currentDirectionalLight = &directionalLights[lightIndex];

		MyVec3 lightDirection = currentDirectionalLight->getDirection();
		float lightOneDirection[4] = { lightDirection.x, lightDirection.y, lightDirection.z, 0.0 };
		multMatrixPoint(VIEW, lightOneDirection, resd_dir + lightIndex * 4);

		int state = currentDirectionalLight->getState();
		memcpy(resd_state + lightIndex, &state, sizeof(int));
	}
	glUniform4fv(ldDirection_uniformId, NUMBER_DIRECTIONAL_LIGHTS, resd_dir);
	glUniform1iv(ldState_uniformId, NUMBER_DIRECTIONAL_LIGHTS, resd_state);

	// Spotlights Load Info
	float* ress_pos = (float*)malloc(NUMBER_SPOTLIGHTS * 4 * sizeof(float));
	float* ress_dir = (float*)malloc(NUMBER_SPOTLIGHTS * 4 * sizeof(float));
	float* ress_angle = (float*)malloc(NUMBER_SPOTLIGHTS * sizeof(float));
	int* ress_state = (int*)malloc(NUMBER_SPOTLIGHTS * sizeof(int));
	for (int lightIndex = 0; lightIndex < NUMBER_SPOTLIGHTS; lightIndex++) {

		MySpotlight* currentSpotlight = &spotlights[lightIndex];
		currentSpotlight->computeEyeStuff();

		MyVec3 lightPos = currentSpotlight->getPosition();
		float lightOnePos[4] = { lightPos.x, lightPos.y, lightPos.z, 1.0 };
		multMatrixPoint(VIEW, lightOnePos, ress_pos + lightIndex * 4);

		MyVec3 lightDirection = currentSpotlight->getDirection();
		float lightOneDirection[4] = { lightDirection.x, lightDirection.y, lightDirection.z, 0.0 };
		multMatrixPoint(VIEW, lightOneDirection, ress_dir + lightIndex * 4);

		float angle = currentSpotlight->getConeAngle();
		memcpy(ress_angle + lightIndex, &angle, sizeof(float));

		int state = currentSpotlight->getState();
		memcpy(ress_state + lightIndex, &state, sizeof(int));
	}
	glUniform4fv(lsPos_uniformId, NUMBER_SPOTLIGHTS, ress_pos);
	glUniform4fv(lsDirection_uniformId, NUMBER_SPOTLIGHTS, ress_dir);
	glUniform1fv(lsAngle_uniformId, NUMBER_SPOTLIGHTS, ress_angle);
	glUniform1iv(lsState_uniformId, NUMBER_SPOTLIGHTS, ress_state);
}

bool isCollision(MyVec3 minObject1, MyVec3 maxObject1, MyVec3 minObject2, MyVec3 maxObject2) {

	bool xCollision = minObject1.x <= maxObject2.x && maxObject1.x >= minObject2.x;
	bool yCollision = minObject1.y <= maxObject2.y && maxObject1.y >= minObject2.y;
	bool zCollision = minObject1.z <= maxObject2.z && maxObject1.z >= minObject2.z;

	return xCollision && yCollision && zCollision;

}

void checkCollisions() {

	enum class CollisionType { NONE, STOP, RESTART };
	CollisionType collision = CollisionType::NONE;

	std::vector<MyVec3> carPositions = car.getBoundRect();
	MyVec3 carMinPosition = carPositions[0];
	MyVec3 carMaxPosition = carPositions[1];

	// Car with Oranges
	for (int i = 0; i < NUMBER_ORANGES; i++) {

		std::vector<MyVec3> orangePositions = oranges[i].getBoundRect();
		MyVec3 orangeMinPosition = orangePositions[0];
		MyVec3 orangeMaxPosition = orangePositions[1];

		// Test Collision: Car - Orange
		if (isCollision(carMinPosition, carMaxPosition, orangeMinPosition, orangeMaxPosition)) {
			collision = CollisionType::RESTART;
		}
	}

	// Car with cheerios
	for (int cheerioIndex = 0; cheerioIndex < road.cheerios.size(); cheerioIndex ++) {

		std::vector<MyVec3> cheerioPositions = road.cheerios[cheerioIndex].getBoundRect();
		MyVec3 cheerioMinPosition = cheerioPositions[0];
		MyVec3 cheerioMaxPosition = cheerioPositions[1];

		if (isCollision(carMinPosition, carMaxPosition, cheerioMinPosition, cheerioMaxPosition)) {
			if (collision != CollisionType::RESTART) collision = CollisionType::STOP;

			road.cheerios[cheerioIndex].velocity = 0.015;
			MyVec3 movementVector = (road.cheerios[cheerioIndex].getPosition() - car.getPosition()).normalize();
			road.cheerios[cheerioIndex].direction = MyVec3{ movementVector.x, 0, movementVector.z };
		}
	}

	// Car with butter
	for (int i = 0; i < butters.size(); i++) {

		std::vector<MyVec3> butterPositions = butters[i].getBoundRect();
		MyVec3 butterMinPosition = butterPositions[0];
		MyVec3 butterMaxPosition = butterPositions[1];

		if (isCollision(carMinPosition, carMaxPosition, butterMinPosition, butterMaxPosition)) {
			if (collision != CollisionType::RESTART) collision = CollisionType::STOP;

			butters[i].velocity = 0.015;
			MyVec3 movementVector = (butters[i].getPosition() - car.getPosition()).normalize();
			butters[i].direction = MyVec3{ movementVector.x, 0, movementVector.z };
		}
	}

	// Update Car according to Collision
	if (collision != CollisionType::NONE) {
		car.collisionStop();

		if (collision == CollisionType::RESTART) { 

			car.position = START_POSITION;
			car.direction = MyVec3{ 0, 0, -1 };

			// Player loses 1 life
			game.loseLife();
		}

		car.velocity = 0; car.acceleration = 0;
	}

}

void drawObjects() {

	MyCamera* currentCamera = cameras[activeCamera];

	skyBox.render(shader);
	cubeReflector.render(shader);

	// Non Transparent Objects
	road.render(shader);
	for (MyOrange& orange : oranges) { orange.render(shader); }
	for (MyCandle& candle : candles) { candle.render(shader); }
	car.render(shader);

	// Transparent with non transparent behavior
	for (MyBillboardTree& tree : trees) {
		tree.update(currentCamera->position);
		tree.render(shader);
	}
	// Trasparent Objects
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (MyPacketButter& butter : butters) { butter.render(shader); }
	for (MyWaterParticle& particle : particles) {

		particle.update(currentCamera->position);
		particle.render(shader);
	}
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

// ------------------------------------------------------------
// ------------------------------------------------------------
//
// Render stufff
//

void renderScene(void) {

	GLint loc;

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt
	MyCamera *currentCamera = cameras[activeCamera];
	lookAt(currentCamera->position.x, currentCamera->position.y, currentCamera->position.z, currentCamera->lookAtPosition.x, currentCamera->lookAtPosition.y, currentCamera->lookAtPosition.z, 0,1,0);
	// use our shader
	glUseProgram(shader.getProgramIndex());

	// Deal with Textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, TextureArray[3]);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, TextureArray[4]);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, TextureArray[5]);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, TextureArray[6]);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, TextureArray[7]);

	glUniform1i(tex_loc0, 0);
	glUniform1i(tex_loc1, 1);
	glUniform1i(tex_loc2, 2);
	glUniform1i(tex_loc3, 3);
	glUniform1i(tex_loc4, 4);
	glUniform1i(tex_cube_loc, 5);
	glUniform1i(tex_loc5, 6);
	glUniform1i(texbump_loc0, 7);

	glUniform1i(fogActivated_uniformId, fogActivated);

	dealWithLights();

	if (game.state == MyGameState::Restart) {

		initGameObjects();
		game.resumeGame();
	}

	// ================================ Check Position Car ================================

	MyVec3 carPosition = car.getPosition();
	if (abs(carPosition.x) > TABLE_SIZE / 2 || abs(carPosition.z) > TABLE_SIZE / 2) {

		car.position = START_POSITION;
		car.direction = MyVec3{ 0, 0, -1 };

		// Player loses 1 life
		game.loseLife();
	}

	// ================================ ================== ================================

	// ================================ Check Position Oranges ================================

	for (int i = 0; i < NUMBER_ORANGES; i++) {

		MyVec3 currentPosition = oranges[i].getPosition();
		if (abs(currentPosition.x) > TABLE_SIZE / 2 || abs(currentPosition.z) > TABLE_SIZE / 2) {

			float orangeX = rand() % TABLE_SIZE - TABLE_SIZE / 2;
			float orangeY = rand() % TABLE_SIZE - TABLE_SIZE / 2;
			oranges[i] = MyOrange(MyVec3{ orangeX, 2.0, orangeY }, MyVec3{ 1, 1, 1 }, float(game.gameTime / 300 + 1));
		}
	}

	// ====================================================================================


	particles.erase(std::remove_if(particles.begin(), particles.end(), [](MyWaterParticle i) { return i.isDead(); }), particles.end());
	generateNeededParticles();

	table.render(shader);

	// ====================================== SHADOWS ======================================

	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);

	glStencilFunc(GL_NEVER, 0x1, 0x1);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	// Fill stencil buffer with Ground shape; never rendered into color buffer
	table.render(shader);

	glEnable(GL_BLEND);

	// Render the Shadows
	glUniform1i(shadowMode_uniformId, true);
	float lightPosConverted[4] = { pointlights[3].positionVec.x, pointlights[3].positionVec.y, pointlights[3].positionVec.z, 1.0 };
	float mat[16];
	GLfloat plano_chao[4] = { 0,1,0,0 };
	shadow_matrix(mat, plano_chao, lightPosConverted);

	glDisable(GL_DEPTH_TEST); //To force the shadow geometry to be rendered even if behind the floor

	//Dark the color stored in color buffer
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

	pushMatrix(MODEL);
	multMatrix(MODEL, mat);
	drawObjects();
	popMatrix(MODEL);

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glUniform1i(shadowMode_uniformId, false);

	// ====================================== ======= ======================================
	
	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	if (activeCamera == CAR_INSIDE_PERSPECTIVE_CAMERA_ACTIVE) drawStencilBackMirror();

	glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	//table.render(shader);
	drawObjects();

	glDisable(GL_STENCIL_TEST);

	// Render back mirror
	if (activeCamera == CAR_INSIDE_PERSPECTIVE_CAMERA_ACTIVE) {

		glEnable(GL_STENCIL_TEST);

		drawStencilBackMirror();

		pushMatrix(VIEW);
		pushMatrix(MODEL);

		loadIdentity(VIEW);
		loadIdentity(MODEL);

		glStencilFunc(GL_EQUAL, 0x1, 0x1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		MyCamera* backCamera = &carBehindCamera;
		lookAt(backCamera->position.x, backCamera->position.y, backCamera->position.z, backCamera->lookAtPosition.x, backCamera->lookAtPosition.y, backCamera->lookAtPosition.z, 0, 1, 0);

		dealWithLights();

		table.render(shader);
		drawObjects();

		popMatrix(VIEW);
		popMatrix(MODEL);

		glDisable(GL_STENCIL_TEST);

	} else glClear(GL_STENCIL_BUFFER_BIT);

	if (flareEffect) {
		pointlights[3].computeEyeStuff();
		MyVec3 lightPos = pointlights[3].getPosition();
		renderWholeFlare(shader, lightPos);
	}

	if (game.state == MyGameState::Running) {

		car.tick();
		road.tick();
		for (MyOrange& orange : oranges) { orange.tick(); }
		for (MyPacketButter& butter : butters) { butter.tick(); }
	}

	checkCollisions();

	// Update Car Camera
	carCamera.translation.x = carPosition.x;
	carCamera.translation.y = carPosition.y;
	carCamera.translation.z = carPosition.z;
	carCamera.lookAtPosition = carPosition;
	carCamera.rotationDegrees = - (car.getDirectionDegrees() - 270.0f);

	carPosition = car.getPosition();

	// Update Car Inside Camera
	carInsideCamera.translation.x = carPosition.x - 0.25f * car.direction.x;
	carInsideCamera.translation.y = carPosition.y + 1.25f;
	carInsideCamera.translation.z = carPosition.z - 0.25f * car.direction.z;
	carInsideCamera.lookAtPosition = carPosition + car.direction * MyVec3{ 10, 10, 10 } + MyVec3{ 0, 1, 0 };
	carInsideCamera.rotationDegrees = -(car.getDirectionDegrees() - 270.0f);

	// Update Car Behind Camera
	carBehindCamera.translation.x = carPosition.x - 0.05f * car.direction.x;
	carBehindCamera.translation.y = carPosition.y + 1.21f;
	carBehindCamera.translation.z = carPosition.z - 0.05f * car.direction.z;
	carBehindCamera.lookAtPosition = carPosition - car.direction * MyVec3{ 10, 0, 10 } + MyVec3{ 0, -2.9, 0 };
	carBehindCamera.rotationDegrees = -(car.getDirectionDegrees() - 90.0f);

	currentCamera->updateCamera();
	if (activeCamera == CAR_INSIDE_PERSPECTIVE_CAMERA_ACTIVE) carBehindCamera.updateCamera();

	game.update(car.getPosition());

	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// Render Text
	glEnable(GL_BLEND);
	game.renderHUD(shaderText, window_width, window_height);
	glDisable(GL_BLEND);

	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy) {

	// ========================== Allocate Variables for Cases ==========================
	MyCamera *currentCamera = cameras[activeCamera];
	// ========================== ============================ ==========================

	switch(key) {

		case 27:
			glutLeaveMainLoop();
			break;

		case '1':
			activeCamera = ORTHO_CAMERA_ACTIVE;
			changeCameraSize();
			break;
		case '2':
			activeCamera = TOP_PERSPECTIVE_CAMERA_ACTIVE;
			changeCameraSize();
			break;
		case '3':
			activeCamera = CAR_PERSPECTIVE_CAMERA_ACTIVE;
			changeCameraSize();
			break;
		case '4':
			activeCamera = CAR_INSIDE_PERSPECTIVE_CAMERA_ACTIVE;
			changeCameraSize();
			break;

		// ================= CAR STUFF =================
		case 'O':
		case 'o':
			car.turnLeft();
			break;
		case 'P':
		case 'p':
			car.turnRight();
			break;
		case 'A':
		case 'a':
			car.backward();
			break;
		case 'Q':
		case 'q':
			car.forward();
			break;

		// ================= LIGHT STUFF =================

		case 'N':
		case 'n':
			if (directionalLights[0].state == MyDirectionalLightState::On) directionalLights[0].turnOff();
			else directionalLights[0].turnOn();
			break;
		case 'C':
		case 'c':
			for (int lightIndex = 0; lightIndex < NUMBER_POINTLIGHTS; lightIndex++) {
				if (pointlights[lightIndex].state == MyPointlightState::On) pointlights[lightIndex].turnOff();
				else pointlights[lightIndex].turnOn();

				if (lightIndex == FLARE_POINTLIGHT && pointlights[lightIndex].state == MyPointlightState::Off) flareEffect = false;
			}

			break;
		case 'H':
		case 'h':
			for (int lightIndex = 0; lightIndex < NUMBER_SPOTLIGHTS; lightIndex++) {
				if (spotlights[lightIndex].state == MySpotlightState::On) spotlights[lightIndex].turnOff();
				else spotlights[lightIndex].turnOn();
			}
			break;
		
		case 'L':
		case 'l':
			if (flareEffect) flareEffect = false;
			else if (pointlights[FLARE_POINTLIGHT].state == MyPointlightState::On) flareEffect = true;
			break;

		// ================= GAME STUFF =================

		case 'S':
		case 's':
			if (game.state == MyGameState::Paused) game.resumeGame();
			else if (game.state == MyGameState::Running) game.pauseGame();
			break;

		case 'R':
		case 'r':
			game.restartGame();
			break;

		case 'W':
		case 'w':
			timesToGenerateParticles = TIME_PARTICLES;
			break;

		// ================= OTHER STUFF =================

		case 'F':
		case 'f':
			fogActivated = !fogActivated;
			break;

		case 'B':
		case 'b':
			bumpMapping = !bumpMapping;
			break;

		/*case 'c':
			printf("Camera Spherical Coordinates (%f, %f, %f)\n", currentCamera->alpha, currentCamera->beta, currentCamera->r);
			break;
		case 'm': glEnable(GL_MULTISAMPLE); break;
		case 'n': glDisable(GL_MULTISAMPLE); break;*/
	}
}

void processKeysUp(unsigned char key, int xx, int yy)
{
	MyCamera* currentCamera = cameras[activeCamera];
	switch (key) {

		// ================= CAR STUFF =================
	case 'O':
	case 'o':
		car.turnLeft();
		break;
	case 'P':
	case 'p':
		car.turnRight();
		break;
	case 'A':
	case 'a':
		car.stop();
		break;
	case 'Q':
	case 'q':
		car.stop();
		break;
		// ==============================================
	}
}

// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN && activeCamera == CAR_PERSPECTIVE_CAMERA_ACTIVE)  {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP || activeCamera != CAR_PERSPECTIVE_CAMERA_ACTIVE) {

		/*
		MyCamera *currentCamera = cameras[CAR_PERSPECTIVE_CAMERA_ACTIVE];
		if (tracking == 1) {
			currentCamera->alpha -= (xx - startX);
			currentCamera->beta += (yy - startY);
		}
		else if (tracking == 2) {
			currentCamera->r += (yy - startY) * 0.01f;
			if (currentCamera->r < 0.1f)
				currentCamera->r = 0.1f;
		}
		*/
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX =  - xx + startX;
	deltaY =    yy - startY;

	// left mouse button: move camera
	MyCamera *currentCamera = cameras[CAR_PERSPECTIVE_CAMERA_ACTIVE];
	if (tracking == 1) {

		alphaAux = currentCamera->alpha + deltaX;
		betaAux = currentCamera->beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = currentCamera->r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = currentCamera->alpha;
		betaAux = currentCamera->beta;
		rAux = currentCamera->r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}

	if (tracking != 0) {
		currentCamera->alpha = alphaAux;
		currentCamera->beta = betaAux;
		currentCamera->r = rAux;
		currentCamera->updateCamera();

		// Reset Values
		startX = xx;
		startY = yy;
	}

//  uncomment this if not using an idle or refresh func
//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	if (activeCamera == CAR_PERSPECTIVE_CAMERA_ACTIVE) {
		MyCamera *currentCamera = cameras[activeCamera];
		currentCamera->r += direction * 0.1f;
		if (currentCamera->r < 0.1f)
			currentCamera->r = 0.1f;

		currentCamera->updateCamera();
	}

//  uncomment this if not using an idle or refresh func
//	glutPostRedisplay();
}

// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for models
	shader.init();
	// shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight.vert");
	// shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight.frag");
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0,"colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");
	glBindAttribLocation(shader.getProgramIndex(), TANGENT_ATTRIB, "tangent");
	glBindAttribLocation(shader.getProgramIndex(), BITANGENT_ATTRIB, "bitangent");

	glLinkProgram(shader.getProgramIndex());

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	model_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_Model");
	view_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_View");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");

	reflect_perFragment_uniformId = glGetUniformLocation(shader.getProgramIndex(), "reflect_perFrag");
	fogActivated_uniformId = glGetUniformLocation(shader.getProgramIndex(), "fogActivated");

	// Pointlight Get UniformID
	lpPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "lp_positions");
	lpState_uniformId = glGetUniformLocation(shader.getProgramIndex(), "lp_states");

	// DirectionalLight Get UniformID
	ldDirection_uniformId = glGetUniformLocation(shader.getProgramIndex(), "ld_directions");
	ldState_uniformId = glGetUniformLocation(shader.getProgramIndex(), "ld_states");

	// Spotlight Get UniformID
	lsPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "ls_positions");
	lsDirection_uniformId = glGetUniformLocation(shader.getProgramIndex(), "ls_directions");
	lsAngle_uniformId = glGetUniformLocation(shader.getProgramIndex(), "ls_angles");
	lsState_uniformId = glGetUniformLocation(shader.getProgramIndex(), "ls_states");

	// Textures Get UniformID
	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");
	tex_loc0 = glGetUniformLocation(shader.getProgramIndex(), "texmap0");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");
	tex_loc3 = glGetUniformLocation(shader.getProgramIndex(), "texmap3");
	tex_loc4 = glGetUniformLocation(shader.getProgramIndex(), "texmap4");
	tex_loc5 = glGetUniformLocation(shader.getProgramIndex(), "texmap5");
	tex_cube_loc = glGetUniformLocation(shader.getProgramIndex(), "cubeMap");
	texbump_loc0 = glGetUniformLocation(shader.getProgramIndex(), "texmapBump0");

	bumpMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "bumpMode");
	shadowMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "shadowMode");

	glUniform1i(shadowMode_uniformId, false);

	// Assimp Shader UniformID
	normalMap_loc = glGetUniformLocation(shader.getProgramIndex(), "normalMap");
	specularMap_loc = glGetUniformLocation(shader.getProgramIndex(), "specularMap");
	diffMapCount_loc = glGetUniformLocation(shader.getProgramIndex(), "diffMapCount");
	
	// printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());
	printf("InfoLog for Per Fragment Gouraud Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	// Shader for bitmap Text
	shaderText.init();
	shaderText.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/text.vert");
	shaderText.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/text.frag");

	glLinkProgram(shaderText.getProgramIndex());
	printf("InfoLog for Text Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());
	
	return(shader.isProgramLinked() && shaderText.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

int init() {
	for (MyCamera* camera : cameras) {
		// set the camera position based on its spherical coordinates
		camera->position.x = camera->r * sin(camera->alpha * 3.14f / 180.0f) * cos(camera->beta * 3.14f / 180.0f);
		camera->position.z = camera->r * cos(camera->alpha * 3.14f / 180.0f) * cos(camera->beta * 3.14f / 180.0f);
		camera->position.y = camera->r * sin(camera->beta * 3.14f / 180.0f);
	}

	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {
		printf("wrong DevIL version \n");
		exit(0);
	}

	ilInit();

	/// Initialization of freetype library with font_name file
	freeType_init(font_name, {
		SymbolInformation{"alive", "./materials/alive.png"},
		SymbolInformation{"dead", "./materials/dead.png"},
		SymbolInformation{"coin", "./materials/coin.png"}
	});

	glGenTextures(8, TextureArray);
	Texture2D_Loader(TextureArray, "./materials/roadGrass3.jpg", 0);
	Texture2D_Loader(TextureArray, "./materials/lightwood.tga", 1);
	Texture2D_Loader(TextureArray, "./materials/orange.jpg", 2);
	Texture2D_Loader(TextureArray, "./materials/tree.tga", 3);
	Texture2D_Loader(TextureArray, "./materials/particle.tga", 4);
	//const char* filenames[] = { "./materials/posX.png", "./materials/negX.png", "./materials/posY.png", "./materials/negY.png", "./materials/posZ.png", "./materials/negZ.png" };
	const char* filenames[] = { "./materials/skyBox/right.png", "./materials/skyBox/left.png", "./materials/skyBox/top.png", "./materials/skyBox/bot.png", "./materials/skyBox/front.png", "./materials/skyBox/back.png" };
	TextureCubeMap_Loader(TextureArray, filenames, 5);
	Texture2D_Loader(TextureArray, "./materials/cheerio.jpg", 6);
	Texture2D_Loader(TextureArray, "./materials/cheerioBumpmap.png", 7);

	//Flare elements textures
	glGenTextures(5, FlareTextureArray);
	Texture2D_Loader(FlareTextureArray, "./materials/crcl.tga", 0);
	Texture2D_Loader(FlareTextureArray, "./materials/flar.tga", 1);
	Texture2D_Loader(FlareTextureArray, "./materials/hxgn.tga", 2);
	Texture2D_Loader(FlareTextureArray, "./materials/ring.tga", 3);
	Texture2D_Loader(FlareTextureArray, "./materials/sun.tga", 4);

	// Initialize Objects
	initGameObjects();

	// Initialize Flar
	initFlare("./materials/flare.txt");

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	//glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Blending Stuff
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// Stencil Stuff
	glClearStencil(0x0);
	glEnable(GL_STENCIL_TEST);
}

// ------------------------------------------------------------
//
// Main function
//

int main(int argc, char **argv) {

	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA|GLUT_STENCIL|GLUT_MULTISAMPLE);

	glutInitContextVersion (3, 3);
	glutInitContextProfile (GLUT_CORE_PROFILE );
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100,100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);

	//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutTimerFunc(0, timer, 0);
	// glutIdleFunc(renderScene);  // Use it for maximum performance
	glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever

	//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutKeyboardUpFunc(processKeysUp);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc ( mouseWheel ) ;
	
	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf ("Vendor: %s\n", glGetString (GL_VENDOR));
	printf ("Renderer: %s\n", glGetString (GL_RENDERER));
	printf ("Version: %s\n", glGetString (GL_VERSION));
	printf ("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	if (!init())
		printf("Could not Load the Model\n");

	//  GLUT main loop
	glutMainLoop();

	return(0);

}

