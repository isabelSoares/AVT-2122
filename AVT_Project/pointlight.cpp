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

MyPointlight::MyPointlight(MyVec3 positionTemp, MyPointlightState stateTemp) {

	position = positionTemp;
	state = stateTemp;
};

void MyPointlight::turnOn() { state = MyPointlightState::On; };
void MyPointlight::turnOff() { state = MyPointlightState::Off;  };

float* MyPointlight::getPosition() {

	float positionW[4] = { position.x, position.y, position.z, 1.0f };
	return positionW;
};

int MyPointlight::getState() { return (state == MyPointlightState::On) ? 1 : 0; }