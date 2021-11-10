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

#include "spotlight.h"

MySpotlight::MySpotlight(MyVec3 positionVecTemp, std::vector<MyVec3Rotation> rotateVecTemp, std::vector<MyVec3> translationBeforeRotationTemp, float coneAngleTemp, MySpotlightState stateTemp) {

	positionVec = positionVecTemp;
	rotateVec = rotateVecTemp;
	translationBeforeRotation = translationBeforeRotationTemp;

	coneAngle = coneAngleTemp;
	state = stateTemp;
};

void MySpotlight::turnOn() { state = MySpotlightState::On; };
void MySpotlight::turnOff() { state = MySpotlightState::Off;  };

void MySpotlight::computeEyeStuff() {

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

	float directionToTranslate[4] = { 0, -0.2, -1, 0 };
	float directionTranslated[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pushMatrix(MODEL);
	multMatrixPoint(MODEL, directionToTranslate, directionTranslated);
	popMatrix(MODEL);

	direction = MyVec3{ directionTranslated[0], directionTranslated[1], directionTranslated[2] };

	popMatrix(MODEL);
}

MyVec3 MySpotlight::getPosition() { return position; };
MyVec3 MySpotlight::getDirection() { return direction; };
float MySpotlight::getConeAngle() { return coneAngle; }

int MySpotlight::getState() { return (state == MySpotlightState::On) ? 1 : 0; }