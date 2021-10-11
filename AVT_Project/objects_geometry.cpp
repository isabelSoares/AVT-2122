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

#include "spotlight.h"
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
	for (MyVec3Rotation rotation : rotateVec) { rotate(MODEL, rotation.angle, rotation.x, rotation.y, rotation.z);  }
	scale(MODEL, scaleVec.x, scaleVec.y, scaleVec.z);

	for (MyVec3 translateBefore : translationBeforeRotation) { translate(MODEL, translateBefore.x, translateBefore.y, translateBefore.z); }

	GLint pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	GLint vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	GLint normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	GLint lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_positions");

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
MyCar::MyCar(MyVec3 positionTemp, std::vector<MySpotlight*> spotlightsTemp) {

	position = positionTemp;

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

	mainBlock = MyObject(mainBlockMesh, position + MAIN_BLOCK_TRANSLATION_VARIATION, scaling * MAIN_BLOCK_SCALING_VARIATION, {MAIN_BLOCK_ROTATION_VARIATION});

	float ambWheel[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float diffWheel[] = { 0.6f, 0.1f, 0.3f, 1.0f };
	float specWheel[] = { 0.0f, 0.7f, 0.2f, 1.0f };
	float emissiveWheel[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininessWheel = 100.0f;
	int texcountWheel = 0;

	for (int i = 0; i < 4; i++) {

		MyMesh wheelMesh = createTorus(0.3, 0.45, 20, 20);

		memcpy(wheelMesh.mat.ambient, ambWheel, 4 * sizeof(float));
		memcpy(wheelMesh.mat.diffuse, diffWheel, 4 * sizeof(float));
		memcpy(wheelMesh.mat.specular, specWheel, 4 * sizeof(float));
		memcpy(wheelMesh.mat.emissive, emissiveWheel, 4 * sizeof(float));
		wheelMesh.mat.shininess = shininessWheel;
		wheelMesh.mat.texCount = texcountWheel;

		MyVec3 wheelPosition = position;
		MyVec3 wheelScale = scaling * WHEEL_SCALING_VARIATION;

		MyObject wheel = MyObject(wheelMesh, wheelPosition, wheelScale, { WHEEL_ROTATION_VARIATION, MyVec3Rotation{float(180), 1, 0, 0} });
		wheels.push_back(wheel);
		wheels[i].translationBeforeRotation = { WHEELS_TRANSLATION_VARIATION[i] };
	}

	spotlights = spotlightsTemp;
	for (int i = 0; i < 2; i++) {
		spotlights[i]->direction = direction;
		spotlights[i]->position = position + SPOTLIGHTS_TRANSLATION_VARIATION[i];
	}

}

void MyCar::render(VSShaderLib shader) {

	mainBlock.render(shader);
	for (MyObject wheel : wheels) { wheel.render(shader); }
	
}

MyVec3 MyCar::getPosition() {
	return position;
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
	float dot = 1 * direction.x + 0 * direction.z;
	float det = 1 * direction.z - 0 * direction.x;
	double angleRadians = atan2(det, dot);
	double angleDegrees = fmod((angleRadians * 180 / O_PI) + 360, 360);

	angleDegrees += rotationWheelAngle;

	direction.x = float(cos((angleDegrees) / (180 / O_PI)));
	direction.z = float(sin((angleDegrees) / (180 / O_PI)));

	// Update position
	position.x += velocity * direction.x;
	position.y += velocity * direction.y;
	position.z += velocity * direction.z;

	// Update Position and Rotation

	mainBlock.scaleVec = scaling * MAIN_BLOCK_SCALING_VARIATION;
	mainBlock.rotateVec = { MyVec3Rotation{float(- angleDegrees - 90), 0, 1, 0} };
	mainBlock.positionVec = position + MAIN_BLOCK_TRANSLATION_VARIATION;

	for (int i = 0; i < 4; i++) {

		MyVec3 wheelPosition = position;
		MyVec3 wheelScale = scaling * WHEEL_SCALING_VARIATION;
		std::vector<MyVec3Rotation> wheelRotations = { WHEEL_ROTATION_VARIATION, MyVec3Rotation{float(- angleDegrees - 90), 1, 0, 0} };
		
		wheels[i].positionVec = wheelPosition;
		wheels[i].scaleVec = wheelScale;
		wheels[i].rotateVec = wheelRotations;

		wheels[i].translationBeforeRotation = { WHEELS_TRANSLATION_VARIATION[i] };
	}

	for (int i = 0; i < 2; i++) {
		spotlights[i]->direction = direction;
		spotlights[i]->position = position + SPOTLIGHTS_TRANSLATION_VARIATION[i];
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

float MyOrange::ANGLE_ROTATION_VELOCITY = 2.0f;

MyOrange::MyOrange() {}
MyOrange::MyOrange(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp, float maxVelocity) {

	MyMesh orangeMesh = createSphere(2.0, 50);

	float amb[] = { 1.0f, 0.55f, 0.0f, 1.0f };
	float diff[] = { 0.6f, 0.1f, 0.3f, 1.0f };
	float spec[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 250.0f;
	int texcount = 0;

	memcpy(orangeMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(orangeMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(orangeMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(orangeMesh.mat.emissive, emissive, 4 * sizeof(float));
	orangeMesh.mat.shininess = shininess;
	orangeMesh.mat.texCount = texcount;

	// Set Parameters
	double angleDegrees = rand() % 360;
	direction.x = float(cos(angleDegrees / (180 / O_PI)));
	direction.y = 0;
	direction.z = float(sin(angleDegrees / (180 / O_PI)));

	float randomFloat = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	velocity = (randomFloat * 0.8 + 0.2) * maxVelocity;


	orange = MyObject(orangeMesh, initialPositionTemp, initialScaleTemp, {});
}

void MyOrange::render(VSShaderLib shader) {
	orange.render(shader);
}

MyVec3 MyOrange::getPosition() {
	return orange.positionVec;
}

void MyOrange::tick() {

	// Update positions
	orange.positionVec.x += velocity * direction.x;
	orange.positionVec.y += velocity * direction.y;
	orange.positionVec.z += velocity * direction.z;
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