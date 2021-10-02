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

MyCamera::MyCamera(MyVec3 positionTemp, MyCameraType typeTemp, float alphaTemp, float betaTemp, float rTemp) {

	position = positionTemp;
	type = typeTemp;

	alpha = alphaTemp;
	beta = betaTemp;
	r = rTemp;
};