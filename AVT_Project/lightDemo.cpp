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

// assimp include files. These three are usually needed.
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/scene.h"

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"
#include "Texture_Loader.h"
#include "meshFromAssimp.h"

#include "avtFreeType.h"

#include "camera.h"

#include "spotlight.h"
#include "directionalLight.h"
#include "pointlight.h"

#include "objects_geometry.h"
#include "game.h"

using namespace std;

#define CAPTION "AVT Per Fragment Phong Lightning Demo"
int WindowHandle = 0;
int WinX = 640, WinY = 480;

// Created an instance of the Importer class in the meshFromAssimp.cpp file
extern Assimp::Importer importer;
// the global Assimp scene object
extern const aiScene* scene;
char model_dir[50];  //initialized by the user input at the console
// scale factor for the Assimp model to fit in the window
extern float scaleFactor;

//Array of meshes 
vector<struct MyMesh> myMeshes;


unsigned int FrameCount = 0;

VSShaderLib shader;
VSShaderLib shaderText;  //render bitmap text

// =================================== RENDER OBJECTS ===================================

MyTable table;
MyRoad road;
MyCar car;
std::vector<MyOrange> oranges;
std::vector<MyPacketButter> butters;
std::vector<MyCandle> candles;

// ======================================================================================

// =================================== CAMERA OBJECTS ===================================

float ratio, window_width, window_height;

const int ORTHO_CAMERA_ACTIVE = 0;
const int TOP_PERSPECTIVE_CAMERA_ACTIVE = 1;
const int CAR_PERSPECTIVE_CAMERA_ACTIVE = 2;

int activeCamera = CAR_PERSPECTIVE_CAMERA_ACTIVE;

MyCamera orthoCamera = MyCamera(MyCameraType::Ortho, 0, 90, 5.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0 });
MyCamera topPerspectiveCamera = MyCamera(MyCameraType::Perspective, 0, 90, 130.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0 });
MyCamera carCamera = MyCamera(MyCameraType::Perspective, 0, 15, 8.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0});

std::vector<MyCamera*> cameras = {
	&orthoCamera ,
	&topPerspectiveCamera,
	&carCamera,
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

MyVec3 START_POSITION = MyVec3{ 0, 0, 0 };

// =====================================================================================

// =================================== OTHER CONSTANTS ==================================
const int FPS = 60;

const int TABLE_SIZE = 250;
const int NUMBER_ORANGES = 15;

const float ORTHO_FRUSTUM_HEIGHT = (TABLE_SIZE / 2) * 1.05;

bool normalMapKey = TRUE;

const string font_name = "fonts/arial.ttf";
// ======================================================================================

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices5
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;

GLint fogActivated_uniformId;

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
GLint tex_loc0, tex_loc1, tex_loc2;
GLint texMode_uniformId;
GLuint TextureArray[3];

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

	table = MyTable(MyVec3{ 0, -0.05, 0 }, MyVec3{ TABLE_SIZE, 0.2, TABLE_SIZE });
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

	START_POSITION = MyVec3{ -0.25 * TABLE_SIZE, 0, 0 };
	car.position = START_POSITION;
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

void changeCameraSize() {

	loadIdentity(PROJECTION);

	MyCamera* currentCamera = cameras[activeCamera];
	if (currentCamera->type == MyCameraType::Perspective) { perspective(53.13f, ratio, 0.1f, 1000.0f); }
	else { ortho(-ORTHO_FRUSTUM_HEIGHT * ratio, ORTHO_FRUSTUM_HEIGHT * ratio, -ORTHO_FRUSTUM_HEIGHT, ORTHO_FRUSTUM_HEIGHT, -20, 20); }
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

// ------------------------------------------------------------
//
// Render stuff
//

// Recursive render of the Assimp Scene Graph

void aiRecursive_render(const aiScene* sc, const aiNode* nd)
{
	GLint loc;

	// Get node transformation matrix
	aiMatrix4x4 m = nd->mTransformation;
	// OpenGL matrices are column major
	m.Transpose();

	// save model matrix and apply node transformation
	pushMatrix(MODEL);

	float aux[16];
	memcpy(aux, &m, sizeof(float) * 16);
	multMatrix(MODEL, aux);

	// draw all meshes assigned to this node
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {

		
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.emissive");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.emissive);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, myMeshes[nd->mMeshes[n]].mat.shininess);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.texCount");
		glUniform1i(loc, myMeshes[nd->mMeshes[n]].mat.texCount);

		GLint texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");
		glUniform1i(texMode_uniformId, 0);

		unsigned int  diffMapCount = 0;  //read 2 diffuse textures
		
		//devido ao fragment shader suporta 2 texturas difusas simultaneas, 1 especular e 1 normal map

		glUniform1i(normalMap_loc, false);   //GLSL normalMap variable initialized to 0
		glUniform1i(specularMap_loc, false);
		glUniform1ui(diffMapCount_loc, 0);

		if(myMeshes[nd->mMeshes[n]].mat.texCount != 0)  
			for (unsigned int i = 0; i < myMeshes[nd->mMeshes[n]].mat.texCount; ++i) {
				if (myMeshes[nd->mMeshes[n]].texTypes[i] == DIFFUSE) {
					if (diffMapCount == 0) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff");
						glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i] + 3);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else if (diffMapCount == 1) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff1");
						glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i] + 3);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else printf("Only supports a Material with a maximum of 2 diffuse textures\n");
				}
				else if (myMeshes[nd->mMeshes[n]].texTypes[i] == SPECULAR) {
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitSpec");
					glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i] + 3);
					glUniform1i(specularMap_loc, true);
				}
				else if (myMeshes[nd->mMeshes[n]].texTypes[i] == NORMALS) { //Normal map
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitNormalMap");
					if (normalMapKey)
						glUniform1i(normalMap_loc, normalMapKey);
					glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i] + 3);

				}
				else printf("Texture Map not supported\n");
			}
		
		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);
		// bind VAO
		glBindVertexArray(myMeshes[nd->mMeshes[n]].vao);

		if (!shader.isProgramValid()) {
			printf("Program Not Valid!\n");
			exit(1);
		}
		// draw
		glDrawElements(myMeshes[nd->mMeshes[n]].type, myMeshes[nd->mMeshes[n]].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	// draw all children
	for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
		aiRecursive_render(sc, nd->mChildren[n]);
	}
	popMatrix(MODEL);
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

	glUniform1i(tex_loc0, 0);
	glUniform1i(tex_loc1, 1);
	glUniform1i(tex_loc2, 2);

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

	// Non Transparent Objects
	table.render(shader);
	road.render(shader);
	for (MyOrange& orange : oranges) { orange.render(shader); }
	for (MyCandle& candle : candles) { candle.render(shader); }
	car.render(shader);
	// Trasparent Objects
	glDepthMask(GL_FALSE);
	for (MyPacketButter& butter : butters) { butter.render(shader); }
	glDepthMask(GL_TRUE);
	
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
	currentCamera->updateCamera();

	// Render OBJs
	translate(MODEL, START_POSITION.x, START_POSITION.y + 2, START_POSITION.z - 10);
	aiRecursive_render(scene, scene->mRootNode);

	// Render Text
	game.renderHUD(shaderText, window_width, window_height);

	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	MyCamera *currentCamera = cameras[activeCamera];
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
			}
			break;
		case 'H':
		case 'h':
			for (int lightIndex = 0; lightIndex < NUMBER_SPOTLIGHTS; lightIndex++) {
				if (spotlights[lightIndex].state == MySpotlightState::On) spotlights[lightIndex].turnOff();
				else spotlights[lightIndex].turnOn();
			}
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

		// ================= OTHER STUFF =================

		case 'F':
		case 'f':
			fogActivated = !fogActivated;
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
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");


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

int init()
{
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
		SymbolInformation{"dead", "./materials/dead.png"}
	});

	glGenTextures(3, TextureArray);
	Texture2D_Loader(TextureArray, "./materials/roadGrass3.jpg", 0);
	Texture2D_Loader(TextureArray, "./materials/lightwood.tga", 1);
	Texture2D_Loader(TextureArray, "./materials/orange.jpg", 2);

	// Initialize Objects
	initGameObjects();

	std::string filepath; 
	
	while (true) {
		cout << "Input the directory name containing the OBJ file: ";
		cin >> model_dir;
		
		std::ostringstream oss;
		oss << model_dir << "/" << model_dir << ".obj";
		filepath = oss.str();   //path of OBJ file in the VS project

		strcat(model_dir, "/");  //directory path in the VS project

		//check if file exists
		ifstream fin(filepath.c_str());
		if (!fin.fail()) {
			fin.close();
			break;
		}
		else
			printf("Couldn't open file: %s\n", filepath.c_str());
	}
	
	//import 3D file into Assimp scene graph
	if (!Import3DFromFile(filepath))
		return(0);

	//creation of Mymesh array with VAO Geometry and Material
	myMeshes = createMeshFromAssimp(scene);

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Blending Stuff
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

}

// ------------------------------------------------------------
//
// Main function
//

int main(int argc, char **argv) {

	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA|GLUT_MULTISAMPLE);

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

