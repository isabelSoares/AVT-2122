#pragma once
#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "objects_geometry.h"

enum class MyPointlightState { Off, On };

class MyPointlight {
public:

	// Eye Coordinates Attributes
	MyVec3 position = MyVec3{ 0, 0, 0 };
	// Light Attributes
	MyPointlightState state;
	// Object Attributes
	MyVec3 positionVec;
	std::vector<MyVec3Rotation> rotateVec;
	std::vector<MyVec3> translationBeforeRotation = {};

	MyPointlight();
	MyPointlight(MyVec3 positionVecTemp, std::vector<MyVec3Rotation> rotateVecTemp, std::vector<MyVec3> translationBeforeRotationTemp, MyPointlightState stateTemp);

	void turnOn();
	void turnOff();

	void computeEyeStuff();

	MyVec3 getPosition();
	int getState();

};
#endif