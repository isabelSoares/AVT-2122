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

#include "directionalLight.h"

MyDirectionalLight::MyDirectionalLight(MyVec3 directionTemp, MyDirectionalLightState stateTemp) {

	direction = directionTemp;
	state = stateTemp;
};

void MyDirectionalLight::turnOn() { state = MyDirectionalLightState::On; };
void MyDirectionalLight::turnOff() { state = MyDirectionalLightState::Off;  };

float* MyDirectionalLight::getDirection() {

	float directionW[4] = { direction.x, direction.y, direction.z, 0.0};
	return directionW;
};

int MyDirectionalLight::getState() { return (state == MyDirectionalLightState::On) ? 1 : 0; }