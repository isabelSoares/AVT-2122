#pragma once
#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

enum class MySpotlightState { Off, On };

class MySpotlight {
public:

	MyVec3 position;
	MyVec3 direction;
	float coneAngle;
	MySpotlightState state;

	MySpotlight();
	MySpotlight(MyVec3 positionTemp,MyVec3 directionTemp, float coneAngleTemp, MySpotlightState stateTemp);

	void turnOn();
	void turnOff();

	float* getPosition();
	float* getDirection();
	float getConeAngle();
	int getState();

};
#endif