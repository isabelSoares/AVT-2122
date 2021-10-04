#ifndef OBJECTS_GEOMETRY_H
#define OBJECTS_GEOMETRY_H

#define O_PI       3.14159265358979323846f

struct MyVec3 {
	float x;
	float y;
	float z;
};

struct MyVec3Rotation {
	float angle;
	float x;
	float y;
	float z;
};

class MyObject {

public:
	MyMesh mesh;
	MyVec3 positionVec;
	MyVec3 scaleVec;
	MyVec3Rotation rotateVec;


	// Display
	VSShaderLib shader;

	MyObject();
	MyObject(MyMesh meshTemp, MyVec3 positionTemp, MyVec3 scaleTemp, MyVec3Rotation rotateVec);

	void render(VSShaderLib shader);
};

class MyTable {
public:

	// ============ Table Objects ============
	MyObject tableTop;

	MyTable();
	MyTable(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp);

	void render(VSShaderLib shader);
};

class MyRoad {
public:

	// ============ Road Objects ============
	MyObject mainRoad;
	MyObject leftMargin;
	MyObject rightMargin;

	MyRoad();
	MyRoad(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp);

	void render(VSShaderLib shader);
};

class MyCar {
public:

	// ============ Car Objects ===============
	MyObject mainBlock;
	std::vector<MyObject> wheels;
	// ============ Car Attributes ============
	static float MAX_VELOCITY;
	static float START_ACCELERATION;
	static float STOP_ACCELERATION;
	static float FRICTION_COEFICIENT;
	static float ANGLE_ROTATION_VELOCITY;
	// ============ Car Attributes ============
	MyVec3 direction = MyVec3{ 0, 0, -1 };
	float velocity = 0.0f;
	float acceleration = 0.0f;
	float rotationVelocity = 0.0f;
	float rotationWheelAngle = 0.0f;

	MyCar();
	MyCar(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp);

	void render(VSShaderLib shader);
	MyVec3 getPosition();

	void tick();
	void forward();
	void backward();
	void turnLeft();
	void turnRight();
	void stop();
};
#endif