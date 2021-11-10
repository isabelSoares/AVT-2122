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

#include "pointlight.h"

MyPointlight::MyPointlight(MyVec3 positionVecTemp, std::vector<MyVec3Rotation> rotateVecTemp, std::vector<MyVec3> translationBeforeRotationTemp, MyPointlightState stateTemp) {

	positionVec = positionVecTemp;
	rotateVec = rotateVecTemp;
	translationBeforeRotation = translationBeforeRotationTemp;

	state = stateTemp;
};

void MyPointlight::turnOn() { state = MyPointlightState::On; };
void MyPointlight::turnOff() { state = MyPointlightState::Off;  };

void MyPointlight::computeEyeStuff() {

	pushMatrix(MODEL);

	translate(MODEL, positionVec.x, positionVec.y, positionVec.z);
	for (MyVec3Rotation rotation : rotateVec) { rotate(MODEL, rotation.angle, rotation.x, rotation.y, rotation.z); }
	for (MyVec3 translateBefore : translationBeforeRotation) { translate(MODEL, translateBefore.x, translateBefore.y, translateBefore.z); }

	float positionToTranslate[4] = { 0, 0, 0, 1 };
	float positionTranslated[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pushMatrix(MODEL);
	multMatrixPoint(MODEL, positionToTranslate, positionTranslated);
	popMatrix(MODEL);

	position = MyVec3{ positionTranslated[0], positionTranslated[1], positionTranslated[2] };

	popMatrix(MODEL);
}

MyVec3 MyPointlight::getPosition() { return position; };
int MyPointlight::getState() { return (state == MyPointlightState::On) ? 1 : 0; }