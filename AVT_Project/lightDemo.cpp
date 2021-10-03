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

// ======================================================================================

// =================================== CAMERA OBJECTS ===================================

float ratio;

const int ORTHO_CAMERA_ACTIVE = 0;
const int TOP_PERSPECTIVE_CAMERA_ACTIVE = 1;
const int CAR_PERSPECTIVE_CAMERA_ACTIVE = 2;

int activeCamera = CAR_PERSPECTIVE_CAMERA_ACTIVE;

MyCamera orthoCamera = MyCamera(MyCameraType::Ortho, 39.0f, 51.0f, 10.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0 });
MyCamera topPerspectiveCamera = MyCamera(MyCameraType::Perspective, 0, 90, 20.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0 });
MyCamera carCamera = MyCamera(MyCameraType::Perspective, 0, 15, 8.0f, MyVec3{ 0, 0, 0 }, MyVec3{ 0, 0, 0});

std::vector<MyCamera*> cameras = {
	&orthoCamera,
	&topPerspectiveCamera,
	&carCamera,
};

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
	

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Frame counting and FPS computation
long myTime,timebase = 0,frame = 0;
char s[32];
float lightPos[4] = {4.0f, 6.0f, 2.0f, 1.0f};

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
    FrameCount = 0;
    glutTimerFunc(1000, timer, 0);
}

void refresh(int value)
{

	glutPostRedisplay();
	glutTimerFunc(1000 / 60, refresh, 0);
}

void changeCameraSize() {

	loadIdentity(PROJECTION);

	MyCamera* currentCamera = cameras[activeCamera];
	if (currentCamera->type == MyCameraType::Perspective) { perspective(53.13f, ratio, 0.1f, 1000.0f); }
	else { ortho(-20, 20, -20, 20); }
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

	float res[4];
	multMatrixPoint(VIEW, lightPos,res);   //lightPos definido em World Coord so is converted to eye space
	glUniform4fv(lPos_uniformId, 1, res);

	/*
	for (int objId = 0; objId < myMeshes.size(); objId++) {
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, myMeshes[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, myMeshes[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, myMeshes[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, myMeshes[objId].mat.shininess);
		pushMatrix(MODEL);
		translate(MODEL, myPositions[objId].x, myPositions[objId].y, myPositions[objId].z);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(myMeshes[objId].vao);
			
		if (!shader.isProgramValid()) {
			printf("Program Not Valid!\n");
			exit(1);	
		}
		glDrawElements(myMeshes[objId].type, myMeshes[objId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}
	*/

	table.render(shader);
	road.render(shader);
	car.render(shader);

	car.tick();
	car.stop();

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
		// ==============================================

		case 'c': 
			printf("Camera Spherical Coordinates (%f, %f, %f)\n", currentCamera->alpha, currentCamera->beta, currentCamera->r);
			break;
		case 'm': glEnable(GL_MULTISAMPLE); break;
		case 'n': glDisable(GL_MULTISAMPLE); break;
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
	}

//  uncomment this if not using an idle or refresh func
//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	MyCamera *currentCamera = cameras[activeCamera];
	currentCamera->r += direction * 0.1f;
	if (currentCamera->r < 0.1f)
		currentCamera->r = 0.1f;

	currentCamera->updateCamera();

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
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");
	
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


	float amb[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float diff[] = { 0.8f, 0.6f, 0.4f, 1.0f };
	float spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;

	MyMesh amesh;

	// create geometry and VAO of the pawn
	amesh = createPawn();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	myPositions.push_back(MyVec3{ 0, 0, 0 });


	// create geometry and VAO of the sphere
	amesh = createSphere(1.0f, 20);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	myPositions.push_back(MyVec3{ 2, 0, 0 });

	float amb1[] = { 0.3f, 0.0f, 0.0f, 1.0f };
	float diff1[] = { 0.8f, 0.1f, 0.1f, 1.0f };
	float spec1[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	shininess = 500.0;

	// create geometry and VAO of the cylinder
	amesh = createCylinder(1.5f, 0.5f, 20);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	myPositions.push_back(MyVec3{ 4, 0, 0 });

	// create geometry and VAO of the 
	amesh = createCone(1.5f, 0.5f, 20);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	myPositions.push_back(MyVec3{ 6, 0, 0 });

	// create geometry and VAO of the 
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	myPositions.push_back(MyVec3{ 2, 0, 2 });

	table = MyTable(MyVec3{ 0, -0.2, 0 }, MyVec3{20, 0.2, 20});
	road = MyRoad(MyVec3{ 0, -0.2, 0 }, MyVec3{ 5, 0.3, 20.2 });
	car = MyCar(MyVec3{ 0, 0.75, 0 }, MyVec3{ 1, 1, 1.7 });

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

