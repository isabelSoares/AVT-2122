#ifndef OBJECTS_GEOMETRY_H
#define OBJECTS_GEOMETRY_H

#define O_PI       3.14159265358979323846f

#include "spotlight.h"
#include "pointlight.h"

enum class MyTextureOption { None, One, Two, Multitexturing, Orange };

class MyObject {

public:
	MyMesh mesh;
	MyVec3 positionVec;
	MyVec3 scaleVec;
	std::vector<MyVec3Rotation> rotateVec;
	std::vector<MyVec3> translationBeforeRotation = {};

	MyTextureOption textureOption = MyTextureOption::None;


	// Display
	VSShaderLib shader;

	MyObject();
	MyObject(MyMesh meshTemp, MyVec3 positionTemp, MyVec3 scaleTemp, std::vector<MyVec3Rotation> rotateVec);

	void render(VSShaderLib shader);
	MyVec3 calculatePointInWorld(MyVec3 point);
};

class MyTable {		
public:

	// ============ Table Objects ============
	MyObject tableTop;

	MyTable();
	MyTable(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp);

	void render(VSShaderLib shader);
};

class MyCheerio {
public:
	// ============ Cheerio Objects ===============
	MyObject cheerio;
	// ============ Cheerio Attributes ============
	float FRICTION_COEFICIENT = 0.02;
	// ============ Cheerio Attributes ============
	MyVec3 position;
	MyVec3 direction = MyVec3{ 0, 0, -1 };
	float velocity = 0.0f;

	float innerCheerioRadius;
	float outterCheerioRadius;

	MyCheerio();
	MyCheerio(MyVec3 positionTemp, float innerCheerioRadiusTemp, float outterCheerioRadiusTemp);

	void render(VSShaderLib shader);

	MyVec3 getPosition();
	std::vector<MyVec3> getBoundRect();

	void tick();
};

class MyRoad {
public:

	// ============ Road Objects ============
	std::vector<MyCheerio> cheerios = {};
	// ============ Road Object Attributes ============
	float ROAD_HEIGHT = 0.3;
	MyVec3 MARGIN_SCALING_VARIATION = MyVec3{ 0.2, 2, 1.01 };
	// ============ Road Attributes ============
	MyVec3 position;
	MyVec3 scaling = MyVec3{ 1, 1, 1 };
	MyVec3 direction = MyVec3{ 0, 0, -1 };

	MyRoad();
	MyRoad(MyVec3 positionTemp, float width, float sideDistance, float sizeStraight, float tableWidthTemp, float tableLengthTemp, float cheerios_distance, float innerCheerioRadiusTemp, float outterCheerioRadiusTemp);

	std::vector<MyCheerio> getCheerios();

	void render(VSShaderLib shader);

	void tick();
};

class MyCar {
public:

	// ============ Car Objects ===============
	MyObject mainBlock;
	std::vector<MyObject> wheels;

	std::vector<MySpotlight*> spotlights;
	// ============ Car Object Attributes ============
	MyVec3 MAIN_BLOCK_SCALING_VARIATION = MyVec3{ 1, 1, 1.5 };
	MyVec3Rotation MAIN_BLOCK_ROTATION_VARIATION = MyVec3Rotation{-90, 0, 1, 0};
	MyVec3 MAIN_BLOCK_TRANSLATION_VARIATION = MyVec3{ 0, 0.95, 0};
	// =====
	MyVec3 WHEEL_SCALING_VARIATION = MyVec3{ 1, 1, 1 };
	MyVec3Rotation WHEEL_ROTATION_VARIATION = MyVec3Rotation{ 90, 0, 0, 1 };
	MyVec3 WHEELS_TRANSLATION_VARIATION[4] = { MyVec3{ 0.55, 0.58, 0.8 }, MyVec3{ 0.55, 0.58, -0.8 }, MyVec3{ 0.55, -0.58, 0.8 }, MyVec3{ 0.55, -0.58, -0.8 } };
	// =====
	MyVec3Rotation SPOTLIGHT_ROTATION_VARIATION = MyVec3Rotation{ -5, 1, 0, 0 };
	MyVec3 SPOTLIGHTS_TRANSLATION_VARIATION[2] = { MyVec3{ 0.49, 1.80, -1}, MyVec3{ -0.49, 1.80, -1} };
	// ============ Car Attributes ============
	static float MAX_VELOCITY;
	static float START_ACCELERATION;
	static float STOP_ACCELERATION;
	static float FRICTION_COEFICIENT;
	static float ANGLE_ROTATION_VELOCITY;
	static float MAX_WHEEL_ANGLE;
	static float FRICTION_ROTATION_COEFICIENT;
	// ============ Car Attributes ============
	MyVec3 position;
	MyVec3 scaling = MyVec3{1, 1, 1};
	MyVec3 direction = MyVec3{ 0, 0, -1 };
	float velocity = 0.0f;
	float acceleration = 0.0f;
	float rotationVelocity = 0.0f;
	float rotationWheelAngle = 0.0f;

	MyCar();
	MyCar(MyVec3 positionTemp, std::vector<MySpotlight*> spotlightsTemp);

	void render(VSShaderLib shader);
	void updateObjects();

	MyVec3 getPosition();
	float getDirectionDegrees();
	std::vector<MyVec3> getBoundRect();

	void tick();
	void collisionStop();

	void forward();
	void backward();
	void turnLeft();
	void turnRight();
	void stop();
};

class MyOrange {
public:
	// ============ Orange Objects ===============
	MyObject orange;
	// ============ Orange Attributes ============
	static float ANGLE_ROTATION_VELOCITY;
	// ============ Orange Attributes ============
	MyVec3 direction = MyVec3{ 0, 0, -1 };
	float velocity = 0.0f;
	float angleRotated = 0.0f;

	MyOrange();
	MyOrange(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp, float maxVelocity);

	void render(VSShaderLib shader);
	MyVec3 getPosition();
	std::vector<MyVec3> getBoundRect();

	void tick();
};

class MyPacketButter {
public:
	// ============ Butter Packet Objects ===============
	MyObject butter;
	// ============ Butter Attributes ============
	float FRICTION_COEFICIENT = 0.02;
	// ============ Butter Attributes ============
	MyVec3 position;
	MyVec3 scaling = MyVec3{ 1, 1, 1 };
	MyVec3 direction = MyVec3{ 0, 0, -1 };
	float velocity = 0.0f;

	MyPacketButter();
	MyPacketButter(MyVec3 positionTemp, MyVec3 scaleTemp);

	MyVec3 getPosition();
	std::vector<MyVec3> getBoundRect();

	void render(VSShaderLib shader);

	void tick();
};

class MyCandle {
public:
	// ============ Candle Objects ===============
	MyObject candle;

	MyPointlight* light;
	// ============ Candle Attributes ============
	MyVec3 position;
	MyVec3 scaling = MyVec3{ 1, 1, 1};

	float height;
	float radius;


	MyCandle();
	MyCandle(MyVec3 positionTemp, float heightTemp, float radiusTemp, MyPointlight* lightTemp);

	void render(VSShaderLib shader);
	MyVec3 getPosition();

	void tick();
};
#endif