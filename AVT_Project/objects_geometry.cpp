#include <string>
#include <assert.h>
#include <stdlib.h>
#include <vector>

#include <iostream>
#include <sstream>
#include <fstream>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>

// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

// assimp include files. These three are usually needed.
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/scene.h"

#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"

#include "VSShaderlib.h"
#include "meshFromAssimp.h"
#include "l3dBillboard.h"

#include "spotlight.h"
#include "pointlight.h"
#include "objects_geometry.h"

using namespace std;

extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

extern Assimp::Importer importer;
extern const aiScene* scene;
char model_dir[50];

const float TARGET_FPS = 50.0f;

MyObject::MyObject() {}
MyObject::MyObject(MyMesh meshTemp, MyVec3 positionTemp, MyVec3 scaleTemp, std::vector<MyVec3Rotation> rotateTemp) {
	mesh = meshTemp;
	positionVec = positionTemp;
	scaleVec = scaleTemp;
	rotateVec = rotateTemp;
}

void MyObject::render(VSShaderLib& shader) {

	GLint loc;

	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh.mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh.mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh.mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh.mat.shininess);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.texCount");
	glUniform1i(loc, mesh.mat.texCount);
	pushMatrix(MODEL);

	translate(MODEL, positionVec.x, positionVec.y, positionVec.z);
	for (MyVec3Rotation rotation : rotateVec) { rotate(MODEL, rotation.angle, rotation.x, rotation.y, rotation.z);  }
	scale(MODEL, scaleVec.x, scaleVec.y, scaleVec.z);
	for (MyVec3 translateBefore : translationBeforeRotation) { translate(MODEL, translateBefore.x, translateBefore.y, translateBefore.z); }

	GLint pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	GLint vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	GLint normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	GLint model_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_Model");
	GLint view_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_View");
	GLint lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_positions");
	GLint texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");

	GLint reflect_perFragment_uniformId = glGetUniformLocation(shader.getProgramIndex(), "reflect_perFrag");
	GLint bumpMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "bumpMode");

	GLint normalMap_loc = glGetUniformLocation(shader.getProgramIndex(), "normalMap");
	GLint specularMap_loc = glGetUniformLocation(shader.getProgramIndex(), "specularMap");
	GLint diffMapCount_loc = glGetUniformLocation(shader.getProgramIndex(), "diffMapCount");

	if (textureOption == MyTextureOption::Multitexturing) glUniform1i(texMode_uniformId, 3);
	else if (textureOption == MyTextureOption::Orange) glUniform1i(texMode_uniformId, 4);
	else if (textureOption == MyTextureOption::Tree) glUniform1i(texMode_uniformId, 6);
	else if (textureOption == MyTextureOption::WaterParticle) glUniform1i(texMode_uniformId, 7);
	else if (textureOption == MyTextureOption::SkyBox) glUniform1i(texMode_uniformId, 8);
	else if (textureOption == MyTextureOption::CubeReflector) glUniform1i(texMode_uniformId, 9);
	else if (textureOption == MyTextureOption::Candle) glUniform1i(texMode_uniformId, 10);
	else glUniform1i(texMode_uniformId, 0);

	glUniform1i(reflect_perFragment_uniformId, reflectedPerFrag ? 1 : 0);

	// Parameters from OBJs
	glUniform1i(normalMap_loc, false);   //GLSL normalMap variable initialized to 0
	glUniform1i(specularMap_loc, false);
	glUniform1ui(diffMapCount_loc, 0);

	if (bumpmapOption == MyBumpMapOption::None) glUniform1i(bumpMode_uniformId, 0);
	else if (bumpmapOption == MyBumpMapOption::Candle) glUniform1i(bumpMode_uniformId, 1);
	else glUniform1i(bumpMode_uniformId, 0);

	glUniformMatrix4fv(model_uniformId, 1, GL_FALSE, mMatrix[MODEL]);
	glUniformMatrix4fv(view_uniformId, 1, GL_FALSE, mMatrix[VIEW]);

	// send matrices to OGL5
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

MyAssimpObject::MyAssimpObject() {}
MyAssimpObject::MyAssimpObject(std::string modelDir, MyVec3 positionTemp, MyVec3 scaleTemp, std::vector<MyVec3Rotation> rotateTemp) {

	std::string filepath; 
	std::ostringstream oss;

	std::strncpy(model_dir, modelDir.c_str(), sizeof(modelDir));

	oss << model_dir << "/" << model_dir << ".obj";
	filepath = oss.str();   //path of OBJ file in the VS project

	strcat(model_dir, "/");  //directory path in the VS project

	//check if file exists
	ifstream fin(filepath.c_str());
	if (!fin.fail()) {
		fin.close();
	} else {
		printf("Couldn't open file: %s\n", filepath.c_str());
		exit(1);
	}
	
	//import 3D file into Assimp scene graph
	if (!Import3DFromFile(filepath)) exit(1);

	//creation of Mymesh array with VAO Geometry and Material
	meshes = createMeshFromAssimp(scene);

	positionVec = positionTemp;
	scaleVec = scaleTemp;
	rotateVec = rotateTemp;
}

void MyAssimpObject::render(VSShaderLib& shader, const aiScene* sc, const aiNode* nd) {

	bool normalMapKey = TRUE;

	GLint loc;

	// Get node transformation matrix
	aiMatrix4x4 m = nd->mTransformation;
	// OpenGL matrices are column major
	m.Transpose();

	// save model matrix and apply node transformation
	pushMatrix(MODEL);

	translate(MODEL, positionVec.x, positionVec.y, positionVec.z);
	for (MyVec3Rotation rotation : rotateVec) { rotate(MODEL, rotation.angle, rotation.x, rotation.y, rotation.z);  }
	scale(MODEL, scaleVec.x, scaleVec.y, scaleVec.z);
	for (MyVec3 translateBefore : translationBeforeRotation) { translate(MODEL, translateBefore.x, translateBefore.y, translateBefore.z); }

	float aux[16];
	memcpy(aux, &m, sizeof(float) * 16);
	multMatrix(MODEL, aux);

	// draw all meshes assigned to this node
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {
		
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, meshes[nd->mMeshes[n]].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, meshes[nd->mMeshes[n]].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, meshes[nd->mMeshes[n]].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.emissive");
		glUniform4fv(loc, 1, meshes[nd->mMeshes[n]].mat.emissive);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, meshes[nd->mMeshes[n]].mat.shininess);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.texCount");
		glUniform1i(loc, meshes[nd->mMeshes[n]].mat.texCount);

		GLint texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");
		glUniform1i(texMode_uniformId, 0);
		GLint reflect_perFragment_uniformId = glGetUniformLocation(shader.getProgramIndex(), "reflect_perFrag");
		glUniform1i(reflect_perFragment_uniformId, 0);
		GLint bumpMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "bumpMode");
		glUniform1i(bumpMode_uniformId, 0);

		unsigned int  diffMapCount = 0;  //read 2 diffuse textures
		
		//devido ao fragment shader suporta 2 texturas difusas simultaneas, 1 especular e 1 normal map

		GLint normalMap_loc = glGetUniformLocation(shader.getProgramIndex(), "normalMap");
		GLint specularMap_loc = glGetUniformLocation(shader.getProgramIndex(), "specularMap");
		GLint diffMapCount_loc = glGetUniformLocation(shader.getProgramIndex(), "diffMapCount");
		glUniform1i(normalMap_loc, false);   //GLSL normalMap variable initialized to 0
		glUniform1i(specularMap_loc, false);
		glUniform1ui(diffMapCount_loc, 0);

		if (meshes[nd->mMeshes[n]].mat.texCount != 0) {

			for (unsigned int i = 0; i < meshes[nd->mMeshes[n]].mat.texCount; ++i) {
				if (meshes[nd->mMeshes[n]].texTypes[i] == DIFFUSE) {
					if (diffMapCount == 0) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff");
						glUniform1i(loc, meshes[nd->mMeshes[n]].texUnits[i] + 13);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else if (diffMapCount == 1) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff1");
						glUniform1i(loc, meshes[nd->mMeshes[n]].texUnits[i] + 13);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else printf("Only supports a Material with a maximum of 2 diffuse textures\n");
				}
				else if (meshes[nd->mMeshes[n]].texTypes[i] == SPECULAR) {
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitSpec");
					glUniform1i(loc, meshes[nd->mMeshes[n]].texUnits[i] + 13);
					glUniform1i(specularMap_loc, true);
				}
				else if (meshes[nd->mMeshes[n]].texTypes[i] == NORMALS) { //Normal map
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitNormalMap");
					if (normalMapKey)
						glUniform1i(normalMap_loc, normalMapKey);
					glUniform1i(loc, meshes[nd->mMeshes[n]].texUnits[i] + 13);

				}
				else printf("Texture Map not supported\n");
			}
		
		}

		// send matrices to OGL
		GLint pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
		GLint vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
		GLint normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");

		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);
		// bind VAO
		glBindVertexArray(meshes[nd->mMeshes[n]].vao);

		if (!shader.isProgramValid()) {
			printf("Program Not Valid!\n");
			exit(1);
		}
		// draw
		glDrawElements(meshes[nd->mMeshes[n]].type, meshes[nd->mMeshes[n]].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	// draw all children
	popMatrix(MODEL);

	for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
		render(shader, sc, nd->mChildren[n]);
	}
}

MyCheerio::MyCheerio() {}
MyCheerio::MyCheerio(MyVec3 positionTemp, float innerCheerioRadiusTemp, float outterCheerioRadiusTemp) {

	position = positionTemp;
	innerCheerioRadius = innerCheerioRadiusTemp;
	outterCheerioRadius = outterCheerioRadiusTemp;

	float ambCheerio[] = { 1.0f, 0.77f, 0.05f, 1.0f };
	float diffCheerio[] = { 1.0f, 1.0f, 0.30f, 1.0f };
	float specCheerio[] = { 1.0f, 1.0f, 0.28f, 1.0f };
	float emissiveCheerio[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininessCheerio = 25.0f;
	int texcountCheerio = 0;

	MyMesh cheerioMesh = createTorus(innerCheerioRadius, outterCheerioRadius, 25, 25);

	memcpy(cheerioMesh.mat.ambient, ambCheerio, 4 * sizeof(float));
	memcpy(cheerioMesh.mat.diffuse, diffCheerio, 4 * sizeof(float));
	memcpy(cheerioMesh.mat.specular, specCheerio, 4 * sizeof(float));
	memcpy(cheerioMesh.mat.emissive, emissiveCheerio, 4 * sizeof(float));
	cheerioMesh.mat.shininess = shininessCheerio;
	cheerioMesh.mat.texCount = texcountCheerio;

	MyVec3 cheerioPosition = position;
	MyVec3 cheerioScale = MyVec3{ 1, 2.3, 1 };

	cheerio = MyObject(cheerioMesh, cheerioPosition, cheerioScale, {});
}

void MyCheerio::render(VSShaderLib& shader) {
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

void MyCheerio::tick(float elapsedTime) {

	float factor = elapsedTime / (1000.0f / TARGET_FPS);

	velocity += - velocity * FRICTION_COEFICIENT * factor;

	// Update position
	position.x += velocity * direction.x * factor;
	position.y += velocity * direction.y * factor;
	position.z += velocity * direction.z * factor;

	cheerio.positionVec = position;
}

MyTable::MyTable() {}
MyTable::MyTable(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp) {

	MyMesh tableTopMesh = createCube();

	float amb[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	float diff[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	float spec[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 20.0f;
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

void MyTable::render(VSShaderLib& shader, bool reflectionPart) {

	if (reflectionPart) tableTop.mesh.mat.diffuse[3] = 0.3f;
	tableTop.render(shader);
	if (reflectionPart) tableTop.mesh.mat.diffuse[3] = 1.0f;
}

MyRoad::MyRoad() {}
MyRoad::MyRoad(MyVec3 positionTemp, float width, float sideDistance, float sizeStraight, float tableWidthTemp, float tableLengthTemp, float cheerios_distance, float innerCheerioRadiusTemp, float outterCheerioRadiusTemp) {

	position = positionTemp;
	scaling = MyVec3{ tableWidthTemp, ROAD_HEIGHT, tableLengthTemp };

	// Left / Margin
	for (int i = 0; i < 2; i++) {
		
		// 1 = Right / -1 = Left
		int correctedMarginFactor = i * 2 - 1;

		float currentPosition = 0.0f;
		float perimeter = sizeStraight + sizeStraight + O_PI * (sideDistance + width);

		while (currentPosition < perimeter) {

			MyVec3 cheerioPositionVariation = MyVec3{ 0, 0, 0 };

			// Left Side 1st Part
			if (currentPosition >= 0 && currentPosition < sizeStraight / 2) {
				cheerioPositionVariation = MyVec3{ -sideDistance / 2 + correctedMarginFactor * width / 2, (outterCheerioRadiusTemp - innerCheerioRadiusTemp) * 2, currentPosition };

			} else if (currentPosition >= sizeStraight / 2 && currentPosition < sizeStraight / 2 + O_PI * (sideDistance + width) / 2) {
				float angleRoad = ((currentPosition - sizeStraight / 2.0f) * 360) / (O_PI * (sideDistance + width)) + 2.0;
				float radius = (sideDistance + correctedMarginFactor * width) / 2;
				MyVec3 center = MyVec3{ 0, 0, sizeStraight / 2 };

				float angleRoadRadians = angleRoad * (O_PI / 180.0f);
				cheerioPositionVariation = center + MyVec3{ radius * cosf(angleRoadRadians), (outterCheerioRadiusTemp - innerCheerioRadiusTemp) * 2, radius * sinf(angleRoadRadians) };

			} else if (currentPosition >= sizeStraight / 2 + O_PI * (sideDistance + width) / 2 && currentPosition < sizeStraight / 2 + O_PI * (sideDistance + width) / 2 + sizeStraight) {

				cheerioPositionVariation = MyVec3{ sideDistance / 2 - correctedMarginFactor * width / 2, (outterCheerioRadiusTemp - innerCheerioRadiusTemp) * 2, - currentPosition + sizeStraight / 2 + O_PI * (sideDistance + width) / 2 + sizeStraight / 2};

			} else if (currentPosition >= sizeStraight / 2 + O_PI * (sideDistance + width) / 2 + sizeStraight && currentPosition < sizeStraight / 2 + O_PI * (sideDistance + width) + sizeStraight) {

				float angleRoad = ((currentPosition - (sizeStraight / 2 + O_PI * (sideDistance + width) / 2 + sizeStraight)) * 360) / (O_PI * (sideDistance + width));
				float radius = (sideDistance + correctedMarginFactor * width) / 2;
				MyVec3 center = MyVec3{ 0, 0, - sizeStraight / 2 };

				float angleRoadRadians = angleRoad * (O_PI / 180.0f);
				cheerioPositionVariation = center + MyVec3{ radius * cosf(angleRoadRadians), (outterCheerioRadiusTemp - innerCheerioRadiusTemp) * 2, - radius * sinf(angleRoadRadians) };

			} else {
				cheerioPositionVariation = MyVec3{ - sideDistance / 2 + correctedMarginFactor * width / 2, (outterCheerioRadiusTemp - innerCheerioRadiusTemp) * 2, currentPosition - (sizeStraight / 2 + O_PI * (sideDistance + width) + sizeStraight) - sizeStraight / 2};
			}

			MyVec3 cheerioPosition = position + cheerioPositionVariation + MyVec3{0, -0.15, 0};
			MyCheerio cheerio = MyCheerio(cheerioPosition, innerCheerioRadiusTemp, outterCheerioRadiusTemp);

			cheerios.push_back(cheerio);

			// Update next position
			currentPosition = currentPosition + cheerios_distance;
		}


	}
}

void MyRoad::render(VSShaderLib& shader) {
	for (MyCheerio cheerio : cheerios) cheerio.render(shader);
}

void MyRoad::tick(float elapsedTime) {
	for (int index = 0; index < cheerios.size(); index++) cheerios[index].tick(elapsedTime);
}

std::vector<MyCheerio> MyRoad::getCheerios() { return cheerios; }

float MyCar::MAX_VELOCITY = 0.7f;
float MyCar::START_ACCELERATION = 0.01f;
float MyCar::STOP_ACCELERATION = 0.0f;
float MyCar::FRICTION_COEFICIENT = 0.0055f;
float MyCar::ANGLE_ROTATION_VELOCITY = 0.9f;
float MyCar::MAX_WHEEL_ANGLE = 3.0f;
float MyCar::FRICTION_ROTATION_COEFICIENT = 0.055f;

MyCar::MyCar() {}
MyCar::MyCar(MyVec3 positionTemp, std::vector<MySpotlight*> spotlightsTemp) {

	position = positionTemp;

	MyMesh mainBlockMesh = createCube();

	float ambBlock[] = { 0.1f, 0.5f, 0.9f, 1.0f };
	float diffBlock[] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float specBlock[] = { 0.0f, 0.6f, 1.0f, 1.0f };
	float emissiveBlock[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininessBlock = 75.0f;
	int texcountBlock = 0;

	memcpy(mainBlockMesh.mat.ambient, ambBlock, 4 * sizeof(float));
	memcpy(mainBlockMesh.mat.diffuse, diffBlock, 4 * sizeof(float));
	memcpy(mainBlockMesh.mat.specular, specBlock, 4 * sizeof(float));
	memcpy(mainBlockMesh.mat.emissive, emissiveBlock, 4 * sizeof(float));
	mainBlockMesh.mat.shininess = shininessBlock;
	mainBlockMesh.mat.texCount = texcountBlock;

	mainBlock = MyObject(mainBlockMesh, position + MAIN_BLOCK_TRANSLATION_VARIATION, scaling * MAIN_BLOCK_SCALING_VARIATION, {MAIN_BLOCK_ROTATION_VARIATION});

	float ambWheel[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float diffWheel[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	float specWheel[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	float emissiveWheel[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininessWheel = 40.0f;
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

	// Deal with OBJs
	carObj = MyAssimpObject("CartoonGreenCar", position + OBJ_TRANSLATION_VARIATION, scaling * OBJ_SCALING_VARIATION, {OBJ_ROTATION_VARIATION});
}

void MyCar::render(VSShaderLib& shader) {

	// mainBlock.render(shader);
	// for (MyObject wheel : wheels) { wheel.render(shader); }
		
	// Render OBJ
	carObj.render(shader, scene, scene->mRootNode);
}

MyVec3 MyCar::getPosition() {
	return position;
}
float MyCar::getDirectionDegrees() {
	float dot = 1 * direction.x + 0 * direction.z;
	float det = 1 * direction.z - 0 * direction.x;
	double angleRadians = atan2(det, dot);
	double angleDegrees = fmod((angleRadians * 180 / O_PI) + 360, 360);

	return angleDegrees;
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

void MyCar::tick(float elapsedTime) {

	float factor = elapsedTime / (1000.0f / TARGET_FPS);

	// Update velocity
	velocity = velocity + acceleration * factor;
	if (velocity >= MAX_VELOCITY) velocity = MAX_VELOCITY;
	else if (velocity <= -MAX_VELOCITY) velocity = -MAX_VELOCITY;

	if (velocity > 0) velocity = std::max(velocity - FRICTION_COEFICIENT, 0.0f);
	else if (velocity < 0) velocity = std::min(velocity + FRICTION_COEFICIENT, 0.0f);

	// Update rotation
	float velocityFactor = velocity / MAX_VELOCITY;
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
	float angleRadians = atan2f(det, dot);
	float angleDegrees = fmod((angleRadians * 180 / O_PI) + 360, 360);

	angleDegrees += rotationWheelAngle * factor;

	direction.x = float(cos((angleDegrees) / (180 / O_PI)));
	direction.y = 0;
	direction.z = float(sin((angleDegrees) / (180 / O_PI)));

	// Update position
	position.x += velocity * direction.x * factor;
	position.y += velocity * direction.y * factor;
	position.z += velocity * direction.z * factor;

	//printf("%f\t\t%f\t\t%f\n", position.x, position.y, position.z);

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

	carObj.scaleVec = scaling * OBJ_SCALING_VARIATION;
	carObj.rotateVec = { OBJ_ROTATION_VARIATION, MyVec3Rotation{float(-angleDegrees - 90), 0, 1, 0} };
	carObj.positionVec = position;
	carObj.translationBeforeRotation = { OBJ_TRANSLATION_VARIATION };

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
		std::vector<MyVec3Rotation> lightRotations = { SPOTLIGHT_ROTATION_VARIATION, MyVec3Rotation{float(-angleDegrees - 90), 0, 1, 0} };

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
	float diff[] = { 0.8f, 0.40f, 0.0f, 1.0f };
	float spec[] = { 0.3f, 0.15f, 0.0f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 50.0f;
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
	orange.textureOption = MyTextureOption::Orange;
}

void MyOrange::render(VSShaderLib& shader) {
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

void MyOrange::tick(float elapsedTime) {

	float factor = elapsedTime / (1000.0f / TARGET_FPS);

	// Update direction
	float dot = 1 * direction.x + 0 * direction.z;
	float det = 1 * direction.z - 0 * direction.x;
	double angleRadians = atan2(det, dot);
	double angleDegrees = fmod((angleRadians * 180 / O_PI) + 360, 360);

	direction.x = float(cos((angleDegrees) / (180 / O_PI)));
	direction.y = 0;
	direction.z = float(sin((angleDegrees) / (180 / O_PI)));

	// Update positions
	orange.positionVec.x += velocity * direction.x * factor;
	orange.positionVec.y += velocity * direction.y * factor;
	orange.positionVec.z += velocity * direction.z * factor;

	angleRotated += velocity * 13 * factor;
	orange.rotateVec = { MyVec3Rotation{float(-angleDegrees - 90), 0, 1, 0}, MyVec3Rotation{- angleRotated, 1, 0, 0} };
}

MyPacketButter::MyPacketButter() {}
MyPacketButter::MyPacketButter(MyVec3 positionTemp, MyVec3 scaleTemp) {

	position = positionTemp;
	scaling = scaleTemp;
	
	MyMesh butterMesh = createCube();

	float amb[] = { 1.0f, 1.0f, 0.2f, 1.0f };
	float diff[] = { 0.8f, 0.8f, 0.20f, 0.7f };
	float spec[] = { 0.8f, 0.8f, 0.18f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 70.0f;
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

void MyPacketButter::render(VSShaderLib& shader) {
	butter.render(shader);
}

void MyPacketButter::tick(float elapsedTime) {

	float factor = elapsedTime / (1000.0f / TARGET_FPS);

	velocity +=  -velocity * FRICTION_COEFICIENT * factor;

	// Update position
	position.x += velocity * direction.x * factor;
	position.y += velocity * direction.y * factor;
	position.z += velocity * direction.z * factor;

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

MyCandle::MyCandle() {}
MyCandle::MyCandle(MyVec3 positionTemp, float heightTemp, float radiusTemp, MyPointlight* lightTemp) {

	position = positionTemp;
	height = heightTemp;
	radius = radiusTemp;
	light = lightTemp;
	
	MyMesh candleMesh = createCylinder(height, radius, 20);

	float amb[] = { 1.0f, 1.0f, 0.2f, 1.0f };
	float diff[] = { 0.80f, 0.80f, 0.7f, 1.0f };
	float spec[] = { 0.90f, 0.90f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;

	memcpy(candleMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(candleMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(candleMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(candleMesh.mat.emissive, emissive, 4 * sizeof(float));
	candleMesh.mat.shininess = shininess;
	candleMesh.mat.texCount = texcount;

	candle = MyObject(candleMesh, position + MyVec3{0, height / 2, 0}, scaling, {});
	candle.textureOption = MyTextureOption::Candle;
	candle.bumpmapOption = MyBumpMapOption::Candle;

	light->positionVec = MyVec3{position.x, position.y + height + radius, position.z};
}

MyVec3 MyCandle::getPosition() {
	return position;
}

void MyCandle::render(VSShaderLib& shader) {
	candle.render(shader);
}

MyBillboardTree::MyBillboardTree() {}
MyBillboardTree::MyBillboardTree(MyVec3 positionTemp, float sizeTemp) {

	position = positionTemp;
	size = sizeTemp;

	
	MyMesh billboardTreeMesh = createQuad(size, size);

	float amb[] = { 1.0f, 0.0f, 0.2f, 1.0f };
	float diff[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float tree_spec[] = { 0.05f, 0.2f, 0.2f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float tree_shininess = 10.0f;
	int texcount = 0;

	memcpy(billboardTreeMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(billboardTreeMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(billboardTreeMesh.mat.specular, tree_spec, 4 * sizeof(float));
	memcpy(billboardTreeMesh.mat.emissive, emissive, 4 * sizeof(float));
	billboardTreeMesh.mat.shininess = tree_shininess;
	billboardTreeMesh.mat.texCount = texcount;

	billboardTree = MyObject(billboardTreeMesh, position + MyVec3{ 0, size / 2, 0 }, scaling, {});
	billboardTree.textureOption = MyTextureOption::Tree;

}

void MyBillboardTree::render(VSShaderLib& shader) {
	billboardTree.render(shader);
}

void MyBillboardTree::update(MyVec3 camPosition) {

	float camPositionTransformed[3] = { camPosition.x, camPosition.y, camPosition.z };
	float positionTransformed[3] = { position.x, position.y, position.z };
	billboardTree.rotateVec = l3dBillboardCylindricalBegin(camPositionTransformed, positionTransformed);
}

MyWaterParticle::MyWaterParticle() {}
MyWaterParticle::MyWaterParticle(MyVec3 positionTemp, MyVec3 velocityTemp, MyVec3 accelarationTemp, float fadeTemp, float sizeTemp) {

	position = positionTemp;
	velocity = velocityTemp;
	accelaration = accelarationTemp;
	fade = fadeTemp;
	size = sizeTemp;


	MyMesh particleMesh = createQuad(size, size);

	float amb[] = { 1.0f, 0.0f, 0.2f, 1.0f };
	float diff[] = { 0.61f, 0.83f, 0.86f, 1.0f };
	float spec[] = { 0.05f, 0.2f, 0.2f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 10.0f;
	int texcount = 0;

	memcpy(particleMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(particleMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(particleMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(particleMesh.mat.emissive, emissive, 4 * sizeof(float));
	particleMesh.mat.shininess = shininess;
	particleMesh.mat.texCount = texcount;

	particle = MyObject(particleMesh, position + MyVec3{ 0, size / 2, 0 }, scaling, {});
	particle.textureOption = MyTextureOption::WaterParticle;

}

void MyWaterParticle::render(VSShaderLib& shader) {
	particle.render(shader);
}

void MyWaterParticle::tick(float elapsedTime) {

	float factor = elapsedTime / (1000.0f / TARGET_FPS);

	// Update Particle Attributes
	velocity = velocity + accelaration * factor;
	position = position + velocity * factor;

	particle.positionVec = position + MyVec3{ 0, size / 2, 0 };
	lifespan = lifespan - fade * factor;

	particle.mesh.mat.diffuse[3] = lifespan;
}

void MyWaterParticle::update(MyVec3 camPosition) {

	float camPositionTransformed[3] = { camPosition.x, camPosition.y, camPosition.z };
	float positionTransformed[3] = { position.x, position.y, position.z };
	particle.rotateVec = l3dBillboardSphericalBegin(camPositionTransformed, positionTransformed);
}

bool MyWaterParticle::isDead() { return lifespan <= 0.0f;  }

void MyWaterParticle::revive(MyVec3 positionTemp, MyVec3 velocityTemp, MyVec3 accelarationTemp, float fadeTemp) {

	position = positionTemp;
	velocity = velocityTemp;
	accelaration = accelarationTemp;
	fade = fadeTemp;

	lifespan = 1.0f;
}

MySkyBox::MySkyBox() {}
MySkyBox::MySkyBox(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp) {

	MyMesh skyBoxMesh = createCube();

	float amb[] = { 1.0f, 0.55f, 0.0f, 1.0f };
	float diff[] = { 0.8f, 0.40f, 0.0f, 1.0f };
	float spec[] = { 0.3f, 0.15f, 0.0f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 50.0f;
	int texcount = 0;

	memcpy(skyBoxMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(skyBoxMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(skyBoxMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(skyBoxMesh.mat.emissive, emissive, 4 * sizeof(float));
	skyBoxMesh.mat.shininess = shininess;
	skyBoxMesh.mat.texCount = texcount;

	skyBox = MyObject(skyBoxMesh, initialPositionTemp, initialScaleTemp, {});
	skyBox.textureOption = MyTextureOption::SkyBox;
}

void MySkyBox::render(VSShaderLib& shader) {

	glDepthMask(GL_FALSE);
	glFrontFace(GL_CW);

	pushMatrix(VIEW);

	mMatrix[VIEW][12] = 0.0f;
	mMatrix[VIEW][13] = 0.0f;
	mMatrix[VIEW][14] = 0.0f;

	skyBox.render(shader);

	popMatrix(VIEW);

	glFrontFace(GL_CCW);
	glDepthMask(GL_TRUE);
}

MyCubeReflector::MyCubeReflector() {}
MyCubeReflector::MyCubeReflector(MyVec3 initialPositionTemp, MyVec3 initialScaleTemp) {

	MyMesh cubeMesh = createCube();

	float amb[] = { 1.0f, 0.55f, 0.0f, 1.0f };
	float diff[] = { 0.8f, 0.40f, 0.0f, 1.0f };
	float spec[] = { 0.3f, 0.15f, 0.0f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 50.0f;
	int texcount = 0;

	memcpy(cubeMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(cubeMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(cubeMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(cubeMesh.mat.emissive, emissive, 4 * sizeof(float));
	cubeMesh.mat.shininess = shininess;
	cubeMesh.mat.texCount = texcount;

	std::vector<MyVec3Rotation> rotations = {};

	cube = MyObject(cubeMesh, initialPositionTemp, initialScaleTemp, rotations);
	cube.textureOption = MyTextureOption::CubeReflector;
}

void MyCubeReflector::render(VSShaderLib& shader) {
	cube.render(shader);
}


MyPuddle::MyPuddle() {}
MyPuddle::MyPuddle(MyVec3 initialPositionTemp, float heightTemp, float radiusTemp) {

	MyMesh puddleMesh = createCylinder(heightTemp, radiusTemp, 25);

	float amb[] = { 1.0f, 0.55f, 0.0f, 1.0f };
	float diff[] = { 0.8f, 0.40f, 0.0f, 1.0f };
	float spec[] = { 0.3f, 0.15f, 0.0f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 50.0f;
	int texcount = 0;

	memcpy(puddleMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(puddleMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(puddleMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(puddleMesh.mat.emissive, emissive, 4 * sizeof(float));
	puddleMesh.mat.shininess = shininess;
	puddleMesh.mat.texCount = texcount;

	std::vector<MyVec3Rotation> rotations = {};
	puddle = MyObject(puddleMesh, initialPositionTemp, MyVec3{ 1, 1, 1 }, rotations);
}

void MyPuddle::render(VSShaderLib& shader) {
	puddle.render(shader);
}