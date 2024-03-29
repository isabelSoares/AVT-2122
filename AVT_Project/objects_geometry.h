#ifndef OBJECTS_GEOMETRY_H
#define OBJECTS_GEOMETRY_H

#define O_PI       3.14159265358979323846f

// assimp include files. These three are usually needed.
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/scene.h"

#include "spotlight.h"
#include "pointlight.h"

enum class MyTextureOption { None, One, Two, Multitexturing, Orange, Flare, Tree, WaterParticle, SkyBox, CubeReflector, Candle, Flame };
enum class MyBumpMapOption { None, Candle };

class MyObject {

public:
	MyMesh mesh;
	MyVec3 positionVec;
	MyVec3 scaleVec;
	std::vector<MyVec3Rotation> rotateVec;
	std::vector<MyVec3> translationBeforeRotation = {};

	MyTextureOption textureOption = MyTextureOption::None;
	bool reflectedPerFrag = false;
	MyBumpMapOption bumpmapOption = MyBumpMapOption::None;

	MyObject();
	MyObject(MyMesh meshTemp, MyVec3 positionTemp, MyVec3 scaleTemp, std::vector<MyVec3Rotation> rotateVec);

	void render(VSShaderLib& shader);
	MyVec3 calculatePointInWorld(MyVec3 point);
};

class MyAssimpObject {
public:
	std::vector<MyMesh> meshes;
	MyVec3 positionVec;
	MyVec3 scaleVec;
	std::vector<MyVec3Rotation> rotateVec;
	std::vector<MyVec3> translationBeforeRotation = {};

	MyAssimpObject();
	MyAssimpObject(std::string modelDir, MyVec3 positionTemp, MyVec3 scaleTemp, std::vector<MyVec3Rotation> rotateVec);

	void render(VSShaderLib& shader, const aiScene* sc, const aiNode* nd);
};

class MyTable {		
public:

	// ============ Table Objects ============
	MyObject tableTop;

	MyTable();
	MyTable(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp);

	void render(VSShaderLib& shader, bool reflectionPart);
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

	void render(VSShaderLib& shader);

	MyVec3 getPosition();
	std::vector<MyVec3> getBoundRect();

	void tick(float elapsedTime);
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

	void render(VSShaderLib& shader);

	void tick(float elapsedTime);
};

class MyCar {
public:

	// ============ Car Objects ===============
	MyObject mainBlock;
	std::vector<MyObject> wheels;

	MyAssimpObject carObj;

	std::vector<MySpotlight*> spotlights;
	// ============ Car Object Attributes ============
	// MyVec3 MAIN_BLOCK_SCALING_VARIATION = MyVec3{ 1, 1, 1.5 };
	MyVec3 MAIN_BLOCK_SCALING_VARIATION = MyVec3{ 2.4, 1, 4.4 };
	MyVec3Rotation MAIN_BLOCK_ROTATION_VARIATION = MyVec3Rotation{-90, 0, 1, 0};
	MyVec3 MAIN_BLOCK_TRANSLATION_VARIATION = MyVec3{ 0, 0.95, 0};
	// =====
	MyVec3 WHEEL_SCALING_VARIATION = MyVec3{ 1, 1, 1 };
	MyVec3Rotation WHEEL_ROTATION_VARIATION = MyVec3Rotation{ 90, 0, 0, 1 };
	MyVec3 WHEELS_TRANSLATION_VARIATION[4] = { MyVec3{ 0.55, 0.58, 0.8 }, MyVec3{ 0.55, 0.58, -0.8 }, MyVec3{ 0.55, -0.58, 0.8 }, MyVec3{ 0.55, -0.58, -0.8 } };
	// =====
	MyVec3Rotation SPOTLIGHT_ROTATION_VARIATION = MyVec3Rotation{ -5, 1, 0, 0 };
	MyVec3 SPOTLIGHTS_TRANSLATION_VARIATION[2] = { MyVec3{ 0.49, 1.80, -1}, MyVec3{ -0.49, 1.80, -1} };
	// =====
	MyVec3 OBJ_SCALING_VARIATION = MyVec3{ 0.07, 0.07, 0.07 };
	MyVec3Rotation OBJ_ROTATION_VARIATION = MyVec3Rotation{ 180, 0, 1, 0 };
	MyVec3 OBJ_TRANSLATION_VARIATION = MyVec3{  327.1, 0.6, - 272.8};
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

	void render(VSShaderLib& shader);
	void updateObjects();

	MyVec3 getPosition();
	float getDirectionDegrees();
	std::vector<MyVec3> getBoundRect();

	void tick(float elapsedTime);
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

	void render(VSShaderLib& shader);
	MyVec3 getPosition();
	std::vector<MyVec3> getBoundRect();

	void tick(float elapsedTime);
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

	void render(VSShaderLib& shader);

	void tick(float elapsedTime);
};

class MyCandle {
public:
	// ============ Candle Objects ===============
	MyObject candle;
	MyObject flame;

	MyPointlight* light;
	// ============ Candle Attributes ============
	MyVec3 position;
	MyVec3 scaling = MyVec3{ 1, 1, 1};

	float height;
	float radius;


	MyCandle();
	MyCandle(MyVec3 positionTemp, float heightTemp, float radiusTemp, MyPointlight* lightTemp);

	void render(VSShaderLib& shader, bool shadowMode);
	void update(MyVec3 camPosition);

	MyVec3 getPosition();
};

class MyBillboardTree {
public:
	// ============ Billboard Tree Objects ===============
	MyObject billboardTree;
	// ============ Billboard Tree Attributes ============
	MyVec3 position;
	MyVec3 scaling = MyVec3{ 1, 1, 1};

	float size;


	MyBillboardTree();
	MyBillboardTree(MyVec3 positionTemp, float sizeTemp);

	void render(VSShaderLib& shader);
	void update(MyVec3 camPosition);
};

class MyWaterParticle {
public:
	// ============ Water Particle Objects ===============
	MyObject particle;
	// ============ Water Particle Attributes ============
	MyVec3 position;
	MyVec3 velocity;
	MyVec3 accelaration;

	float lifespan = 1.0f;
	float fade;

	MyVec3 scaling = MyVec3{ 1, 1, 1 };

	float size;


	MyWaterParticle();
	MyWaterParticle(MyVec3 positionTemp, MyVec3 velocityTemp, MyVec3 accelarationTemp, float fadeTemp, float sizeTemp);

	void render(VSShaderLib& shader);
	void tick(float elapsedTime);
	void update(MyVec3 camPosition);

	bool isDead();
	void revive(MyVec3 positionTemp, MyVec3 velocityTemp, MyVec3 accelarationTemp, float fadeTemp);
};

class MySkyBox {
public:
	// ============ SkyBox Objects ===============
	MyObject skyBox;
	// ============ SkyBox Attributes ============
	// ============ SkyBox Attributes ============

	MySkyBox();
	MySkyBox(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp);

	void render(VSShaderLib& shader);
};

class MyCubeReflector {
public:
	// ============ Cube Reflector Objects ===============
	MyObject cube;
	// ============ Cube Reflector Attributes ============
	// ============ Cube Reflector Attributes ============

	MyCubeReflector();
	MyCubeReflector(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp);

	void render(VSShaderLib& shader);
};

class MyPuddle {
public:
	// ============ Cube Reflector Objects ===============
	MyObject puddle;
	// ============ Cube Reflector Attributes ============
	// ============ Cube Reflector Attributes ============

	MyPuddle();
	MyPuddle(MyVec3 initialPositionTemp, float heightTemp, float radiusTemp);

	void render(VSShaderLib& shader);
};
#endif