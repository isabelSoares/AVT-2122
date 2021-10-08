#pragma once
#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "objects_geometry.h"

enum MySpotlightState { Off, On };

class MySpotlight {
public:

	MyVec3 position;
	MySpotlightState state;

	MySpotlight();
	MySpotlight(MyVec3 positionTemp, MySpotlightState stateTemp);

	void turnOn();
	void turnOff();

	float* getPosition();
	int getState();

};
#endif