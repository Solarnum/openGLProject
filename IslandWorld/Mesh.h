#ifndef _MESH
#define _MESH
#include "SDL.h"


class Mesh : public Shape{
public:
	enum RenderMode {
		MODE_WIRE = 0,
		MODE_SOLID,
		MODE_WIRE_SOLID
	};

	string fname; // holds file name for this Mesh


	Mesh();
	Mesh(string fname);

	typedef double(FuncPtr)(double);

	void drawEdges();
	void drawFaces();
	virtual void drawOpenGL();
	void freeMesh();
	int isEmpty();
	void makeEmpty();
	void makeExtrudedQuadStrip(Point2 P[], int numPts, Vector3 w);
	void makeSmoothTube(FuncPtr spineX, FuncPtr spineY, FuncPtr spineZ, double tMax);
	Vector3 newell4(int indx[]);
	void printMesh();
	void readMesh(string fname);
	void readObj(string fname);
	void setRenderMode(RenderMode m);
	void setScale(float s);
	void writeMesh(char* fname);
	void Mesh::readInVertices();
	void Mesh::readInNormals();
	void Mesh::readInFaces();
	void Mesh::readInMaterial(string);
	void Mesh::setTexture(GLuint t);
	GLuint texture;
	bool textureLoaded = false;
private:
	int numVerts, numNorms,numUVs, numFaces;
	Point3 *pt; // array of points
	Vector3 *norm; // array of normals
	Vector3 *uv;
	Face *face; // array of faces
	string mtl;
	string objectname;
	char * texturefile;
	vector<unsigned int*> tx;
	bool usesTexture = false;
	int lastVertUsed;
	int lastNormUsed;
	int lastUvUsed;
	int lastFaceUsed;
	float scale;
	RenderMode mode;
	//loadTexture method
	unsigned int Mesh::loadTexture(const char * filename);
	
	
	
}; // end of Mesh class

#endif
