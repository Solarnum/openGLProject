// SDL.h
// definition of simple support classes:
// 3D Classes for Graphics
#ifndef _SDL
#define _SDL
#include <string>
#include <iostream>
#include <fstream>
#include <strstream>
#include <thread>
#include <vector>

using std::string;
using std::ifstream;
using std::strstream;
using std::cout;
using std::endl;
using std::fstream;
using std::stringstream;
using std::ios;
using std::vector;
using std::thread;
using std::cerr;


#include <windows.h>
#include <assert.h>
#include <math.h>
#include <gl/Gl.h>
#include <gl/glu.h>
#include <GL/glut.h>

// include RGBpixmap if you wish to add pixmap field to Scene:
//#include "RGBpixmap.h"

//############################### class Point2 #######################
class Point2{
public:
	float x, y;

	Point2(float xx, float yy){ x = xx; y = yy; }
	Point2(){ x = y = 0; }

	void set(float xx, float yy){ x = xx; y = yy; }
};

//@@@@@@@@@@@@@@@@@@ Point3 class @@@@@@@@@@@@@@@@
class Point3{
public:
	float x, y, z;

	Point3(float xx, float yy, float zz){ x = xx; y = yy; z = zz; }
	Point3(){ x = y = z = 0; }

	void build4tuple(float v[])
	{
		// load 4-tuple with this color: v[3] = 1 for homogeneous
		v[0] = x; v[1] = y; v[2] = z; v[3] = 1.0f;
	}
	void set(float dx, float dy, float dz){ x = dx; y = dy; z = dz; }
	void set(Point3& p){ x = p.x; y = p.y; z = p.z; }
};

//@@@@@@@@@@@@@@@@@@ Vector3 class @@@@@@@@@@@@@@@@
class Vector3{
public:
	float x, y, z;

	Vector3(float xx, float yy, float zz){ x = xx; y = yy; z = zz; }
	Vector3(Vector3& v){ x = v.x; y = v.y; z = v.z; }
	Vector3(){ x = y = z = 0; } //default constructor
	Vector3 cross(Vector3 b); //return this cross b

	float dot(Vector3 b); // return this dotted with b
	void flip(){ x = -x; y = -y; z = -z; } // reverse this vector
	void normalize();//adjust this vector to unit length
	void set(float dx, float dy, float dz){ x = dx; y = dy; z = dz; }
	void set(const Vector3& v){ x = v.x; y = v.y; z = v.z; }
	void setDiff(Point3& a, Point3& b)//set to difference a - b
	{
		x = a.x - b.x; y = a.y - b.y; z = a.z - b.z;
	}
};

// @@@@@@@@@@@@@@@@@@@@@ Color3 class @@@@@@@@@@@@@@@@
class Color3 { // holds an red,green,blue 3-tuple
public:
	float red, green, blue;

	Color3(){ red = green = blue = 0; }
	Color3(float r, float g, float b){ red = r; green = g; blue = b; }
	Color3(Color3& c){ red = c.red; green = c.green; blue = c.blue; }

	void add(float r, float g, float b)
	{
		red += r; green += g; blue += b;
	}
	void add(Color3& src, Color3& refl);
	void add(Color3& colr);
	void build4tuple(float v[]);
	void set(float r, float g, float b){ red = r; green = g; blue = b; }
	void set(Color3& c)
	{
		red = c.red; green = c.green; blue = c.blue;
	}
};

//@@@@@@@@@@@@@@@@@@@@ light class @@@@@@@@@@@@@@@@@@@
class Light{ // for a linked list of light sources’ color and position
public:
	Point3 pos;
	Color3 color;
	Light* next;

	Light(){ next = NULL; }

	void setColor(Color3 c){ color.set(c); }
	void setPosition(Point3 p){ pos.set(p); }
};

// @@@@@@@@@@@@@@@@@@@@@ Affine4 class @@@@@@@@@@@@@@@@
class Affine4 {
	// manages homogeneous affine transformations
	// including inverse transformations
	// and a stack to put them on
	// used by Scene class to read SDL files
public:
	float m[16]; // hold a 4 by 4 matrix

	Affine4();

	void preMult(Affine4 n);
	void postMult(Affine4 n);
	void set(Affine4 a);
	void setIdentityMatrix();
}; // end of Affine4 class

//@@@@@@@@@@ AffineNode class @@@@@@@@@@@
class AffineNode{
	// used by Scene class to read SDL files
public:
	Affine4 * affn;
	Affine4 * invAffn;
	AffineNode * next;

	AffineNode()
	{
		next = NULL;
		affn = new Affine4; // new affine with identity in it
		invAffn = new Affine4; // and for the inverse
	}

	~AffineNode() //destructor
	{
		delete affn;
		delete invAffn;
	}
};

//@@@@@@@@@@@@@@@@ AffineStack class @@@@@@@@@@@@
class AffineStack{
	// used by Scene class to read SDL files
public:
	AffineNode * tos;

	AffineStack()//default constructor;puts identity on top
	{
		tos = new AffineNode; // node with identity in it
		tos->next = NULL;
	}

	void dup();
	void popAndDrop();
	void releaseAffines(); // pop and drop all remaining items 
	void rotate(float angle, Vector3 u);
	void scale(float sx, float sy, float sz);
	void setIdentity();// make top item the identity matrix
	void translate(Vector3 d);
}; // end of AffineStack class

//this was Shapes.h 
//Shapes class and Supporting classes
//@@@@@@@@@@@@@@@@@ Material class @@@@@@@@@@@@@@
class Material{
public:
	Color3 ambient, diffuse, specular, emissive;
	int numParams; // for textures
	float params[10]; // for textures
	int textureType;  // 0 for none, neg for solids, pos for images
	float specularExponent, reflectivity, transparency, speedOfLight;
	float specularFraction, surfaceRoughness;

	void set(Material& m);
	void setDefault();
}; // end of Material

//@@@@@@@@@@@@@@@@@@@@@ GeomObj class @@@@@@@@@@@@@@@@
class GeomObj{
public:
	GeomObj * next;

	GeomObj() : next(NULL){}

	virtual void drawOpenGL(){}
	virtual void loadStuff(){}
	virtual void tellMaterialsGL(){}
};

//@@@@@@@@@@@@@@@@@@@ Boolean @@@@@@@@@@@@@@@@@@
class Boolean : public GeomObj{
public:
	GeomObj *left, *right;

	Boolean() :left(NULL), right(NULL){}

	virtual void drawOpenGL()
	{ // just draw its children
		if (left)left->drawOpenGL();
		if (right)right->drawOpenGL();
	}
};

//@@@@@@@@@@@@@@@@@@@@ UnionBool @@@@@@@@@@@@@@@@
class UnionBool : public Boolean{
public:
	UnionBool(){ Boolean(); } //constructor
};

//@@@@@@@@@@@@@@@@@@@@ IntersectionBool @@@@@@@@@@@@@@@@
class IntersectionBool : public Boolean{
public:
	IntersectionBool(){ Boolean(); }
};

//@@@@@@@@@@@@@@@@@@@@ DifferenceBool @@@@@@@@@@@@@@@@
class DifferenceBool : public Boolean{
public:
	DifferenceBool(){ Boolean(); }
};

//@@@@@@@@@@@@@@@@@ Shape @@@@@@@@@@@@@@@@@@@@@
class Shape : public GeomObj{
public:
	Material mtrl;
	Affine4 transf, invTransf;
	//virtual Color3 texture(HitInfo& h, int whichTexture);

	Shape(){ mtrl.textureType = 0; mtrl.numParams = 0; }

	virtual void drawOpenGL(){}
	void setMaterial(Material& mt){ mtrl.set(mt); }
	void tellMaterialsGL();
}; //end: Shape class

//@$@$@$@$@$@$@$@$@ Cube class $@$@$@$@$@$@$@$@$@$@
class Cube : public Shape{
public:
	Cube(){}

	void drawOpenGL()
	{
		tellMaterialsGL(); glPushMatrix();
		glMultMatrixf(transf.m); //load affine
		glEnable(GL_NORMALIZE);
		glutSolidCube(2.0);	// a cuber with vertices -1 to +1
		glPopMatrix();
	}
};

//@$@$@$@$@$@$@$@$ Sphere class @$@$@$@$@$@$@$@$@$@$@$@
class Sphere : public Shape{
public:
	Sphere() { }

	void drawOpenGL()
	{
		tellMaterialsGL(); 	glPushMatrix();
		glMultMatrixf(transf.m);
		glutSolidSphere(1.0, 20, 20);
		glPopMatrix();
	}
};

//@$@$@$@$@$@$@$@$@$@ TaperedCylinder class @$@$@$@$@$@$@$@$@$
class TaperedCylinder : public Shape{
private:
	GLUquadricObj * pwall; // OpenGL needs pointers to quadric objects
	GLUquadricObj * pbase;
	GLUquadricObj * pcap;

public:
	float smallRadius;

	TaperedCylinder() // constructor
	{
		//make the quadrics once only.
		pwall = gluNewQuadric(); //make a cylinder
		pbase = gluNewQuadric(); //make a cylinder
		pcap = gluNewQuadric(); //make a cylinder
		gluQuadricDrawStyle(pwall, GLU_FILL);
		gluQuadricDrawStyle(pbase, GLU_FILL);
		gluQuadricDrawStyle(pcap, GLU_FILL);
	}

	void drawOpenGL()
	{
		tellMaterialsGL();
		glPushMatrix();
		glMultMatrixf(transf.m);
		glEnable(GL_NORMALIZE);
		gluCylinder(pwall, 1.0, smallRadius, 1.0, 30, 4); // draw the cylinder wall
		glPushMatrix();
		glScalef(1, 1, -1); //flip base so normals point outside
		gluDisk(pbase, 0, 1, 30, 8); // draw base at z = 0
		glPopMatrix();
		glTranslatef(0, 0, 1);
		gluDisk(pcap, 0, smallRadius, 30, 8); //draw cap at z = 1
		glPopMatrix();
	}
};

//@$@$@$@$@$@$@$@$@$@ Square class @$@$@$@$@$@$@$@$@$
class Square : public Shape{
public:
	Square(){}

	void drawOpenGL(){ /* to be implemented */ }
};

//@$@$@$@$@$@$@$@$@$@ Plane class @$@$@$@$@$@$@$@$@$
class Plane : public Shape{
public:
	Plane() {}

	void drawOpenGL(){ /* to be implemented */ }
};

//################## class VertexID ################
//used to define a Mesh
class VertexID{
public:
	int vertIndex;
	int normIndex;
	int uvIndex;
};

//################# class FACE ##############
//used to define a Mesh
class Face{
public:
	int nVerts;
	VertexID * vert; // array of vertex and normal indices

	Face(){ nVerts = 0; vert = NULL; }
	~Face(){ delete[] vert; nVerts = 0; }
};

//@$@$@$@$@$@$@$@$@$@ Mesh class @$@$@$@$@$@$@$@$@$

//@$@$@$@$@$@$@$@$@$@ Torus class @$@$@$@$@$@$@$@$@$
class Torus : public Shape{
public:
	void drawOpenGL()
	{
		tellMaterialsGL(); 	glPushMatrix();
		glMultMatrixf(transf.m);
		glutSolidTorus(0.2, 1.0, 10, 12);
		//if(doEdges) glutWireTorus(0.2,1.0,10,12);	
		glPopMatrix();
	}
};

//@$@$@$@$@$@$@$@$@$@ Teapot class @$@$@$@$@$@$@$@$@$
class Teapot : public Shape{
public:
	void drawOpenGL()
	{
		tellMaterialsGL();
		glPushMatrix();
		glMultMatrixf(transf.m);
		glutSolidTeapot(1.0);
		glPopMatrix();
	}
};

// this was scene.h
// uses several of the basic classes developed above
// uses classes: DefUnit, DefUnitStack 
// uses Affine4 and associated classes
// uses Shapes class as well
//@@@@@@@@@@@@@@@@@ DefUnit & DefUnitStack classes @@@@@@@@@@@@@@
//used in Scene to read SDL files
class DefUnit {
	// developed by Steve Morin
public:
	string    name, stuff;

	DefUnit(string n, string s) {
		stuff = s;
		name = n;
	}
};

class DefUnitStack {
public:
	DefUnitStack() { stack = NULL; }
	void push(string n, string s);
	void print();
	int search(string s);
	string contents(string s);
	void release();
private:
	struct D4S {
		DefUnit *current;
		struct D4S *next;
	} d4s;
	D4S *stack;
}; // end of DefUnitStack class

//+++++++++++++ TokenType +++++++++++++
enum mTokenType {
	IDENT, LIGHT, ROTATE, TRANSLATE,
	SCALE, PUSH, POP, IDENTITYAFFINE,
	GLOBALAMBIENT, BACKGROUND, MINREFLECTIVITY,
	MINTRANSPARENCY, MAXRECURSIONDEPTH, CUBE, SPHERE, TORUS, PLANE,
	SQUARE, CYLINDER, CONE, TAPEREDCYLINDER, TETRAHEDRON, OCTAHEDRON,
	DODECAHEDRON, ICOSAHEDRON, BUCKYBALL, TEAPOT,
	DIAMOND, UNION, INTERSECTION, DIFFERENCEa, MAKEPIXMAP,
	MESH, DEFAULTMATERIALS, AMBIENT, DIFFUSE, SPECULAR,
	SPECULARFRACTION, SURFACEROUGHNESS, EMISSIVE, SPECULAREXPONENT,
	SPEEDOFLIGHT, TRANSPARENCY, REFLECTIVITY, PARAMETERS, TEXTURE,
	LFTCURLY, RGHTCURLY, DEF, USE, T_NULL, F_EOF, UNKNOWN
};

//@@@@@@@@@@@@@  Scene class @@@@@@@@@@@@@@@@@@@@
class Scene{
public:
	Light *light;  // attach linked list of lights here
	GeomObj * obj;  // attach the object list here
	Color3 background, ambient;
	int maxRecursionDepth;

	//must #include RGBpixmap.h to have following texture fields
	//RGBpixmap pixmap[8]; //list of attached pixmaps

	float minReflectivity, minTransparency;

	Scene() :light(NULL), obj(NULL), tail(NULL) //default constructor
	{
		currMtrl.setDefault();
		background.set(0, 0, 0.6f);
		ambient.set(0.1f, 0.1f, 0.1f);
		minReflectivity = 0.5;
		minTransparency = 0.5;
		maxRecursionDepth = 3;
	}
	Scene(string fname){ Scene(); read(fname); }

	//bool isInShadow(Ray& f); // for ray tracing: implementation left to the reader

	void drawSceneOpenGL();
	GeomObj* getObject();
	void freeScene();
	void makeLightsOpenGL();
	bool read(string fname);

private:
	// private stuff used only for reading a scene
	int line;
	int nextline;
	ifstream  *file_in;
	strstream *f_in;
	strstream temp_fin;
	DefUnitStack *def_stack;
	GeomObj * tail; // tail of object list
	AffineStack affStk; // affine stack
	Material currMtrl;
	string nexttoken(void);
	float getFloat();
	bool isidentifier(string keyword);
	void cleanUp();
	mTokenType whichtoken(string keyword);
}; // end of Scene.h


Vector3 newell4Pts(Point3 a, Point3 b, Point3 c, Point3 d);

void checkOrthogVects(Vector3 a, Vector3 b, Vector3 c);

#endif



