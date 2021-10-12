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

MyVec3 MyDirectionalLight::getDirection() { return direction; };
int MyDirectionalLight::getState() { return (state == MyDirectionalLightState::On) ? 1 : 0; }