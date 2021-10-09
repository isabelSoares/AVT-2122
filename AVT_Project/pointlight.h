#pragma once
#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "objects_geometry.h"

enum class MyPointlightState { Off, On };

class MyPointlight {
public:

	MyVec3 position;
	MyPointlightState state;

	MyPointlight();
	MyPointlight(MyVec3 positionTemp, MyPointlightState stateTemp);

	void turnOn();
	void turnOff();

	float* getPosition();
	int getState();

};
#endif