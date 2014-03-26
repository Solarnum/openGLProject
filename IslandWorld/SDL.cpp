// SDL.cpp  , fsh, 11/4/99
// support code for the classes in SDL.h
#define _CRT_SECURE_NO_WARNINGS

#include "SDL.h"
#include "Mesh.h"

#include <vector>
#define TWOPI 6.283185

////////// Geometry functions //////////////////////////

//<<<<<<<<<<<<<<<<<<<<< dot3 >>>>>>>>>>>>>>>
double dot3(Vector3 a, Vector3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

//<<<<<<<<<<<<<<<<<<<<<< checkOrthogVects >>>>>>>>>>>>>>>>
void checkOrthogVects(Vector3 a, Vector3 b, Vector3 c)
{ // check that system is orthonormal
	double aDotb = fabs(dot3(a, b));
	double aDotc = fabs(dot3(a, c));
	double bDotc = fabs(dot3(b, c));
	if (aDotb > 0.000001 || aDotc > 0.000001 || bDotc > 0.000001)
		cout << " Bad!! vectors NOT orthogonal!\n";
	double A = fabs(dot3(a, a) - 1.0);
	double B = fabs(dot3(b, b) - 1.0);
	double C = fabs(dot3(c, c) - 1.0);
#if 0
	if (A > 0.0000001)
		cout << "Bad!! first is not normalized!\n";
	if (B > 0.0000001)
		cout << "Bad!! second is not normalized!\n";
	if (C > 0.0000001)
		cout << "Bad!! third is not normalized!\n";
#endif
}

//<<<<<<<<<<<<<<<<<<<<< SIZESQ >>>>>>>>>>>>>>>>
#define SIZESQ(n) ((n.x)*(n.x)+(n.y)*(n.y)+(n.z)*(n.z))

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< newell4Pts >>>>>>>>>>>>>>>>>>
Vector3 newell4Pts(Point3 a, Point3 b, Point3 c, Point3 d)
{ // don't normalize the vector here
	Vector3 m;
	m.x = (a.y - b.y)*(a.z + b.z) + (b.y - c.y)*(b.z + c.z) + (c.y - d.y)*(c.z + d.z) + (d.y - a.y)*(d.z + a.z);
	m.y = (a.z - b.z)*(a.x + b.x) + (b.z - c.z)*(b.x + c.x) + (c.z - d.z)*(c.x + d.x) + (d.z - a.z)*(d.x + a.x);
	m.z = (a.x - b.x)*(a.y + b.y) + (b.x - c.x)*(b.y + c.y) + (c.x - d.x)*(c.y + d.y) + (d.x - a.x)*(d.y + a.y);
	//cout << " in newell4Pts, sizeSq(m) = " << SIZESQ(m) << endl;
	return m;
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< sec >>>>>>>>>>>>>>>>>>>>
double sec(double t) // trigonometric secant function
{
	double cs = cos(t);
	if (fabs(cs) > 0.000001)
		return 1.0 / cs;
	else return 0;
}

//<<<<<<<<<<<<<<<<<<<<<<<<<< myTan >>>>>>>>>>>>>>
double myTan(double t)
{
	double cs = cos(t), sn = sin(t);
	if (fabs(cs) > 0.0001)
		return sn / cs;
	else return 0.0;
}

// Vector3 methods
Vector3 Vector3::cross(Vector3 b) //return this cross b
{
	Vector3 c(y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x);
	return c;
}
float Vector3::dot(Vector3 b) // return this dotted with b
{
	return x * b.x + y * b.y + z * b.z;
}

void Vector3::normalize()//adjust this vector to unit length
{
	double sizeSq = x * x + y * y + z * z;
	if (sizeSq < 0.0000001)
	{
		cerr << "\nnormalize() sees vector (0,0,0)!";
		return; // does nothing to zero vectors;
	}
	float scaleFactor = 1.0 / (float)sqrt(sizeSq);
	x *= scaleFactor; y *= scaleFactor; z *= scaleFactor;
}

// Color3 methods
void Color3::add(Color3& src, Color3& refl)
{ // add the product of source color and reflection coefficient
	red += src.red   * refl.red;
	green += src.green * refl.green;
	blue += src.blue  * refl.blue;
}
void Color3::add(Color3& colr)
{ // add colr to this color
	red += colr.red; green += colr.green; blue += colr.blue;
}

void Color3::build4tuple(float v[])
{// load 4-tuple with this color: v[3] = 1 for homogeneous
	v[0] = red; v[1] = green; v[2] = blue; v[3] = 1.0f;
}


//Affine4 methods
Affine4::Affine4(){ // make identity transform
	m[0] = m[5] = m[10] = m[15] = 1.0;
	m[1] = m[2] = m[3] = m[4] = 0.0;
	m[6] = m[7] = m[8] = m[9] = 0.0;
	m[11] = m[12] = m[13] = m[14] = 0.0;
}
void Affine4::setIdentityMatrix(){ // make identity transform
	m[0] = m[5] = m[10] = m[15] = 1.0;
	m[1] = m[2] = m[3] = m[4] = 0.0;
	m[6] = m[7] = m[8] = m[9] = 0.0;
	m[11] = m[12] = m[13] = m[14] = 0.0;
}
void Affine4::set(Affine4 a)// set this matrix to a
{
	for (int i = 0; i < 16; i++)
		m[i] = a.m[i];
}
//<<<<<<<<<<<<<< preMult >>>>>>>>>>>>
void Affine4::preMult(Affine4 n)
{// postmultiplies this with n
	float sum;
	Affine4 tmp;
	tmp.set(*this); // tmp copy
	// following mult's : this = tmp * n
	for (int c = 0; c < 4; c++)
	for (int r = 0; r < 4; r++)
	{
		sum = 0;
		for (int k = 0; k < 4; k++)
			sum += n.m[4 * k + r] * tmp.m[4 * c + k];
		m[4 * c + r] = sum;
	}// end of for loops
}// end of preMult()	
//<<<<<<<<<<<< postMult >>>>>>>>>>>
void Affine4::postMult(Affine4 n){// postmultiplies this with n
	float sum;
	Affine4 tmp;
	tmp.set(*this); // tmp copy
	for (int c = 0; c < 4; c++)// form this = tmp * n
	for (int r = 0; r < 4; r++)
	{
		sum = 0;
		for (int k = 0; k < 4; k++)
			sum += tmp.m[4 * k + r] * n.m[4 * c + k];
		m[4 * c + r] = sum;
	}// end of for loops
}

// AffineStack methods
void AffineStack::dup()
{
	AffineNode* tmp = new AffineNode;
	tmp->affn = new Affine4(*(tos->affn));
	tmp->invAffn = new Affine4(*(tos->invAffn));
	tmp->next = tos;
	tos = tmp;
}
void AffineStack::setIdentity() // make top item the identity matrix
{
	assert(tos != NULL);
	tos->affn->setIdentityMatrix();
	tos->invAffn->setIdentityMatrix();
}
void AffineStack::popAndDrop()
{
	if (tos == NULL) return; // do nothing
	AffineNode *tmp = tos;
	tos = tos->next;
	delete tmp; // should call destructor, which deletes trices
}
void AffineStack::releaseAffines()
{ // pop and drop all remaining items 
	while (tos) popAndDrop();
}
void AffineStack::rotate(float angle, Vector3 u)
{
	Affine4 rm; // make identity matrix
	Affine4 invRm;
	u.normalize(); // make the rotation axis unit length
	float ang = angle * 3.14159265 / 180; // deg to  
	float c = cos(ang), s = sin(ang);
	float mc = 1.0 - c;
	//fill the 3x3 upper left matrix - Chap.5 p. 29
	rm.m[0] = c + mc * u.x * u.x;
	rm.m[1] = mc * u.x * u.y + s * u.z;
	rm.m[2] = mc * u.x * u.z - s * u.y;
	rm.m[4] = mc * u.y * u.x - s * u.z;
	rm.m[5] = c + mc * u.y * u.y;
	rm.m[6] = mc * u.y * u.z + s * u.x;
	rm.m[8] = mc * u.z * u.x + s * u.y;
	rm.m[9] = mc * u.z * u.y - s * u.x;
	rm.m[10] = c + mc * u.z * u.z;
	// same for inverse : just sign of s is changed
	invRm.m[0] = c + mc * u.x * u.x;
	invRm.m[1] = mc * u.x * u.y - s * u.z;
	invRm.m[2] = mc * u.x * u.z + s * u.y;
	invRm.m[4] = mc * u.y * u.x + s * u.z;
	invRm.m[5] = c + mc * u.y * u.y;
	invRm.m[6] = mc * u.y * u.z - s * u.x;
	invRm.m[8] = mc * u.z * u.x - s * u.y;
	invRm.m[9] = mc * u.z * u.y + s * u.x;
	invRm.m[10] = c + mc * u.z * u.z;
	tos->affn->postMult(rm);
	tos->invAffn->preMult(invRm);
}
void AffineStack::scale(float sx, float sy, float sz)
{ // post-multiply top item by scaling
#define sEps 0.00001
	Affine4 scl;// make an identity
	Affine4 invScl;
	scl.m[0] = sx;
	scl.m[5] = sy;
	scl.m[10] = sz;// adjust it to a scaling matrix
	if (fabs(sx) < sEps || fabs(sy) < sEps || fabs(sz) < sEps)
	{
		cerr << "degenerate scaling transformation!\n";
	}
	invScl.m[0] = 1 / sx; invScl.m[5] = 1 / sy; invScl.m[10] = 1 / sz;
	tos->affn->postMult(scl); //
	tos->invAffn->preMult(invScl);
}
void AffineStack::translate(Vector3 d)
{
	Affine4 tr; // make identity matrix
	Affine4 invTr;
	tr.m[12] = d.x; tr.m[13] = d.y;	tr.m[14] = d.z;
	invTr.m[12] = -d.x;	invTr.m[13] = -d.y; invTr.m[14] = -d.z;
	tos->affn->postMult(tr);
	tos->invAffn->preMult(invTr);
}

// Material methods
void Material::setDefault(){
	textureType = 0; // for none
	numParams = 0;
	reflectivity = transparency = 0.0;
	speedOfLight = specularExponent = 1.0;
	specularFraction = 0.0;
	surfaceRoughness = 1.0;
	ambient.set(0.1f, 0.1f, 0.1f);
	diffuse.set(0.8f, 0.8f, 0.8f);
	specular.set(0, 0, 0);
	emissive.set(0, 0, 0);
}
void Material::set(Material& m)
{
	textureType = m.textureType;
	numParams = m.numParams;
	for (int i = 0; i < numParams; i++) params[i] = m.params[i];
	transparency = m.transparency;
	speedOfLight = m.speedOfLight;
	reflectivity = m.reflectivity;
	specularExponent = m.specularExponent;
	specularFraction = m.specularFraction;
	surfaceRoughness = m.surfaceRoughness;
	ambient.set(m.ambient);
	diffuse.set(m.diffuse);
	specular.set(m.specular);
	emissive.set(m.emissive);
}

// Shape methods
void Shape::tellMaterialsGL()
{
	float amb[4], diff[4], spec[4], emiss[4];
	float zero[] = { 0, 0, 0, 1 };
	mtrl.ambient.build4tuple(amb); // fill the array
	mtrl.diffuse.build4tuple(diff);
	mtrl.specular.build4tuple(spec);
	mtrl.emissive.build4tuple(emiss);
	glMaterialfv(GL_FRONT/*_AND_BACK*/, GL_AMBIENT, amb);
	glMaterialfv(GL_FRONT/*_AND_BACK*/, GL_DIFFUSE, diff);
	glMaterialfv(GL_FRONT/*_AND_BACK*/, GL_SPECULAR, spec);
	glMaterialfv(GL_FRONT/*_AND_BACK*/, GL_EMISSION, emiss);
	glMaterialf(GL_FRONT/*_AND_BACK*/, GL_SHININESS, mtrl.specularExponent);
}


// Scene methods
//<<<<<<<< methods >>>>>>>>>>>
string Scene::nexttoken(void) //########## nexttoken()
{
	char c;
	string token;
	int lastchar = 1;
	if (!f_in) { return(token); }
	if (f_in->eof()) { return(token); }
	while (f_in->get(c))
	{
		if (f_in->eof()) {
			return(token);
		}
		switch (c) {
		case '\n': nextline += 1;
		case ' ':
		case '\t':
		case '\a':
		case '\b':
		case '\v':
		case '\f':
		case '\r': {
					   if (lastchar == 0) { return(token); }break; }
		case '{': {
					  token = c; return(token); break; }
		case '}': {
					  token = c;
					  return(token);
					  break; }
		case '!': {
					  while (c != '\n' && f_in->get(c)) {
					  }
					  nextline++; break; }
		default: {
					 token = token + c;
					 lastchar = 0;

					 if ((f_in->peek() == '{') ||
						 (f_in->peek() == '}')) {
						 if (lastchar == 0) {
							 return(token);
						 }
						 else {
							 f_in->get(c);
							 token = c;
							 return(token);
						 }
					 }
					 line = nextline;
		}
		}
	}
	return(" ");
}
//<<<<<<<<<<<<<< getFloat >>>>>>>>>>>>>>>>
float Scene::getFloat() //############ getFloat()
{
	strstream tmp;
	float number;
	string str = nexttoken();
	tmp << str;
	if (!(tmp >> number))
	{
		cerr << "Line " << line << ": error getting float" << endl;
		exit(-1);
	}
	else
	{
		char t;
		if ((tmp >> t))
		{
			cerr << "Line " << line << ": bum chars in number" << endl;
			exit(-1);
		}
	}
	return number;
}
//<<<<<<<<<<<<<<<<< isidentifier >>>>>>>>>>>>>>>>
bool Scene::isidentifier(string keyword) { //######## isidentifier
	string temp = keyword;
	if (!isalpha(temp[0])) return(false);
	int count;
	for (count = 1; count < temp.length(); count++) {
		if ((!isalnum(temp[count])) && (temp[count] != '.')) return(false);
	}
	return(true);
}
//<<<<<<<<<<<<<<<< cleanUp >>>>>>>>>>>>>>>>
void Scene::cleanUp() //######### cleanUp
{ // release stuff after parsing file
	affStk.releaseAffines(); 		//delete affine stack
	def_stack->release();
	delete def_stack; // release the DefUnitStack memory
}
//<<<<<<<<<<<<<<<<< freeScene >>>>>>>>>>>>>>
void Scene::freeScene()
{ // release the object and light lists
	GeomObj *p = obj;
	while (p)
	{
		GeomObj* q = p;
		p = p->next;
		delete q;
	}
	Light * q = light;
	while (q)
	{
		Light* r = q;
		q = q->next;
		delete r;
	}
}

//<<<<<<<<<<<<<<<<< makeLightsOpenGL >>>>>>>>>>>>>>>
void Scene::makeLightsOpenGL()
{
	// build lights from scene's light list
	/* in GL.h: GL_LIGHT0..GL_LIGHT7 are defined as consecutive ints from 0x4000 to 0x4007 */

	int num, lightNum = GL_LIGHT0;
	Light *p;
	float colr[4], posn[4]; // arrays to hold openGL color and position
	for (p = light, num = 0; p && num < 8; p = p->next, lightNum++, num++)
	{ // set up to 8 OpenGL lights
		glEnable(lightNum);
		(p->color).build4tuple(colr);
		(p->pos).build4tuple(posn);
		glLightfv(lightNum, GL_DIFFUSE, colr);
		glLightfv(lightNum, GL_SPECULAR, colr);
		glLightfv(lightNum, GL_POSITION, posn);
	}
} // end of makeLightsOpenGL

//<<<<<<<<<<<<<<<<< whichToken >>>>>>>>>>>>>>>
mTokenType Scene::whichtoken(string keyword)
{
	string temp = keyword;
	if (temp == "light")				 return LIGHT;
	if (temp == "rotate")           return ROTATE;
	if (temp == "translate")        return TRANSLATE;
	if (temp == "scale")             return (SCALE);
	if (temp == "push")        		 return (PUSH);
	if (temp == "pop")					 return (POP);
	if (temp == "identityAffine")    return (IDENTITYAFFINE);
	if (temp == "cube")              return (CUBE);
	if (temp == "sphere")            return (SPHERE);
	if (temp == "torus")             return (TORUS);
	if (temp == "plane")             return (PLANE);
	if (temp == "square")            return (SQUARE);
	if (temp == "cylinder")          return (CYLINDER);
	if (temp == "taperedCylinder")   return (TAPEREDCYLINDER);
	if (temp == "cone")              return (CONE);
	if (temp == "tetrahedron")       return (TETRAHEDRON);
	if (temp == "octahedron")        return (OCTAHEDRON);
	if (temp == "dodecahedron")      return (DODECAHEDRON);
	if (temp == "icosahedron")       return (ICOSAHEDRON);
	if (temp == "buckyball")         return (BUCKYBALL);
	if (temp == "diamond")           return (DIAMOND);
	if (temp == "teapot")				 return (TEAPOT);
	if (temp == "union")             return (UNION);
	if (temp == "intersection")      return (INTERSECTION);
	if (temp == "difference")        return (DIFFERENCEa);
	if (temp == "mesh")              return (MESH);
	if (temp == "makePixmap")        return (MAKEPIXMAP);
	if (temp == "defaultMaterials")  return (DEFAULTMATERIALS);
	if (temp == "ambient")           return (AMBIENT);
	if (temp == "diffuse")           return (DIFFUSE);
	if (temp == "specular")          return (SPECULAR);
	if (temp == "specularFraction")  return (SPECULARFRACTION);
	if (temp == "surfaceRoughness")  return (SURFACEROUGHNESS);
	if (temp == "emissive")          return (EMISSIVE);
	if (temp == "specularExponent")  return (SPECULAREXPONENT);
	if (temp == "speedOfLight")      return (SPEEDOFLIGHT);
	if (temp == "transparency")      return (TRANSPARENCY);
	if (temp == "reflectivity")      return (REFLECTIVITY);
	if (temp == "parameters")        return (PARAMETERS);
	if (temp == "texture")				 return (TEXTURE);
	if (temp == "globalAmbient")	    return (GLOBALAMBIENT);
	if (temp == "minReflectivity")	 return (MINREFLECTIVITY);
	if (temp == "minTransparency")	 return (MINTRANSPARENCY);
	if (temp == "maxRecursionDepth") return (MAXRECURSIONDEPTH);
	if (temp == "background")        return (BACKGROUND);
	if (temp == "{")                 return (LFTCURLY);
	if (temp == "}")                 return (RGHTCURLY);
	if (temp == "def")               return (DEF);
	if (temp == "use")               return (USE);
	if (temp == " ")                return (T_NULL);
	if (isidentifier(temp))         return (IDENT);
	cout << temp << ":" << temp.length() << endl;
	return(UNKNOWN);
} // end of whichtoken

//<<<<<<<<<<  drawSceneOpenGL >>>>>>>>>>>>>>>>.
void Scene::drawSceneOpenGL()
{ //draw each object on object list
	for (GeomObj* p = obj; p; p = p->next)
		p->drawOpenGL(); //draw it
}
//<<<<<<<<<<<<<<< Scene :: read >>>>>>>>>>>>>>>>
bool Scene::read(string fname)// return true if ok; else false
{
	file_in = new ifstream(fname.c_str());
	if (!(*file_in))
	{
		cout << "I can't find or open file: " << fname << endl;
		return false;
	}
	f_in = new strstream();
	line = nextline = 1;
	def_stack = new DefUnitStack();
	char ch;
	freeScene(); //delete any previous scene	
	// initialize all for reading:
	obj = tail = NULL;
	light = NULL;
	affStk.tos = new AffineNode;
	affStk.tos->next = NULL;

	while (file_in->get(ch)) { *f_in << ch; } // read whole file
	while (1) //read file, collecting objects, until EOF or an error
	{
		GeomObj * shp = getObject(); // get the next shape
		if (!shp) break; // no object: either error or EOF
		shp->next = NULL; // to be safe
		if (obj == NULL){ obj = tail = shp; } // empty list so far
		else{ tail->next = shp; tail = shp; } // add new object to queue
	}
	file_in->close();
	cleanUp(); // delete temp lists, etc.
	return true;
} // end of read()

//<<<<<<<<<<<<<< Scene :: getObject >>>>>>>>>>>>>>>	
GeomObj* Scene::getObject()
{ //reads tokens from stream f_in (a data member of Scene),
	// building lights, getting materials, doing transformations,
	// until it finds a new object
	// returns NULL if any error occurs, or end of file
	string s;
	GeomObj * newShape = NULL;
	mTokenType typ;
	while ((typ = (whichtoken(s = nexttoken()))) != T_NULL)
	{
		if (typ == UNION || typ == INTERSECTION || typ == DIFFERENCEa)
		{
			switch (typ)
			{
			case UNION:				newShape = new UnionBool();	break;
			case INTERSECTION:	newShape = new IntersectionBool();	break;
			case DIFFERENCEa:		newShape = new DifferenceBool(); break;
			} // end of little switch
			GeomObj* p = newShape;

			p = getObject(); // get left child
			if (!p) return NULL; // Error! should always get an object
			((Boolean*)newShape)->left = p; // hook it up
			p = getObject();// get right child
			if (!p) return NULL;
			((Boolean*)newShape)->right = p; // hook it up
			return newShape;
		}// end of if(typ == UNION etc....
		switch (typ)
		{
		case LIGHT: {
						Point3 p;
						Color3 c;
						p.x = getFloat(); p.y = getFloat();	p.z = getFloat();
						c.red = getFloat(); c.green = getFloat();	c.blue = getFloat();
						Light *l = new Light;
						l->setPosition(p);
						l->setColor(c);
						l->next = light; //put it on the list
						light = l; break; }
		case ROTATE: {
						 float angle;
						 Vector3 u;
						 angle = getFloat(); u.x = getFloat();
						 u.y = getFloat(); u.z = getFloat();
						 affStk.rotate(angle, u); break; }
		case TRANSLATE: {
							Vector3 d;
							d.x = getFloat(); d.y = getFloat(); d.z = getFloat();
							affStk.translate(d); break; }
		case SCALE: {
						float sx, sy, sz;
						sx = getFloat(); sy = getFloat(); sz = getFloat();
						affStk.scale(sx, sy, sz); break; }
		case PUSH: affStk.dup(); break;
		case POP:  affStk.popAndDrop(); break;
		case IDENTITYAFFINE: affStk.setIdentity(); break;
		case AMBIENT: {
						  float dr, dg, db;
						  dr = getFloat(); dg = getFloat(); db = getFloat();
						  currMtrl.ambient.set(dr, dg, db); break; }
		case DIFFUSE: {
						  float dr, dg, db;
						  dr = getFloat(); dg = getFloat(); db = getFloat();
						  currMtrl.diffuse.set(dr, dg, db); break; }
		case SPECULAR:{
						  float dr, dg, db;
						  dr = getFloat(); dg = getFloat(); db = getFloat();
						  currMtrl.specular.set(dr, dg, db); break; }
		case EMISSIVE: {
						   float dr, dg, db;
						   dr = getFloat(); dg = getFloat(); db = getFloat();
						   currMtrl.emissive.set(dr, dg, db); break; }
		case PARAMETERS: { // get a list of numParams parameters
							 currMtrl.numParams = (int)getFloat();
							 for (int i = 0; i < currMtrl.numParams; i++)
								 currMtrl.params[i] = getFloat();
							 break; }
		case SPECULARFRACTION: currMtrl.specularFraction = getFloat(); break;
		case SURFACEROUGHNESS: currMtrl.surfaceRoughness = getFloat(); break;
		case TEXTURE: { // get type, 0 for none
						  currMtrl.textureType = getFloat(); }
			break;
		case DEFAULTMATERIALS: 	currMtrl.setDefault(); break;
		case SPEEDOFLIGHT: currMtrl.speedOfLight = getFloat(); break;
		case SPECULAREXPONENT: currMtrl.specularExponent = getFloat(); break;
		case TRANSPARENCY:currMtrl.transparency = getFloat(); break;
		case REFLECTIVITY: currMtrl.reflectivity = getFloat(); break;
		case GLOBALAMBIENT:
			ambient.red = getFloat(); ambient.green = getFloat();
			ambient.blue = getFloat(); break;
		case BACKGROUND:
			background.red = getFloat();
			background.green = getFloat();
			background.blue = getFloat(); break;
		case MINREFLECTIVITY: minReflectivity = getFloat(); break;
		case MINTRANSPARENCY:minTransparency = getFloat(); break;
		case MAXRECURSIONDEPTH: maxRecursionDepth = getFloat(); break;
		case MAKEPIXMAP: {	// get BMP file name for a pixmap			
							 /* to be implemented, along the lines:
							 int which = getFloat();// index of this pixmap in pixmap array
							 if(which < 0 || which > 7){cout << "\nbad index of RGBpixmap!\n";}
							 string fname = nexttoken(); // get file name for mesh
							 cout << "I got fname = " << fname << endl;
							 if(!pixmap[which].readBMPFile(fname))
							 {// read BMP file into this pixmap
							 cout << " \ncan't read that RGBpixmap file!\n";
							 return NULL;  }  */
							 break; }// end of case: MAKEPIXMAP
		case T_NULL: break; // The null token represents end-of-file 
		case DEF: {
					  string name, temp, lb, rb;
					  int l = line;
					  string inp;
					  name = nexttoken();
					  if (whichtoken(name) != IDENT) {
						  cout << "Error:  Identifier expected." << endl;
						  return NULL;
					  }
					  if (def_stack->search(name)) {
						  cout << line << ": " << name;
						  cout << ": attempt to redefine. " << endl;
						  return NULL;
					  }
					  lb = nexttoken();
					  if (whichtoken(lb) != LFTCURLY) {
						  cout << "Error: { expected." << endl;
						  return NULL;
					  }
					  while (whichtoken(temp = nexttoken()) != RGHTCURLY) {
						  cout << temp << endl;
						  inp = inp + temp + " ";
						  if (!f_in) {
							  cout << "Error: end of file detected." << endl;
							  return NULL;
						  }
					  }
					  // Push the contents of the string on the stack.
					  def_stack->push(name, inp);
					  break; } // end of case: DEF
		case USE: {
					  string name;
					  name = nexttoken();
					  if (whichtoken(name) != IDENT) {
						  cout << line << ": " << name;
						  cout << ": identifier expected.";
						  return NULL;
					  }
					  if (!def_stack->search(name)) {
						  cout << line << ": " << name;
						  cout << ": not defined.";
						  return NULL;
					  }
					  cout << def_stack->contents(name) << endl;
					  strstream *temp_fin = new strstream;
					  *temp_fin << def_stack->contents(name) << " ";
					  *temp_fin << f_in->rdbuf();
					  delete (f_in);
					  f_in = temp_fin;
					  break; } // end of case: USE
		default:  { // inner switch for Shapes
					  switch (typ)
					  {
						  //float param;
					  case CUBE:			newShape = new Cube; break;
					  case SPHERE:		newShape = new Sphere; break;
					  case TETRAHEDRON:	newShape = new Mesh("tetra.3vn"); break;
					  case TORUS:		newShape = new Torus; break;
					  case PLANE:		newShape = new Plane; break;
					  case SQUARE:		newShape = new Square; break;
					  case TAPEREDCYLINDER:	newShape = new TaperedCylinder;
						  ((TaperedCylinder*)newShape)->smallRadius = getFloat(); break;
					  case CONE:			newShape = new TaperedCylinder;
						  ((TaperedCylinder*)newShape)->smallRadius = 0; break;
					  case CYLINDER:		newShape = new TaperedCylinder;
						  ((TaperedCylinder*)newShape)->smallRadius = 1; break;
					  case OCTAHEDRON:		newShape = new Mesh("octa.3vn"); break;
					  case DODECAHEDRON:	newShape = new Mesh("dodeca.3vn"); break;
					  case ICOSAHEDRON:	newShape = new Mesh("icosa.3vn"); break;
					  case BUCKYBALL:		newShape = new Mesh("bucky.3vn");	break;
					  case DIAMOND:		newShape = new Mesh("diamond.3vn"); break;
					  case TEAPOT:		newShape = new Teapot; break;
					  case MESH: {// get a filename (with extension) for this mesh		
									 string fname = nexttoken(); // get file name for mesh
									 newShape = new Mesh(fname); break;
					  }// end of case: MESH
					  default: {
								   cerr << "Line " << nextline << ": unknown keyword " << s << endl;
								   return NULL;
					  }
					  } // end of inner switch
					  // common things to do to all Shape’s
					  ((Shape*)newShape)->mtrl.set(currMtrl);
					  // load transform and its inverse
					  ((Shape*)newShape)->transf.set(*(affStk.tos->affn));
					  ((Shape*)newShape)->invTransf.set(*(affStk.tos->invAffn));
					  return newShape;
		}// end of default: block			
		} // end of outer switch
	} // end of while
	return NULL;
} // end of getObject

// DefUnitStack methods
void DefUnitStack::push(string n, string s) {
	D4S *temp_d4s = new D4S;
	temp_d4s->current = new DefUnit(n, s);
	temp_d4s->next = stack;
	stack = temp_d4s;
}
void DefUnitStack::print() {
	D4S *temp = stack;
	string t;
	while (temp) {
		cout << temp->current->name << ":";
		cout << temp->current->stuff << endl;
		temp = temp->next;
	}
}
int DefUnitStack::search(string s) {
	D4S *temp = stack;
	while (temp) {
		if (temp->current->name == s) {
			return(1);
		}
		temp = temp->next;
	}
	return(0);
}
string DefUnitStack::contents(string s) {
	D4S *temp = stack;
	while (temp) {
		if (temp->current->name == s) {
			return(temp->current->stuff);
		}
		temp = temp->next;
	}
	return(NULL);
}
void DefUnitStack::release()
{
	while (stack)
	{
		D4S* tmp = stack; // grab it
		//cerr << "releasing def_stack item: "<< tmp->current->name<< endl;
		stack = stack->next; // advance p
		delete tmp->current; // release 2 strings
		delete tmp; // release node
	}
	stack = NULL;
}
