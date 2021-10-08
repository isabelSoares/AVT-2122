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

#include "spotlight.h"

MySpotlight::MySpotlight(MyVec3 positionTemp, MySpotlightState stateTemp) {

	position = positionTemp;
	state = stateTemp;
};

void MySpotlight::turnOn() { state = MySpotlightState::On; };
void MySpotlight::turnOff() { state = MySpotlightState::Off;  };

float* MySpotlight::getPosition() {

	float positionW[4] = { position.x, position.y, position.z, 1.0f };
	return positionW;
};

int MySpotlight::getState() { return (state == MySpotlightState::On) ? 1 : 0; }