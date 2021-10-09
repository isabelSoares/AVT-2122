#pragma once
#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "objects_geometry.h"

enum class MySpotlightState { Off, On };

class MySpotlight {
public:

	MyVec3 position;
	float coneAngle;
	MySpotlightState state;

	MySpotlight();
	MySpotlight(MyVec3 positionTemp, float coneAngleTemp, MySpotlightState stateTemp);

	void turnOn();
	void turnOff();

	float* getPosition();
	float getConeAngle();
	int getState();

};
#endif