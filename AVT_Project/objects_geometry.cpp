#include <string>
#include <assert.h>
#include <stdlib.h>
#include <vector>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>

// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"

#include "VSShaderlib.h"

#include "objects_geometry.h"

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

MyObject::MyObject() {}
MyObject::MyObject(MyMesh meshTemp, MyVec3 positionTemp, MyVec3 scaleTemp, std::vector<MyVec3Rotation> rotateTemp) {
	mesh = meshTemp;
	positionVec = positionTemp;
	scaleVec = scaleTemp;
	rotateVec = rotateTemp;

}

void MyObject::render(VSShaderLib shader) {

	GLint loc;

	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh.mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh.mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh.mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh.mat.shininess);
	pushMatrix(MODEL);
	translate(MODEL, positionVec.x, positionVec.y, positionVec.z);
	scale(MODEL, scaleVec.x, scaleVec.y, scaleVec.z);
	for (MyVec3Rotation rotation : rotateVec) { rotate(MODEL, rotation.angle, rotation.x, rotation.y, rotation.z);  }

	GLint pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	GLint vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	GLint normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	GLint lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glBindVertexArray(mesh.vao);

	if (!shader.isProgramValid()) {
		printf("Program Not Valid!\n");
		exit(1);
	}
	glDrawElements(mesh.type, mesh.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);
}

MyTable::MyTable() {}
MyTable::MyTable(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp) {

	MyMesh tableTopMesh = createCube();

	float amb[] = { 0.2f, 0.50f, 0.1f, 1.0f };
	float diff[] = { 0.8f, 0.1f, 0.4f, 1.0f };
	float spec[] = { 0.8f, 0.2f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;

	memcpy(tableTopMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(tableTopMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(tableTopMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(tableTopMesh.mat.emissive, emissive, 4 * sizeof(float));
	tableTopMesh.mat.shininess = shininess;
	tableTopMesh.mat.texCount = texcount;

	tableTop = MyObject(tableTopMesh, initialPositionTemp, initialScaleTemp, {});
}

void MyTable::render(VSShaderLib shader) {
	tableTop.render(shader);
}

MyRoad::MyRoad() {}
MyRoad::MyRoad(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp) {

	MyMesh mainRoadMesh = createCube();

	float ambRoad[] = { 0.66f, 0.66f, 0.66f, 1.0f };
	float diffRoad[] = { 0.2f, 0.2f, 0.1f, 1.0f };
	float specRoad[] = { 0.8f, 0.50f, 0.8f, 1.0f };
	float emissiveRoad[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininessRoad = 100.0f;
	int texcountRoad = 0;

	memcpy(mainRoadMesh.mat.ambient, ambRoad, 4 * sizeof(float));
	memcpy(mainRoadMesh.mat.diffuse, diffRoad, 4 * sizeof(float));
	memcpy(mainRoadMesh.mat.specular, specRoad, 4 * sizeof(float));
	memcpy(mainRoadMesh.mat.emissive, emissiveRoad, 4 * sizeof(float));
	mainRoadMesh.mat.shininess = shininessRoad;
	mainRoadMesh.mat.texCount = texcountRoad;

	mainRoad = MyObject(mainRoadMesh, initialPositionTemp, initialScaleTemp, {});

	MyMesh leftMarginMesh = createCube();
	MyMesh rightMarginMesh = createCube();

	float ambMargin[] = { 0.50f, 0.50f, 0.50f, 1.0f };
	float diffMargin[] = { 0.2f, 0.8f, 0.50f, 1.0f };
	float specMargin[] = { 0.8f, 0.2f, 0.8f, 1.0f };
	float emissiveMargin[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininessMargin = 100.0f;
	int texcountMargin = 0;

	memcpy(leftMarginMesh.mat.ambient, ambMargin, 4 * sizeof(float));
	memcpy(leftMarginMesh.mat.diffuse, diffMargin, 4 * sizeof(float));
	memcpy(leftMarginMesh.mat.specular, specMargin, 4 * sizeof(float));
	memcpy(leftMarginMesh.mat.emissive, emissiveMargin, 4 * sizeof(float));
	leftMarginMesh.mat.shininess = shininessMargin;
	leftMarginMesh.mat.texCount = texcountMargin;

	memcpy(rightMarginMesh.mat.ambient, ambMargin, 4 * sizeof(float));
	memcpy(rightMarginMesh.mat.diffuse, diffMargin, 4 * sizeof(float));
	memcpy(rightMarginMesh.mat.specular, specMargin, 4 * sizeof(float));
	memcpy(rightMarginMesh.mat.emissive, emissiveMargin, 4 * sizeof(float));
	rightMarginMesh.mat.shininess = shininessMargin;
	rightMarginMesh.mat.texCount = texcountMargin;

	MyVec3 leftMarginPosition = MyVec3{ initialPositionTemp.x - float(2), initialPositionTemp.y, initialPositionTemp.z };
	MyVec3 rightMarginPosition = MyVec3{ initialPositionTemp.x + float(2), initialPositionTemp.y, initialPositionTemp.z };
	MyVec3 marginScale = MyVec3{ initialScaleTemp.x / 5, initialScaleTemp.y + float(0.5), initialScaleTemp.z + float(0.02) };

	leftMargin = MyObject(leftMarginMesh, leftMarginPosition, marginScale, {});
	rightMargin = MyObject(rightMarginMesh, rightMarginPosition, marginScale, {});
}

void MyRoad::render(VSShaderLib shader) {
	mainRoad.render(shader);
	leftMargin.render(shader);
	rightMargin.render(shader);
}

float MyCar::MAX_VELOCITY = 1.5f;
float MyCar::START_ACCELERATION = 0.01f;
float MyCar::STOP_ACCELERATION = 0.0f;
float MyCar::FRICTION_COEFICIENT = 0.0055f;
float MyCar::ANGLE_ROTATION_VELOCITY = 2.0f;
float MyCar::MAX_WHEEL_ANGLE = 3.0f;
float MyCar::FRICTION_ROTATION_COEFICIENT = 0.055f;

MyCar::MyCar() {}
MyCar::MyCar(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp) {

	MyMesh mainBlockMesh = createCube();

	float ambBlock[] = { 0.1f, 0.5f, 0.9f, 1.0f };
	float diffBlock[] = { 0.6f, 0.1f, 0.3f, 1.0f };
	float specBlock[] = { 0.0f, 0.7f, 0.2f, 1.0f };
	float emissiveBlock[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininessBlock = 100.0f;
	int texcountBlock = 0;

	memcpy(mainBlockMesh.mat.ambient, ambBlock, 4 * sizeof(float));
	memcpy(mainBlockMesh.mat.diffuse, diffBlock, 4 * sizeof(float));
	memcpy(mainBlockMesh.mat.specular, specBlock, 4 * sizeof(float));
	memcpy(mainBlockMesh.mat.emissive, emissiveBlock, 4 * sizeof(float));
	mainBlockMesh.mat.shininess = shininessBlock;
	mainBlockMesh.mat.texCount = texcountBlock;

	mainBlock = MyObject(mainBlockMesh, initialPositionTemp, initialScaleTemp, {});

	float ambWheel[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float diffWheel[] = { 0.6f, 0.1f, 0.3f, 1.0f };
	float specWheel[] = { 0.0f, 0.7f, 0.2f, 1.0f };
	float emissiveWheel[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininessWheel = 100.0f;
	int texcountWheel = 0;

	for (int i = 0; i < 4; i++) {

		MyMesh wheelMesh = createTorus(1, 1.5, 20, 20);

		memcpy(wheelMesh.mat.ambient, ambWheel, 4 * sizeof(float));
		memcpy(wheelMesh.mat.diffuse, diffWheel, 4 * sizeof(float));
		memcpy(wheelMesh.mat.specular, specWheel, 4 * sizeof(float));
		memcpy(wheelMesh.mat.emissive, emissiveWheel, 4 * sizeof(float));
		wheelMesh.mat.shininess = shininessWheel;
		wheelMesh.mat.texCount = texcountWheel;

		MyVec3 varPosition = { 0, 0, 0 };
		float distanceWheelX = 0.58;
		float distanceWheelZ = 0.8;
		if (i % 2 == 0) { varPosition.x = distanceWheelX; }
		else { varPosition.x = -distanceWheelX;  }
		if (float(i) / 2.0 >= 1.0) { varPosition.z = distanceWheelZ; }
		else { varPosition.z = -distanceWheelZ; }

		MyVec3 wheelPosition = MyVec3{ initialPositionTemp.x + varPosition.x, initialPositionTemp.y - (initialScaleTemp.y / 2.2f), initialPositionTemp.z + varPosition.z };
		MyVec3 wheelScale = MyVec3{ initialScaleTemp.x / 3, initialScaleTemp.y / 4, initialScaleTemp.x / 3 };
		MyVec3Rotation wheelRotation = MyVec3Rotation{ 90, 0, 0, 1 };

		MyObject wheel = MyObject(wheelMesh, wheelPosition, wheelScale, { wheelRotation });
		wheels.push_back(wheel);
	}
}

void MyCar::render(VSShaderLib shader) {

	mainBlock.render(shader);
	for (MyObject wheel : wheels) { wheel.render(shader); }
	
}

MyVec3 MyCar::getPosition() {
	return mainBlock.positionVec;
}

void MyCar::tick() {

	// Update velocity
	velocity = velocity + acceleration;
	if (velocity >= MAX_VELOCITY) velocity = MAX_VELOCITY;
	else if (velocity <= -MAX_VELOCITY) velocity = -MAX_VELOCITY;

	if (velocity > 0) velocity = std::max(velocity - FRICTION_COEFICIENT, 0.0f);
	else if (velocity < 0) velocity = std::min(velocity + FRICTION_COEFICIENT, 0.0f);

	// Update rotation
	float velocityFactor = abs(velocity) / MAX_VELOCITY;
	int rotationSignal = (signbit(rotationVelocity)) ? -1 : 1;
	if (rotationVelocity == 0.0f) { rotationSignal = 0; }

	if (rotationVelocity > 0) rotationVelocity = std::max(rotationVelocity - FRICTION_ROTATION_COEFICIENT, 0.0f);
	else if (rotationVelocity < 0) rotationVelocity = std::min(rotationVelocity + FRICTION_ROTATION_COEFICIENT, 0.0f);
	
	rotationWheelAngle = rotationVelocity * velocityFactor;
	if (rotationWheelAngle >= MAX_WHEEL_ANGLE) { rotationWheelAngle = MAX_WHEEL_ANGLE;  }
	else if (rotationWheelAngle <= - MAX_WHEEL_ANGLE) { rotationWheelAngle = - MAX_WHEEL_ANGLE; }

	// Update direction
	double angleRadians = atan2(direction.z, direction.x);
	double angleDegrees = angleRadians * 180 / O_PI;

	angleDegrees += rotationWheelAngle;

	direction.x = float(cos(angleDegrees / (180 / O_PI)));
	direction.z = float(sin(angleDegrees / (180 / O_PI)));

	// Update positions
	mainBlock.positionVec.x += velocity * direction.x;
	mainBlock.positionVec.y += velocity * direction.y;
	mainBlock.positionVec.z += velocity * direction.z;

	for (MyObject & wheel : wheels) {
		wheel.positionVec.x += velocity * direction.x;
		wheel.positionVec.y += velocity * direction.y;
		wheel.positionVec.z += velocity * direction.z;
	}

	// Update rotations
	mainBlock.rotateVec = { MyVec3Rotation{ -float(angleDegrees) - 90 , 0, 1, 0 } };
	MyVec3Rotation wheelStandardRotation = MyVec3Rotation{ 90, 0, 0, 1 };
	MyVec3Rotation wheelDriveRotation = MyVec3Rotation{ -float(angleDegrees) - 90, 1, 0, 0 };
	for (MyObject& wheel : wheels) {
		wheel.rotateVec = { wheelStandardRotation, wheelDriveRotation };
	}
}

void MyCar::forward() {
	acceleration = START_ACCELERATION;
}

void MyCar::backward() {
	acceleration = -START_ACCELERATION;
}

void MyCar::stop() {
	acceleration = STOP_ACCELERATION;
}

void MyCar::turnLeft() {
	rotationVelocity = - ANGLE_ROTATION_VELOCITY;
}

void MyCar::turnRight() {
	rotationVelocity = ANGLE_ROTATION_VELOCITY;
}

float MyOrange::MAX_VELOCITY = 1.5f;
float MyOrange::ANGLE_ROTATION_VELOCITY = 2.0f;

MyOrange::MyOrange() {}
MyOrange::MyOrange(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp) {

	MyMesh orangeMesh = createSphere(2.0, 20);

	float amb[] = { 1.0f, 0.55f, 0.0f, 1.0f };
	float diff[] = { 0.6f, 0.1f, 0.3f, 1.0f };
	float spec[] = { 0.0f, 0.7f, 0.2f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;

	memcpy(orangeMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(orangeMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(orangeMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(orangeMesh.mat.emissive, emissive, 4 * sizeof(float));
	orangeMesh.mat.shininess = shininess;
	orangeMesh.mat.texCount = texcount;

	orange = MyObject(orangeMesh, initialPositionTemp, initialScaleTemp, {});
}

void MyOrange::render(VSShaderLib shader) {
	orange.render(shader);
}

void MyOrange::tick() {

}

MyPacketButter::MyPacketButter() {}
MyPacketButter::MyPacketButter(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp) {

	MyMesh butterMesh = createCube();

	float amb[] = { 1.0f, 1.0f, 0.2f, 1.0f };
	float diff[] = { 0.6f, 0.1f, 0.3f, 1.0f };
	float spec[] = { 0.0f, 0.7f, 0.2f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;

	memcpy(butterMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(butterMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(butterMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(butterMesh.mat.emissive, emissive, 4 * sizeof(float));
	butterMesh.mat.shininess = shininess;
	butterMesh.mat.texCount = texcount;

	butter = MyObject(butterMesh, initialPositionTemp, initialScaleTemp, {});
}

void MyPacketButter::render(VSShaderLib shader) {
	butter.render(shader);
}