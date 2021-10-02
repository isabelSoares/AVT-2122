#ifndef CAMERA_H
#define CAMERA_H

#include "objects_geometry.h"

enum MyCameraType { Ortho, Perspective };

class MyCamera {
public:

	MyVec3 position;
	MyCameraType type;

	float alpha;
	float beta;
	float r;

	MyCamera();
	MyCamera(MyVec3 positionTemp, MyCameraType typeTemp, float alphaTemp, float betaTemp, float rTemp);
};
#endif