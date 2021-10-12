#pragma once
#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

enum class MySpotlightState { Off, On };

class MySpotlight {
public:

	// Eye Coordinates Attributes
	MyVec3 position = MyVec3{ 0, 0, 0 };
	MyVec3 direction = MyVec3{ 0, 0, 0 };
	// Light Attributes
	float coneAngle;
	MySpotlightState state;
	// Object Attributes
	MyVec3 positionVec;
	std::vector<MyVec3Rotation> rotateVec;
	std::vector<MyVec3> translationBeforeRotation = {};

	MySpotlight();
	MySpotlight(MyVec3 positionVecTemp, std::vector<MyVec3Rotation> rotateVecTemp, std::vector<MyVec3> translationBeforeRotationTemp, float coneAngleTemp, MySpotlightState stateTemp);

	void turnOn();
	void turnOff();

	void computeEyeStuff();

	MyVec3 getPosition();
	MyVec3 getDirection();
	float getConeAngle();
	int getState();

};
#endif