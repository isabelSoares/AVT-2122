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
	GLint texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");

	if (textureOption == MyTextureOption::Multitexturing) glUniform1i(texMode_uniformId, 3);
	else glUniform1i(texMode_uniformId, 0);

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
MyVec3 MyObject::calculatePointInWorld(MyVec3 point) {
	pushMatrix(MODEL);

	translate(MODEL, positionVec.x, positionVec.y, positionVec.z);
	for (MyVec3Rotation rotation : rotateVec) { rotate(MODEL, rotation.angle, rotation.x, rotation.y, rotation.z); }
	scale(MODEL, scaleVec.x, scaleVec.y, scaleVec.z);
	for (MyVec3 translateBefore : translationBeforeRotation) { translate(MODEL, translateBefore.x, translateBefore.y, translateBefore.z); }

	float positionInArray[4] = { point.x, point.y, point.z , 1.0 };
	float* positionTranslated = (float*)malloc(4 * sizeof(float));
	multMatrixPoint(MODEL, positionInArray, positionTranslated);

	popMatrix(MODEL);

	return MyVec3{ positionTranslated[0], positionTranslated[1], positionTranslated[2] };
}

MyCheerio::MyCheerio() {}
MyCheerio::MyCheerio(MyVec3 positionTemp, float innerCheerioRadiusTemp, float outterCheerioRadiusTemp) {

	position = positionTemp;
	innerCheerioRadius = innerCheerioRadiusTemp;
	outterCheerioRadius = outterCheerioRadiusTemp;

	float ambCheerio[] = { 1.0f, 0.77f, 0.05f, 1.0f };
	float diffCheerio[] = { 0.2f, 0.8f, 0.50f, 1.0f };
	float specCheerio[] = { 0.8f, 0.2f, 0.8f, 1.0f };
	float emissiveCheerio[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininessCheerio = 100.0f;
	int texcountCheerio = 0;

	MyMesh cheerioMesh = createTorus(innerCheerioRadius, outterCheerioRadius, 25, 25);

	memcpy(cheerioMesh.mat.ambient, ambCheerio, 4 * sizeof(float));
	memcpy(cheerioMesh.mat.diffuse, diffCheerio, 4 * sizeof(float));
	memcpy(cheerioMesh.mat.specular, specCheerio, 4 * sizeof(float));
	memcpy(cheerioMesh.mat.emissive, emissiveCheerio, 4 * sizeof(float));
	cheerioMesh.mat.shininess = shininessCheerio;
	cheerioMesh.mat.texCount = texcountCheerio;

	MyVec3 cheerioPosition = position;
	MyVec3 cheerioScale = MyVec3{ 1, 3, 1 };

	cheerio = MyObject(cheerioMesh, cheerioPosition, cheerioScale, {});
}

void MyCheerio::render(VSShaderLib shader) {
	cheerio.render(shader);
}

MyVec3 MyCheerio::getPosition() {
	return position;
}
std::vector<MyVec3> MyCheerio::getBoundRect() {

	//MyMesh wheelMesh = createTorus(innerCheerioRadius, outterCheerioRadius, 25, 25);

	std::vector<MyVec3> pointsToCheck = { MyVec3{ outterCheerioRadius, (outterCheerioRadius - innerCheerioRadius) / 2, outterCheerioRadius },
											MyVec3{ outterCheerioRadius, (outterCheerioRadius - innerCheerioRadius) / 2, - outterCheerioRadius },
											MyVec3{ outterCheerioRadius, - (outterCheerioRadius - innerCheerioRadius) / 2, outterCheerioRadius },
											MyVec3{ outterCheerioRadius, - (outterCheerioRadius - innerCheerioRadius) / 2, - outterCheerioRadius },
											MyVec3{ - outterCheerioRadius, (outterCheerioRadius - innerCheerioRadius) / 2, outterCheerioRadius },
											MyVec3{ - outterCheerioRadius, (outterCheerioRadius - innerCheerioRadius) / 2, - outterCheerioRadius },
											MyVec3{ - outterCheerioRadius, - (outterCheerioRadius - innerCheerioRadius) / 2, outterCheerioRadius },
											MyVec3{ - outterCheerioRadius, - (outterCheerioRadius - innerCheerioRadius) / 2, - outterCheerioRadius } };

	MyVec3 tempWorld = cheerio.calculatePointInWorld(pointsToCheck[0]);

	MyVec3 minPosition = tempWorld;
	MyVec3 maxPosition = tempWorld;

	for (MyVec3 point : pointsToCheck) {

		MyVec3 pointConverted = cheerio.calculatePointInWorld(point);

		minPosition = MyVec3{ std::min(minPosition.x, pointConverted.x), std::min(minPosition.y, pointConverted.y), std::min(minPosition.z, pointConverted.z) };
		maxPosition = MyVec3{ std::max(maxPosition.x, pointConverted.x), std::max(maxPosition.y, pointConverted.y), std::max(maxPosition.z, pointConverted.z) };
	}

	// Store this cheerio bounds
	return { minPosition, maxPosition };
}

void MyCheerio::tick() {

	velocity = velocity * (1 - FRICTION_COEFICIENT);

	// Update position
	position.x += velocity * direction.x;
	position.y += velocity * direction.y;
	position.z += velocity * direction.z;

	cheerio.positionVec = position;
}

MyTable::MyTable() {}
MyTable::MyTable(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp) {

	MyMesh tableTopMesh = createCube();

	float amb[] = { 0.3f, 0.0f, 0.0f, 1.0f };
	float diff[] = { 0.8f, 0.1f, 0.1f, 1.0f };
	float spec[] = { 0.3f, 0.3f, 0.3f, 1.0f };
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
	tableTop.textureOption = MyTextureOption::Multitexturing;
}

void MyTable::render(VSShaderLib shader) {
	tableTop.render(shader);
}

MyRoad::MyRoad() {}
MyRoad::MyRoad(MyVec3 positionTemp, float width, float length, float cheerios_distance, float innerCheerioRadiusTemp, float outterCheerioRadiusTemp) {

	position = positionTemp;
	scaling = MyVec3{ width, ROAD_HEIGHT, length };


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

	MyVec3 mainRoadScaling = scaling;
	MyVec3 mainRoadTranslation = position;

	mainRoad = MyObject(mainRoadMesh, mainRoadTranslation, mainRoadScaling, {});

	// Left / Margin
	for (int i = 0; i < 2; i++) {
		
		// 1 = Right / -1 = Left
		int correctedMarginFactor = i * 2 - 1;

		// Backwards / Forwards
		for (int j = 0; j < 2; j++) {

			// 1 = Forwards / -1 = Backwards
			int correctedPlacingFactor = - (j * 2 - 1);

			float currentPosition = 0.0f;
			while (abs(currentPosition) <= length / 2) {

				// Backward does not place in 0
				if (correctedPlacingFactor == -1 && currentPosition == 0) { 
					// Update next position
					currentPosition = currentPosition + correctedPlacingFactor * cheerios_distance;
					continue;
				}

				MyVec3 cheerioPosition = position + MyVec3{ correctedMarginFactor * (width / 2 - outterCheerioRadiusTemp), (outterCheerioRadiusTemp - innerCheerioRadiusTemp) * 2, currentPosition};
				MyCheerio cheerio = MyCheerio(cheerioPosition, innerCheerioRadiusTemp, outterCheerioRadiusTemp);
				cheerios.push_back(cheerio);

				// Update next position
				currentPosition = currentPosition + correctedPlacingFactor * cheerios_distance;
			}
		}
	}
}

void MyRoad::render(VSShaderLib shader) {
	mainRoad.render(shader);
	for (MyCheerio cheerio : cheerios) cheerio.render(shader);
}

void MyRoad::tick() {
	for (int index = 0; index < cheerios.size(); index++) cheerios[index].tick();
}

std::vector<MyCheerio> MyRoad::getCheerios() { return cheerios; }

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
std::vector<MyVec3> MyCar::getBoundRect() {

	std::vector<MyVec3> pointsToCheck = { MyVec3{ 0.5, 0.5, 0.5 }, MyVec3{ -0.5, 0.5, 0.5 } , MyVec3{ 0.5, -0.5, 0.5 } , MyVec3{ 0.5, 0.5, -0.5 } , MyVec3{ -0.5, -0.5, 0.5 } , MyVec3{ 0.5, -0.5, -0.5 } , MyVec3{ -0.5, 0.5, -0.5 } , MyVec3{ -0.5, -0.5, -0.5 } };

	MyVec3 tempWorld = mainBlock.calculatePointInWorld(pointsToCheck[0]);

	MyVec3 minPosition = tempWorld;
	MyVec3 maxPosition = tempWorld;

	for (MyVec3 point : pointsToCheck) {

		MyVec3 pointConverted = mainBlock.calculatePointInWorld(point);

		minPosition = MyVec3{ std::min(minPosition.x, pointConverted.x), std::min(minPosition.y, pointConverted.y), std::min(minPosition.z, pointConverted.z) };
		maxPosition = MyVec3{ std::max(maxPosition.x, pointConverted.x), std::max(maxPosition.y, pointConverted.y), std::max(maxPosition.z, pointConverted.z) };
	}

	for (int i = 0; i < 4; i++) {

		//MyMesh wheelMesh = createTorus(0.3, 0.45, 20, 20);
		std::vector<MyVec3> pointsToCheck = { MyVec3{ 0.45, (0.45 - 0.3) / 2, 0.45 }, MyVec3{ 0.45, (0.45 - 0.3) / 2, - 0.45 }, 
											MyVec3{ 0.45, - (0.45 - 0.3) / 2, 0.45 }, MyVec3{ 0.45, - (0.45 - 0.3) / 2, - 0.45 }, 
											MyVec3{ - 0.45, (0.45 - 0.3) / 2, 0.45 }, MyVec3{ - 0.45, (0.45 - 0.3) / 2, - 0.45 }, 
											MyVec3{ -0.45, - (0.45 - 0.3) / 2, 0.45 }, MyVec3{ - 0.45, - (0.45 - 0.3) / 2, - 0.45 } };

		for (MyVec3 point : pointsToCheck) {

			MyVec3 pointConverted = wheels[i].calculatePointInWorld(point);

			minPosition = MyVec3{ std::min(minPosition.x, pointConverted.x), std::min(minPosition.y, pointConverted.y), std::min(minPosition.z, pointConverted.z) };
			maxPosition = MyVec3{ std::max(maxPosition.x, pointConverted.x), std::max(maxPosition.y, pointConverted.y), std::max(maxPosition.z, pointConverted.z) };
		}
	}

	return { minPosition, maxPosition };
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
	direction.y = 0;
	direction.z = float(sin((angleDegrees) / (180 / O_PI)));

	// Update position
	position.x += velocity * direction.x;
	position.y += velocity * direction.y;
	position.z += velocity * direction.z;

	// Update Position and Rotation
	updateObjects();
}

void MyCar::collisionStop() {

	// Update position
	position.x -= velocity * direction.x;
	position.y -= velocity * direction.y;
	position.z -= velocity * direction.z;

	// Update Position and Rotation
	updateObjects();
}

void MyCar::updateObjects() {

	float dot = 1 * direction.x + 0 * direction.z;
	float det = 1 * direction.z - 0 * direction.x;
	double angleRadians = atan2(det, dot);
	double angleDegrees = fmod((angleRadians * 180 / O_PI) + 360, 360);

	mainBlock.scaleVec = scaling * MAIN_BLOCK_SCALING_VARIATION;
	mainBlock.rotateVec = { MyVec3Rotation{float(-angleDegrees - 90), 0, 1, 0} };
	mainBlock.positionVec = position + MAIN_BLOCK_TRANSLATION_VARIATION;

	for (int i = 0; i < 4; i++) {

		MyVec3 wheelPosition = position;
		MyVec3 wheelScale = scaling * WHEEL_SCALING_VARIATION;
		std::vector<MyVec3Rotation> wheelRotations = { WHEEL_ROTATION_VARIATION, MyVec3Rotation{float(-angleDegrees - 90), 1, 0, 0} };

		wheels[i].positionVec = wheelPosition;
		wheels[i].scaleVec = wheelScale;
		wheels[i].rotateVec = wheelRotations;

		wheels[i].translationBeforeRotation = { WHEELS_TRANSLATION_VARIATION[i] };
	}

	for (int i = 0; i < 2; i++) {

		MyVec3 lightPosition = position;
		std::vector<MyVec3Rotation> lightRotations = { SPOTLIGHT_ROTATION_VARIATION, MyVec3Rotation{float(-angleDegrees - 90), 1, 0, 0} };

		spotlights[i]->positionVec = lightPosition;
		spotlights[i]->rotateVec = lightRotations;
		spotlights[i]->translationBeforeRotation = { SPOTLIGHTS_TRANSLATION_VARIATION[i] };
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
std::vector<MyVec3> MyOrange::getBoundRect() {

	MyVec3 orangeWorldTop = orange.calculatePointInWorld(MyVec3{ 2, 2, 2 });
	MyVec3 orangeWorldBottom = orange.calculatePointInWorld(MyVec3{ -2, -2, -2 });

	MyVec3 minPosition = MyVec3{ std::min(orangeWorldBottom.x, orangeWorldTop.x), std::min(orangeWorldBottom.y, orangeWorldTop.y), std::min(orangeWorldBottom.z, orangeWorldTop.z) };
	MyVec3 maxPosition = MyVec3{ std::max(orangeWorldBottom.x, orangeWorldTop.x), std::max(orangeWorldBottom.y, orangeWorldTop.y), std::max(orangeWorldBottom.z, orangeWorldTop.z) };

	return { minPosition, maxPosition };
}

void MyOrange::tick() {

	// Update positions
	orange.positionVec.x += velocity * direction.x;
	orange.positionVec.y += velocity * direction.y;
	orange.positionVec.z += velocity * direction.z;
}

MyPacketButter::MyPacketButter() {}
MyPacketButter::MyPacketButter(MyVec3 positionTemp, MyVec3 scaleTemp) {

	position = positionTemp;
	scaling = scaleTemp;
	
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

	butter = MyObject(butterMesh, position, scaling, {});
}

MyVec3 MyPacketButter::getPosition() {
	return position;
}

void MyPacketButter::render(VSShaderLib shader) {
	butter.render(shader);
}

void MyPacketButter::tick() {

	velocity = velocity * (1 - FRICTION_COEFICIENT);

	// Update position
	position.x += velocity * direction.x;
	position.y += velocity * direction.y;
	position.z += velocity * direction.z;

	butter.positionVec = position;
}

std::vector<MyVec3> MyPacketButter::getBoundRect() {

	std::vector<MyVec3> pointsToCheck = { MyVec3{ 0.5, 0.5, 0.5 }, MyVec3{ -0.5, 0.5, 0.5 } , MyVec3{ 0.5, -0.5, 0.5 } , MyVec3{ 0.5, 0.5, -0.5 } , MyVec3{ -0.5, -0.5, 0.5 } , MyVec3{ 0.5, -0.5, -0.5 } , MyVec3{ -0.5, 0.5, -0.5 } , MyVec3{ -0.5, -0.5, -0.5 } };

	MyVec3 tempWorld = butter.calculatePointInWorld(pointsToCheck[0]);

	MyVec3 minPosition = tempWorld;
	MyVec3 maxPosition = tempWorld;

	for (MyVec3 point : pointsToCheck) {

		MyVec3 pointConverted = butter.calculatePointInWorld(point);

		minPosition = MyVec3{ std::min(minPosition.x, pointConverted.x), std::min(minPosition.y, pointConverted.y), std::min(minPosition.z, pointConverted.z) };
		maxPosition = MyVec3{ std::max(maxPosition.x, pointConverted.x), std::max(maxPosition.y, pointConverted.y), std::max(maxPosition.z, pointConverted.z) };
	}

	return { minPosition, maxPosition };
}