#ifndef OBJECTS_GEOMETRY_H
#define OBJECTS_GEOMETRY_H

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

	// ============ Block Objects ============
	MyObject mainBlock;
	std::vector<MyObject> wheels;

	MyCar();
	MyCar(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp);

	void render(VSShaderLib shader);
};
#endif