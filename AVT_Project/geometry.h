#ifndef GEOMETRY_H
#define GEOMETRY_H

#define MAX_TEXTURES 8

struct MyVec3 {
	float x;
	float y;
	float z;

	inline MyVec3 operator+(MyVec3 a) {
		return { a.x + x, a.y + y, a.z + z };
	}

	inline MyVec3 operator-(MyVec3 a) {
		return { x - a.x, y - a.y, z - a.z };
	}

	inline MyVec3 operator*(MyVec3 a) {
		return { a.x * x, a.y * y, a.z * z };
	}

	inline MyVec3 operator*(float a) {
		return { a * x, a * y, a * z };
	}

	inline MyVec3 operator/(MyVec3 a) {
		return { x / a.x, y / a.y , z / a.z};
	}
	
	inline MyVec3 normalize() {

		float length = sqrtf(pow(x, 2) + pow(y, 2) + pow(z, 2));
		return { x / length, y / length, z / length };
	}
};


struct MyVec3Rotation {
	float angle;
	float x;
	float y;
	float z;
};

enum texType {DIFFUSE, SPECULAR, NORMALS, BUMP};

struct Material{
	float diffuse[4];
	float ambient[4];
	float specular[4];
	float emissive[4];
	float shininess;
	int texCount;
};
// A model can be made of many meshes. Each is stored  in the following structure
struct MyMesh {
		GLuint vao;
		GLuint texUnits[MAX_TEXTURES];
		texType texTypes[4];
		float transform[16];
		GLuint numIndexes;
		unsigned int type;
		struct Material mat;
	};

MyMesh createCube();
MyMesh createQuad(float size_x, float size_y);
MyMesh createSphere(float radius, int divisions);
MyMesh createTorus(float innerRadius, float outerRadius, int rings, int sides);
MyMesh createCylinder(float height, float radius, int sides);
MyMesh createCone(float height, float baseRadius, int sides);
MyMesh createPawn();
MyMesh computeVAO(int numP, float *p, float *pfloatoints, int sides, float smoothCos);
int revSmoothNormal2(float *p, float *nx, float *ny, float smoothCos, int beginEnd);
float *circularProfile(float minAngle, float maxAngle, float radius, int divisions, float transX= 0.0f, float transY = 0.0f);
void ComputeTangentArray(int vertexCount, float *vertex, float *normal, float *texcoord, GLuint indexesCount, GLuint *faceIndex, float *tangent);

#endif