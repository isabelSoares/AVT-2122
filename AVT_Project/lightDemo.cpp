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

#include <math.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"
#include "objects_geometry.h"
#include "camera.h"
#include "spotlight.h"

using namespace std;

#define CAPTION "AVT Per Fragment Phong Lightning Demo"
int WindowHandle = 0;
int WinX = 640, WinY = 480;

unsigned int FrameCount = 0;

VSShaderLib shader;

//Vector with meshes
vector<struct MyMesh> myMeshes;
vector<struct MyVec3> myPositions;

// =================================== RENDER OBJECTS ===================================

MyTable table;
MyRoad road;
MyCar car;
std::vector<MyOrange> oranges;
MyPacketButter butter;

// ======================================================================================

// =================================== CAMERA OBJECTS ===================================

float ratio;

const int ORTHO_CAMERA_ACTIVE = 0;
const int TOP_PERSPECTIVE_CAMERA_ACTIVE = 1;
const int CAR_PERSPECTIVE_CAMERA_ACTIVE = 2;

int activeCamera = CAR_PERSPECTIVE_CAMERA_ACTIVE;

MyCamera orthoCamera = MyCamera(MyCameraType::Ortho, 0, 90, 5.0f, MyVec3{ 0, 5, 0 }, MyVec3{ 0, 0, 0 });
MyCamera topPerspectiveCamera = MyCamera(MyCameraType::Perspective, 0, 90, 20.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0 });
MyCamera carCamera = MyCamera(MyCameraType::Perspective, 0, 15, 8.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0});

std::vector<MyCamera*> cameras = {
	&orthoCamera ,
	&topPerspectiveCamera,
	&carCamera,
};

// ======================================================================================

// =================================== LIGHT OBJECTS ===================================

const int NUMBER_SPOTLIGHTS = 3;

MySpotlight spotlights[NUMBER_SPOTLIGHTS] = {
	MySpotlight(MyVec3{10.0f, 6.0f, 2.0f}, MySpotlightState::Off),
	MySpotlight(MyVec3{-10.0f, 6.0f, 2.0f}, MySpotlightState::On),
	MySpotlight(MyVec3{0.0f, 20.0f, 2.0f}, MySpotlightState::On),
};

// =====================================================================================

// =================================== OTHER OBJECTS ===================================

int gameTime = 0;

// =====================================================================================

// =================================== OTHER CONSTANTS ==================================
const int FPS = 60;

const int TABLE_SIZE = 500;
const int NUMBER_ORANGES = 75;

const float ORTHO_FRUSTUM_HEIGHT = (TABLE_SIZE / 2) * 1.05;
// ======================================================================================

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint lPos_uniformId;
GLint lState_uniformId;
	

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Frame counting and FPS computation
long myTime,timebase = 0,frame = 0;
char s[32];

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
    FrameCount = 0;
    glutTimerFunc(1000, timer, 0);

	// Update GameTime
	gameTime = gameTime + 1;
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
	changeCameraSize();
}


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

	//send the light position in eye coordinates

	//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 

	
	float* res_pos = (float*) malloc(NUMBER_SPOTLIGHTS * 4 * sizeof(float));
	int* res_state = (int*)malloc(NUMBER_SPOTLIGHTS * sizeof(int));
	for (int lightIndex = 0; lightIndex < NUMBER_SPOTLIGHTS; lightIndex++) {

		MySpotlight* currentSpotlight = &spotlights[lightIndex];

		float * lightOnePos = currentSpotlight->getPosition();
		multMatrixPoint(VIEW, lightOnePos, res_pos + lightIndex * 4);   //lightPos definido em World Coord so is converted to eye space

		int state = currentSpotlight->getState();
		memcpy(res_state + lightIndex, &state, sizeof(int));
	}
	glUniform4fv(lPos_uniformId, NUMBER_SPOTLIGHTS, res_pos);
	glUniform1iv(lState_uniformId, NUMBER_SPOTLIGHTS, res_state);
	

	// ================================ Check Position Car ================================
	for (int i = 0; i < NUMBER_ORANGES; i++) {

		MyVec3 currentPosition = oranges[i].getPosition();
		if (abs(currentPosition.x) > TABLE_SIZE / 2 || abs(currentPosition.z) > TABLE_SIZE / 2) {

			float orangeX = rand() % TABLE_SIZE - TABLE_SIZE / 2;
			float orangeY = rand() % TABLE_SIZE - TABLE_SIZE / 2;
			oranges[i] = MyOrange(MyVec3{ orangeX, 2.0, orangeY }, MyVec3{ 1, 1, 1 }, float(gameTime / 30 + 1));
		}
	}
	

	// ====================================================================================

	table.render(shader);
	road.render(shader);
	car.render(shader);
	for (MyOrange& orange : oranges) { orange.render(shader); }
	butter.render(shader);

	car.tick();
	for (MyOrange& orange : oranges) { orange.tick(); }

	// Update Car Camera
	MyVec3 carPosition = car.getPosition();
	carCamera.translation.x = carPosition.x;
	carCamera.translation.y = carPosition.y;
	carCamera.translation.z = carPosition.z;
	carCamera.lookAtPosition = carPosition;
	currentCamera->updateCamera();

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
		case 'N':
		case 'n':
			break;
		case 'C':
		case 'c':
			break;
		case 'H':
		case 'h':
			break;
		// ==============================================

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
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlightGouraud.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlightGouraud.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0,"colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	//glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_positions");
	lState_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_states");
	
	// printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());
	printf("InfoLog for Per Fragment Gouraud Shader\n%s\n\n", shader.getAllInfoLogs().c_str());
	
	return(shader.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

void init()
{
	for (MyCamera* camera : cameras) {
		// set the camera position based on its spherical coordinates
		camera->position.x = camera->r * sin(camera->alpha * 3.14f / 180.0f) * cos(camera->beta * 3.14f / 180.0f);
		camera->position.z = camera->r * cos(camera->alpha * 3.14f / 180.0f) * cos(camera->beta * 3.14f / 180.0f);
		camera->position.y = camera->r * sin(camera->beta * 3.14f / 180.0f);
	}

	table = MyTable(MyVec3{ 0, -0.2, 0 }, MyVec3{TABLE_SIZE, 0.2, TABLE_SIZE});
	road = MyRoad(MyVec3{ 0, -0.2, 0 }, MyVec3{ 5, 0.3, TABLE_SIZE + 0.2 });
	car = MyCar(MyVec3{ 0, 0.75, 0 }, MyVec3{ 1, 1, 1.7 });
	oranges = { MyOrange(MyVec3{0, 2.0, -10.0}, MyVec3{1, 1, 1}, 0) };
	for (int i = 0; i < NUMBER_ORANGES; i++) {
		float orangeX = rand() % TABLE_SIZE - TABLE_SIZE / 2;
		float orangeY = rand() % TABLE_SIZE - TABLE_SIZE / 2;
		oranges.push_back(MyOrange(MyVec3{ orangeX, 2.0, orangeY }, MyVec3{ 1, 1, 1 }, float(gameTime / 30 + 1)));
	}
	butter = MyPacketButter(MyVec3{ 5.0f, 0.2f, 5.0f }, MyVec3{1.0f, 0.4f, 0.5f});

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

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

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);

}

