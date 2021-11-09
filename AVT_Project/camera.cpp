#include <string>
#include <assert.h>
#include <stdlib.h>
#include <vector>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>

// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"
#include "VSShaderlib.h"
#include "objects_geometry.h"

#include "camera.h"

MyCamera::MyCamera(MyCameraType typeTemp, float alphaTemp, float betaTemp, float rTemp, MyVec3 translationTemp, MyVec3 lookAtPositionTemp, float fogFactorTemp) {

	position = MyVec3 { 0, 0, 0 };
	type = typeTemp;

	alpha = alphaTemp;
	beta = betaTemp;
	r = rTemp;

	translation = translationTemp;
	lookAtPosition = lookAtPositionTemp;

	fogFactor = fogFactorTemp;
};

void MyCamera::updateCamera() {
	updateCameraWithAux(alpha, beta, r);
};

void MyCamera::updateCameraWithAux(float alphaAux, float betaAux, float rAux) {
	position.x = rAux * sin((alphaAux + rotationDegrees) * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f) + translation.x;
	position.z = rAux * cos((alphaAux  + rotationDegrees) * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f) + translation.z;
	position.y = rAux * sin(betaAux * 3.14f / 180.0f) + translation.y;
};