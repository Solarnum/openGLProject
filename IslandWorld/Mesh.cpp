#define _CRT_SECURE_NO_WARNINGS

#include "Mesh.h"
#include "SDL.h"
#include "imageloader.h"

#define TWOPI 6.283185

//Mesh methods
Mesh::Mesh(){
	numVerts = numFaces = numNorms = 0;
	pt = NULL; norm = NULL; face = NULL;
	lastVertUsed = lastNormUsed = lastFaceUsed = -1;
	scale = 1.0;
	mode = MODE_WIRE;
}
void Mesh::freeMesh()
{ // free up memory used by this mesh.
	delete[] pt; // release whole vertex list
	delete[] norm;
	for (int f = 0; f < numFaces; f++)
		delete[] face[f].vert; // delete the vert[] array of this face
	delete[] face;
}
int Mesh::isEmpty()
{
	return (numVerts == 0) || (numFaces == 0) || (numNorms == 0);
}
void Mesh::makeEmpty()
{
	numVerts = numFaces = numNorms = 0;
}

void Mesh::drawOpenGL()
{
	tellMaterialsGL();
	glPushMatrix();
	if (usesTexture)
	{
		if (!textureLoaded)
		{
			cout << " \n TextureFileName = " << texturefile << endl;
		setTexture(loadTexture(texturefile));
		}
			
		glBindTexture(GL_TEXTURE_2D, texture);
		glEnable(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	glMultMatrixf(transf.m);
	switch (mode) {
	case MODE_WIRE:
		drawEdges();
		break;
	case MODE_SOLID:
		drawFaces();
		break;
	default:
		drawFaces();
		drawEdges();
		break;
	}
	
	glPopMatrix();
}

Mesh::Mesh(string fname){ // read this file to build mesh
	numVerts = numFaces = numNorms = 0;
	pt = NULL; norm = NULL; face = NULL;
	lastVertUsed = lastNormUsed = lastFaceUsed = -1;
	scale = 1.0;
	mode = MODE_WIRE;
	readMesh(fname);
}

Vector3 Mesh::newell4(int indx[])
{ /* return the normalized normal to face with vertices
  pt[indx[0]],...,pt[indx[3]]. i.e. indx[] contains the four indices
  into the vertex list to be used in the Newell calculation */
	Vector3 m;
	for (int i = 0; i < 4; i++)
	{
		int next = (i == 3) ? 0 : i + 1; // which index is next?
		int f = indx[i], n = indx[next]; // names for the indices in the pair
		m.x += (pt[f].y - pt[n].y) * (pt[f].z + pt[n].z);
		m.y += (pt[f].z - pt[n].z) * (pt[f].x + pt[n].x);
		m.z += (pt[f].x - pt[n].x) * (pt[f].y + pt[n].y);
	}
	m.normalize();
	return m;
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<< setRenderMode >>>>>>>>>>>>>>>>>>>>>>>>
void Mesh::setRenderMode(RenderMode m)
{
	mode = m;
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<< setScale >>>>>>>>>>>>>>>>>>>>>>>>
void Mesh::setScale(float s)
{
	scale = s;
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<< readMesh >>>>>>>>>>>>>>>>>>>>>>>>
void Mesh::readMesh(string fname)
{
	int pch;
	if (strstr(fname.c_str(), ".obj") != 0){
		cout << "filename = " << fname << endl;
		readObj(fname);
	}
	else{
		fstream inStream;
		inStream.open(fname.c_str(), ios::in); //open needs a c-like string


		if (inStream.fail() || inStream.eof())
		{
			cout << "can't open file or eof: " << fname << endl;
			makeEmpty(); return;
		}
		inStream >> numVerts >> numNorms >> numFaces;
		// make arrays for vertices, normals, and faces
		pt = new Point3[numVerts];        assert(pt != NULL);
		norm = new Vector3[numNorms];     assert(norm != NULL);
		face = new Face[numFaces];        assert(face != NULL);
		for (int i = 0; i < numVerts; i++) 	// read in the vertices
			inStream >> pt[i].x >> pt[i].y >> pt[i].z;
		for (int ii = 0; ii < numNorms; ii++) 	// read in the normals
			inStream >> norm[ii].x >> norm[ii].y >> norm[ii].z;
		for (int f = 0; f < numFaces; f++)   // read in face data
		{
			inStream >> face[f].nVerts;


			int n = face[f].nVerts;
			face[f].vert = new VertexID[n]; assert(face[f].vert != NULL);
			for (int k = 0; k < n; k++) 		// read vertex indices for this face
				inStream >> face[f].vert[k].vertIndex;
			for (int kk = 0; kk < n; kk++) 		// read normal indices for this face
				inStream >> face[f].vert[kk].normIndex;
		}



		inStream.close();
	}
} // end of readMesh

void Mesh::readInVertices()
{
	FILE* file = fopen(fname.c_str(), "r");
	if (file == NULL){
		printf("Can't open file");
		getchar();
	}
	std::vector<Point3> temp_vertices;
	std::vector<Point2> temp_uvs;
	char mtllib[256];
	bool mtllibFound = false;

	while (1){
		char lineHeader[128];
		// read in the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		//read in vertices
		if (strcmp(lineHeader, "v") == 0){
			Point3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0){
			Point2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			//uv.y = (1 - uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "mtllib") == 0)
		{
			mtllibFound = true;
			fscanf(file, "%s\n", mtllib);
		}
		else if (strcmp(lineHeader, "o") == 0)
		{
			char objname[256];
			fscanf(file, "%s\n", objname);
			objectname = objname;
		}
		else if (strcmp(lineHeader, "usemtl") == 0)
		{
			char materialName[256];
			fscanf(file, "%s\n", materialName);

		}
		else{
			// Eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}
	numVerts = temp_vertices.size() + 1;
	numUVs = temp_uvs.size() + 1;

	pt = new Point3[numVerts + 1];        assert(pt != NULL);

	for (unsigned int i = 1; i < numVerts; i++){

		Point3 vertex = temp_vertices[i - 1];
		
		//Point2 uv = temp_uvs[i];

		pt[i].x = vertex.x;
		pt[i].y = vertex.y;
		pt[i].z = vertex.z;
		
		//cout << "Vertex " << i << " = " << pt[i].x << " " << pt[i].y << " " << pt[i].z << endl;
	}

	uv = new Vector3[numUVs + 1];

	for(unsigned int i = 1; i < numUVs; i++)
	{
		Point2 curuv = temp_uvs[i - 1];
		uv[i].x = curuv.x;
		uv[i].y = curuv.y;
	}
	cout << "Vertices parsed" << endl;
	cout << "Objectname = " << objectname << endl;

	if (mtllibFound)
	{
		thread t4(&Mesh::readInMaterial, this, mtllib);
		t4.join();
	}
	
}

void Mesh::readInMaterial(string mtllib)
{
	mtllib = "models/" + mtllib;
	cout << "\nReading In Materials "<< mtllib << "\n" << endl;
	FILE* file = fopen(mtllib.c_str(), "r");
	if (file == NULL){
		printf("Can't open file\n");
		cout << "Filename = " << mtllib << endl;
		getchar();
	}
	char materialName[256];
	float specularExponent;
	Point3 ambientReflectivity;
	Point3 diffuseReflectivity;
	Point3 specularReflectivity;
	float dissolve;
	float opticalDensity;
	int illumination;
	string textureMap;
	


	while (1){
		char lineHeader[128];
		// read in the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		//read in vertices
		if (strcmp(lineHeader, "newmtl") == 0){

			fscanf(file, "%s\n", materialName);
		}
		else if (strcmp(lineHeader, "Ns") == 0)
		{
			fscanf(file, "%f\n", &specularExponent);
		}
		else if (strcmp(lineHeader, "Ka") == 0)
		{
			fscanf(file, "%f %f %f\n", &ambientReflectivity.x, &ambientReflectivity.y, &ambientReflectivity.z);
		}
		else if (strcmp(lineHeader, "Kd") == 0)
		{
			fscanf(file, "%f %f %f\n", &diffuseReflectivity.x, &diffuseReflectivity.y, &diffuseReflectivity.z);
		}
		else if (strcmp(lineHeader, "Ks") == 0)
		{
			fscanf(file, "%f %f %f\n", &specularReflectivity.x, &specularReflectivity.y, &diffuseReflectivity.z);
		}
		else if (strcmp(lineHeader, "Ni") == 0)
		{
			fscanf(file, "%f\n", &opticalDensity);
		}
		else if (strcmp(lineHeader, "d") == 0)
		{
			fscanf(file, "%f\n", &dissolve);
		}
		else if (strcmp(lineHeader, "illum") == 0)
		{
			fscanf(file, "%d\n", &illumination);
		}
		else if (strcmp(lineHeader, "map_Kd") == 0)
		{
			usesTexture = true;
			//char texturemap[256];
			texturefile = new char[256];
			fscanf(file, "%s\n", texturefile);
			//string tname = "models/" + *texturemap;
			cout << "Texture Name = " << texturefile << endl;
			
			//texturefile = texturemap;
			//setTexture(loadTexture(texturemap));
		}

		
		mtrl.ambient.set(ambientReflectivity.x, ambientReflectivity.y, ambientReflectivity.z);
		mtrl.diffuse.set(diffuseReflectivity.x, diffuseReflectivity.y, diffuseReflectivity.z);
		mtrl.specular.set(specularReflectivity.x, specularReflectivity.y, diffuseReflectivity.z);
		mtrl.emissive.set(specularReflectivity.x, specularReflectivity.y, diffuseReflectivity.z);
		mtrl.specularExponent = specularExponent;
		
		
	}

	//cout << "mtllib parsed - "<< texturemap << endl;
}

void Mesh::readInNormals()
{
	FILE* file = fopen(fname.c_str(), "r");
	if (file == NULL){
		printf("Can't open file");
		getchar();
	}
	vector<Point3> temp_normals;
	while (1)
	{
		char lineHeader[128];
		// read in the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		if (strcmp(lineHeader, "vn") == 0){
			Point3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else{
			// Eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}
	numNorms = temp_normals.size() + 1;
	cout << "Num norms = " << numNorms << endl;
	norm = new Vector3[numNorms + 1];     assert(norm != NULL);

	for (unsigned int i = 1; i < numNorms; i++)
	{
		Point3 normal = temp_normals[i - 1];
		norm[i].x = normal.x;
		norm[i].y = normal.y;
		norm[i].z = normal.z;
	}
	cout << "Normals parsed" << endl;
}

void Mesh::readInFaces()
{
	std::vector<unsigned int*> vertfaces, normfaces;
	FILE* file = fopen(fname.c_str(), "r");
	if (file == NULL){
		printf("Can't open file");
		getchar();
	}
	while (1)
	{
		char lineHeader[128];
		// read in the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		if (strcmp(lineHeader, "f") == 0){

			unsigned int* temp_face;
			temp_face = new unsigned int[9];
			unsigned int* temp_unused = new unsigned int[3];

			//Scans in face vertices and norms -- it goes face, norm, face, norm, face, norm
			//int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &temp_face[0], &temp_face[1], &temp_face[2], &temp_face[3], &temp_face[4], &temp_face[5]);
			//int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &temp_face[0], &temp_unused[0], &temp_face[3], &temp_face[1], &temp_unused[1], &temp_face[4], &temp_face[2], &temp_unused[1], &temp_face[5]);
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &temp_face[0], &temp_face[6], &temp_face[3], &temp_face[1], &temp_face[7], &temp_face[4], &temp_face[2], &temp_face[8], &temp_face[5]);
			if (matches == 9){
				vertfaces.push_back(temp_face);
				//tx.push_back(temp_unused);
			}
		}
		else{
			// Eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}
	
	numFaces = vertfaces.size();
	face = new Face[numFaces];        assert(face != NULL);
	for (unsigned int i = 0; i < numFaces; i++){
		face[i].nVerts = 3;
		face[i].vert = new VertexID[3]; assert(face[i].vert != NULL);
		//Save the 3 face vertices to the mesh object

		for (unsigned int j = 0; j < 9; j++){
			if (j < 3)
			{
				face[i].vert[j % 3].vertIndex = vertfaces[i][j];
			}
			else if (j < 6)
			{
				face[i].vert[j % 3].normIndex = vertfaces[i][j];
			}
			else
			{
				face[i].vert[j % 3].uvIndex = vertfaces[i][j];
			}
		}

		

		//Failsafe
		/*
		face[i].vert[0].vertIndex = vertfaces[i][0];
		face[i].vert[1].vertIndex = vertfaces[i][2];
		face[i].vert[2].vertIndex = vertfaces[i][4];

		face[i].vert[0].normIndex = vertfaces[i][1];
		face[i].vert[1].normIndex = vertfaces[i][3];
		face[i].vert[2].normIndex = vertfaces[i][5];
		*/


	}
}

void Mesh::readObj(string fnamek){

	fname = fnamek;
	/* Starts three threads to begin reading in OBJ data concurrently*/
	thread t1(&Mesh::readInVertices, this);
	thread t2(&Mesh::readInNormals, this);
	thread t3(&Mesh::readInFaces, this);
	t1.join();
	t2.join();
	t3.join();

	cout << fnamek << " Finished Parsing! :: " << "numNorms = " << numNorms << " numFaces = " << numFaces << " VertexIndcies = " << numVerts << endl;
	
}

unsigned int Mesh::loadTexture(const char * filename)
{
	Image* image = loadBMP(filename);
	GLuint textureId;
	glGenTextures(1, &textureId); //Make room for our texture
	glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
	//Map the image to the texture
	glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
		0,                            //0 for now
		GL_RGB,                       //Format OpenGL uses for image
		image->width, image->height,  //Width and height
		0,                            //The border of the image
		GL_RGB, //GL_RGB, because pixels are stored in RGB format
		GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
		//as unsigned numbers
		image->pixels);               //The actual pixel data
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	delete(image);
	cout << "Texture Loaded!" << endl;
	textureLoaded = true;
	return textureId; //Returns the id of the texture
}

void Mesh::setTexture(GLuint t)
{ 
	texture = t; 
}

//<<<<<<<<<<<<<<<<<<<<<< drawEdges >>>>>>>>>>>>>>>>>>>>
void Mesh::drawEdges()
{
	if (isEmpty()){
		cout << "mesh is empty :!!!" << endl;
		return; // mesh is empty

	}
	for (int f = 0; f < numFaces; f++)
	{
		int n = face[f].nVerts;
		glBegin(GL_LINE_LOOP);
		for (int v = 0; v < n; v++) {
			int iv = face[f].vert[v].vertIndex; assert(iv >= 0 && iv < numVerts);
			glVertex3f(pt[iv].x*scale, pt[iv].y*scale, pt[iv].z*scale);
			//glTexCoord2f(uv[iv].x*scale, uv[iv].y*scale);
		}
		//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
		glEnd();
	}
	glFlush();
}

int count = 0;
//<<<<<<<<<<<<<<<<<<<<<< drawFaces >>>>>>>>>>>>>>>>>>>>
void Mesh::drawFaces()
{ // draw each face of this mesh using OpenGL: draw each polygon.
	
	if (isEmpty()) return; // mesh is empty
	for (int f = 0; f < numFaces; f++)
	{
		int n = face[f].nVerts;
		glBegin(GL_TRIANGLES);
		for (int v = 0; v < n; v++)
		{
			int in = face[f].vert[v].normIndex;
			//cout << "face[f].vert[v].normIndex = " << in << "  numNorms = " << numNorms << endl;
			assert(in >= 0 && in < numNorms);
			glNormal3f(norm[in].x, norm[in].y, norm[in].z);

			int iu = face[f].vert[v].uvIndex;
			glTexCoord2f(uv[iu].x, uv[iu].y);

			int iv = face[f].vert[v].vertIndex;
			//cout << "face[f].vert[v].vertIndex = " << iv << "  numVerts = " << numVerts << endl;
			assert(iv >= 0 && iv < numVerts);
			glVertex3f(pt[iv].x*scale, pt[iv].y*scale, pt[iv].z*scale);
			

			
			//if (v < 2)
				//glTexCoord2f(uv[iv].x * scale, uv[iv].y * scale);
			//else
				//glTexCoord2f(tx[iv][2] * scale, tx[iv][0] * scale);
				


			
		}
		glEnd();
	}
	glFlush();
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<< write mesh>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Mesh::writeMesh(char * fname)
{
	cout << "writeMesh" << endl;
	// write this mesh object into a new Chapter 6 format file.
	if (numVerts == 0 || numNorms == 0 || numFaces == 0) return; //empty
	fstream outStream(fname, ios::out); // open the output stream
	if (outStream.fail()) {
		cout << "can't make new file: " << fname << endl;
		return;
	}
	outStream << numVerts << " " << numNorms << " " << numFaces << "\n";
	// write the vertex and vertex normal list
	for (int i = 0; i < numVerts; i++)
		outStream << pt[i].x << " " << pt[i].y << " " << pt[i].z << "\n";
	for (int ii = 0; ii < numNorms; ii++)
		outStream << norm[ii].x << " " << norm[ii].y << " " << norm[ii].z << "\n";
	// write the face data
	for (int f = 0; f < numFaces; f++)
	{
		int n = face[f].nVerts;
		outStream << n << "\n";
		for (int v = 0; v < n; v++)// write vertex indices for this face
			outStream << face[f].vert[v].vertIndex << " ";	outStream << "\n";
		for (int k = 0; k < n; k++)	// write normal indices for this face 
			outStream << face[f].vert[k].normIndex << " "; outStream << "\n";
	}
	outStream.close();
}

//<<<<<<<<<<<<<<<<<<<<<< makeExtruded QuadStripMesh >>>>>>>>>>>>>>>
void Mesh::makeExtrudedQuadStrip(Point2 P[], int numPts, Vector3 w)
{
	// quad strip in list P[0],P[1], P[2], .., of points in xy-plane
	// extrusion is along vector w
	// Original quadstrip should be ordered CW: P0,P1,P3,P2 
	// as seen from pos. z-axis looking at xy-plane.

	int numSegments = numPts / 2 - 1;
	numVerts = numPts * 2; // for all waists = quadrilaterals
	numFaces = numSegments * 4; // each segment has 4 faces
	numNorms = numFaces; // for visibly flat faces

	pt = new Point3[numVerts];  assert(pt); // make space for the lists
	face = new Face[numFaces];    assert(face);
	norm = new Vector3[numNorms]; assert(norm);

	for (int s = 0; s <= numSegments; s++) // for each waist
	{
		int twoI = 2 * s;
		pt[4 * s].set(P[twoI].x, P[twoI].y, 0);
		pt[4 * s + 1].set(P[twoI + 1].x, P[twoI + 1].y, 0);
		pt[4 * s + 2].set(P[twoI + 1].x + w.x, P[twoI + 1].y + w.y, w.z);
		pt[4 * s + 3].set(P[twoI].x + w.x, P[twoI].y + w.y, w.z);
	}
	for (int i = 0; i < numSegments; i++)
	for (int j = 0; j < 4; j++)
	{
		int which = 4 * i + j; // which face
		int i0 = which, i1 = i0 + 4, i3 = 4 * i + (j + 3) % 4, i2 = i3 + 4;
		norm[which] = newell4Pts(pt[i0], pt[i1], pt[i2], pt[i3]);
		norm[which].normalize();
		face[which].vert = new VertexID[4]; assert(face[which].vert != NULL);
		face[which].nVerts = 4;
		face[which].vert[0].vertIndex = i0;// same as norm index
		face[which].vert[0].normIndex = which;
		face[which].vert[1].vertIndex = i1;
		face[which].vert[1].normIndex = which;
		face[which].vert[2].vertIndex = i2;
		face[which].vert[2].normIndex = which;
		face[which].vert[3].vertIndex = i3;
		face[which].vert[3].normIndex = which;
	}
} // end, makeExtrudedQuadStrip

//######################### MAKE SMOOTH TUBE ######################
void Mesh::makeSmoothTube(FuncPtr spineX, FuncPtr spineY, FuncPtr spineZ, double tMax)
{
	// make tube: wrap polygon about spine
	int numSpinePts = 200;
	int numSlices = 12; // # vrtices in polygon
	float radius = 1.0;
	float fractForInnerOuterRadii = 0.4f; // use inner-outer radii n-gon for polygon

	//---------------------------- make space for lists -----------------
	numVerts = numSlices *  numSpinePts; // total # vertices in mesh
	numFaces = numSlices * (numSpinePts - 1);       // total # faces in mesh
	numNorms = numVerts;
	pt = new Point3[numVerts];  assert(pt != NULL); // make space for the lists
	face = new Face[numFaces];    assert(face != NULL);
	norm = new Vector3[numNorms]; assert(norm != NULL);

	//make polygon that wraps spine:
	Point2 *p = new Point2[numSlices];  assert(p);// place for polygon that wraps
	float ang = 0, delAng = TWOPI / numSlices;
	for (int j = 0; j < numSlices; j++, ang += delAng) // numSlices should be even
	{
		float rad = radius;
		//if(j%2) rad = fractForInnerOuterRadii * radius; //alternating radius
		p[j].x = rad * cos(ang);
		p[j].y = rad * sin(ang);
	}
	double t = 0, delT, eps = 0.0001;
	delT = tMax / (numSpinePts - 1);
	int last = -1;

	// loop over values of t for different spine segments
	for (int k = 0; k < numSpinePts; k++, t += delT) // for each spine point..
	{
		Point3 spine(spineX(t), spineY(t), spineZ(t));
		//make Frenet frame at spine points:
		Vector3 tangent, spinePrimePrime, B, N;
		//do tangent with numerical derivative
		double xderiv = (spineX(t + eps) - spineX(t - eps)) / (2 * eps);
		double yderiv = (spineY(t + eps) - spineY(t - eps)) / (2 * eps);
		double zderiv = (spineZ(t + eps) - spineZ(t - eps)) / (2 * eps);
		tangent.set(xderiv, yderiv, zderiv);
		tangent.normalize();
		// second differences for the acceleration
		double xDoublePrime = (spineX(t - eps) - 2 * spineX(t) + spineX(t + eps)) / (eps*eps);
		double yDoublePrime = (spineY(t - eps) - 2 * spineY(t) + spineY(t + eps)) / (eps*eps);
		double zDoublePrime = (spineZ(t - eps) - 2 * spineZ(t) + spineZ(t + eps)) / (eps*eps);

		spinePrimePrime.set(xDoublePrime, yDoublePrime, zDoublePrime);
		B = tangent.cross(spinePrimePrime); B.normalize();
		N = B.cross(tangent); N.normalize();
		checkOrthogVects(tangent, B, N); // are tangent, B, and n orthonormal?
		//compute vertex list
		for (int j = 0; j < numSlices; j++)
		{
			Vector3 V(p[j].x * B.x + p[j].y * N.x, // construct new vector
				p[j].x * B.y + p[j].y * N.y,
				p[j].x * B.z + p[j].y * N.z);
			pt[++last].set(spine.x + V.x, spine.y + V.y, spine.z + V.z);
			norm[last].set(V.x, V.y, V.z);
			norm[last].normalize();
		} // end: for each polygon vertex at this spine point
	} // end: for each spine segment
	// ----------------- make face lists --------------------
	int ind = -1;
	for (int spinePt = 0; spinePt < numSpinePts - 1; spinePt++)
	{
		for (int f = 0; f < numSlices; f++) // for each face in this segment
		{
			face[++ind].nVerts = 4;
			face[ind].vert = new VertexID[4]; assert(face[ind].vert != NULL);
			int i0 = spinePt * numSlices + f; // first index of the vert in this face
			int i1 = (f == numSlices - 1) ? spinePt * numSlices : (i0 + 1);
			int i2 = i1 + numSlices;
			int i3 = i0 + numSlices;  // last index of the vert in this face
			face[ind].vert[0].vertIndex = face[ind].vert[0].normIndex = i0;
			face[ind].vert[1].vertIndex = face[ind].vert[1].normIndex = i1;
			face[ind].vert[2].vertIndex = face[ind].vert[2].normIndex = i2;
			face[ind].vert[3].vertIndex = face[ind].vert[3].normIndex = i3;
		} // end: for each face in this segment
	} // end: for each spine segment - building face and normal lists
} // end: makeTube()