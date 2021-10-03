#ifndef CAMERA_H
#define CAMERA_H

#include "objects_geometry.h"

enum MyCameraType { Ortho, Perspective };

class MyCamera {
public:

	MyCameraType type;

	MyVec3 position;
	float alpha;
	float beta;
	float r;

	MyVec3 lookAtPosition;
	MyVec3 translation;

	MyCamera();
	MyCamera(MyCameraType typeTemp, float alphaTemp, float betaTemp, float rTemp, MyVec3 translationTemp, MyVec3 lookAtPositionTemp);

	void updateCamera();
	void updateCameraWithAux(float alphaAux, float betaAux, float rAux);
};
#endif