#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "objects_geometry.h"

enum class MyDirectionalLightState { Off, On };

class MyDirectionalLight {
public:

	MyVec3 direction;
	MyDirectionalLightState state;

	MyDirectionalLight();
	MyDirectionalLight(MyVec3 directionTemp, MyDirectionalLightState stateTemp);

	void turnOn();
	void turnOff();

	MyVec3 getDirection();
	int getState();
};
#endif