//////////////////////////////////////////////////////////////////////////
// ECE-660, PROJECT 3
// AUTHOR:  PANKAJ GUPTA
// Date:    May 15th, 2003
//////////////////////////////////////////////////////////////////////////


#include "SDL.h"
#include "camera.h"
#include "Mesh.h"
#include "imageloader.h"
#include "RGBpixmap.h"


#define BUTTON_NOT_HELD	false
#define BUTTON_HELD		true

using namespace std;

static int screenHeight = 800;
static int screenWidth = 1200;
static int mainWindow;
static GLUquadricObj *cylSolidQuadric;
static GLUquadricObj *cylWireQuadric;
static Camera camera;
static int wireframe = 0;
static float scale = 0.75;

int camera_x = 0;
int camera_y = 0;
bool left_btn = false;
bool right_btn = false;

void Tree();
static Mesh rock("models/rock_1.obj");
static Mesh island("models/island_1.obj");
static Mesh palm("models/palmleaf_group.obj");
static Mesh palmtree("models/palmtrunk.obj");
static Mesh sky("models/skybox.obj");
//Mesh palmtree;


static float nearPlane = 1.0;
static float farPlane = 300.0;
static float transitionSpeed = 1.0;
double mx, my, mz = 0;

//---------FPS counter------------
GLvoid *font_style = GLUT_BITMAP_TIMES_ROMAN_24;
float fps = 0;
int frameCount = 0;
int currentTime = 0, previousTime = 0;
void drawFPS();
void printw(float x, float y, float z, char* format, ...);
void calculateFPS();
//----------End FPS counter---------

class Traverse {
public:
	enum Mode {
		MODE_GO = 0,
		MODE_ROTATE,
		MODE_ROLL,
		MODE_SLIDE,
		MODE_TILT
	};

	Traverse() : _mode(MODE_GO), _active(false), _pivotScale(10) {
		_prevRotU = 0.0;
		_prevRotV = 0.0;
		_rotU = 0.0;
		_rotV = 0.0;
	}

	Mode getMode() {
		return _mode;
	}

	void setMode(Mode m) {
		_mode = m;
	}

	void setCameraPosition(int position) {
		_prevRotU = 0.0;
		_prevRotV = 0.0;
		_rotU = 0.0;
		_rotV = 0.0;
		scale = 0.75;

		switch (position) {
		case 1:
			camera.set(Point3(0.0, 50.0, 200.0), Point3(0.0, 50.0, 0.0), Vector3(0.0, 1.0, 0.0));
			break;
		case 2:
			camera.set(Point3(-0.394212, 24.7825, 71.8279), Point3(-0.380191, 24.7825, 70.828), Vector3(0.0, 1.0, 0.0));
			break;
		case 3:
			camera.set(Point3(0.0, 20.0, 29.0), Point3(0.0, 25.0, 0.0), Vector3(0.0, 1.0, 0.0));
			break;
		case 4:
			camera.set(Point3(100.0, 100.0, 150.0), Point3(0.0, 25.0, 0.0), Vector3(0.0, 1.0, 0.0));
			break;
		case 5:
			camera.set(Point3(54.0, 65.0, 70.0), Point3(50.0, 60.0, 50.0), Vector3(0.0, 1.0, 0.0));
			break;
		case 6:
			camera.set(Point3(-50.4335, 57.6836, 53.1501), Point3(-50.2238, 57.6788, 52.1724), Vector3(0.0, 1.0, 0.0));
			break;
		default:
			break;
		}
	}

	void setPivotPosition(int x, int y) {
		_pivotX = x; _pivotY = y;
		_mouseX = x; _mouseY = y;
		activate();
	}

	void setMousePosition(int x, int y) {
		_mouseX = x; _mouseY = y;
	}

	void drawPivot() {
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINES);
		glVertex2i(_pivotX - _pivotScale, _pivotY);
		glVertex2i(_pivotX + _pivotScale, _pivotY);
		glVertex2i(_pivotX, _pivotY - _pivotScale);
		glVertex2i(_pivotX, _pivotY + _pivotScale);
		glEnd();
	}

	void drawLine() {
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINES);
		glVertex2i(_pivotX, _pivotY);
		glVertex2i(_mouseX, _mouseY);
		glEnd();
	}

	void draw() {
		if (_active) {
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(0, screenWidth, 0, screenHeight);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			glColor3f(0.0f, 0.0f, 0.0f);
			drawPivot();
			drawLine();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}
	}

	void activate() {
		_active = true;
		camera.getAxes(_cameraU, _cameraV, _cameraN);
	}

	void deActivate() {
		_active = false;
		_prevRotU = _rotU;
		_prevRotV = _rotV;
	}

	void rotate() {
		if (_mode == MODE_ROTATE && _active) {
			_rotV = _getSpeedX(360.0) + _prevRotV;
			_rotU = _getSpeedY(360.0) + _prevRotU;
			glRotated(_rotV, 0.0, 1.0, 0.0);
			glRotated(-_rotU, 1.0, 0.0, 0.0);
		}
		else {
			glRotated(_prevRotV, 0.0, 1.0, 0.0);
			glRotated(-_prevRotU, 1.0, 0.0, 0.0);
		}
	}

	void setCamera() {
		glMatrixMode(GL_MODELVIEW);
		if (_active) {
			switch (_mode) {
			case MODE_GO:
			{
							camera.slide(0.0, 0.0, -_getSpeedY(transitionSpeed));
							camera.yaw(-_getSpeedX(0.5));
							break;
			}
			case MODE_SLIDE:
			{
							   camera.slide(_getSpeedX(transitionSpeed), _getSpeedY(transitionSpeed), 0.0);
							   break;
			}
			case MODE_TILT:
			{
							  camera.setAxes(_cameraU, _cameraV, _cameraN);
							  camera.yaw(-_getSpeedX(150.0));
							  camera.pitch(_getSpeedY(150.0));
							  break;
			}
			case MODE_ROLL:
			{
							  camera.setAxes(_cameraU, _cameraV, _cameraN);
							  camera.roll(_getSpeedY(150.0));
							  break;
			}
			default:
				break;
			}
		}
	}

private:
	Mode _mode;
	bool _active;
	int _pivotScale;
	int _pivotX, _pivotY;
	int _mouseX, _mouseY;
	Vector3 _cameraU, _cameraV, _cameraN;
	float _rotU, _rotV, _prevRotU, _prevRotV;

	float _getSpeedX(float maxSpeed) {
		return (_mouseX - _pivotX)*maxSpeed / screenWidth;
	}

	float _getSpeedY(float maxSpeed) {
		return (_mouseY - _pivotY)*maxSpeed / screenHeight;
	}
};

Traverse traverse;

void setMeshMaterial(Mesh &mesh, GLfloat matAmbient[], GLfloat matDiffuse[], GLfloat matSpecular[]) {
	mesh.mtrl.setDefault();
	mesh.mtrl.ambient.set(matAmbient[0], matAmbient[1], matAmbient[2]);
	mesh.mtrl.diffuse.set(matDiffuse[0], matDiffuse[1], matDiffuse[2]);
	mesh.mtrl.specular.set(matSpecular[0], matSpecular[1], matSpecular[2]);
	mesh.mtrl.emissive.set(matSpecular[0], matSpecular[1], matSpecular[2]);
	mesh.mtrl.specularExponent = 0.0;
}

double identitys(double t) {
	return t;
}

double negCos(double t) {
	return -cos(t);
}

double negSin(double t) {
	return -sin(t);
}

void init(void) {

	glClearColor(0.2f, 0.2f, 1.0f, 1.0f); // background is blue
	
	float position[] = { 100.0, 100.0, 100.0, 1.0 };
	float intensity[] = { 0.5, 0.5, 0.5 };
	GLfloat lightAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat mat_emission[] = { 0.4, 0.4, 0.4, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, intensity);
	glLightfv(GL_LIGHT0, GL_EMISSION, mat_emission);

	GLfloat matDiffuse[] = { 0.9766, 0.9375, 0.8984, 1.0f };
	GLfloat matSpecular[] = { 0.0, 0.0f, 0.0f, 1.0f };
	GLfloat baseDiffuse[] = { 0.82, 0.82, 0.9, 1.0f };
	GLfloat wallDiffuse[] = { 0.9375, 0.82, 0.703, 1.0f };
	GLfloat helixDiffuse[] = { 1.0, 0.2, 0.2, 1.0f };

	glEnable(GL_BLEND);//enable blend

	glEnable(GL_DEPTH_TEST);//enable depth
	glEnable(GL_DEPTH);
	glEnable(GL_COLOR_MATERIAL);

	traverse.setCameraPosition(1);
	camera.setShape(45, screenWidth / (float)screenHeight, nearPlane, farPlane);
}

void mouse(int button, int state, int x, int y) {
	y = screenHeight - y;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		traverse.setPivotPosition(x, y);
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		traverse.deActivate();
}

void mouseMove(int x, int y) {
	y = screenHeight - y;
	traverse.setMousePosition(x, y);
}

void initMeshRenderMode(Mesh &mesh) {
	if (wireframe)
		mesh.setRenderMode(Mesh::MODE_WIRE);
	else
		mesh.setRenderMode(Mesh::MODE_SOLID);
}



void renderScene() {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glShadeModel(GL_SMOOTH);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	initMeshRenderMode(rock);
	initMeshRenderMode(island);
	initMeshRenderMode(palm);
	initMeshRenderMode(palmtree);
	initMeshRenderMode(sky);
	glPushMatrix();
	glTranslated(mx, my, mz);
	sky.drawOpenGL();
	sky.setScale(20.0);
	glPopMatrix();
	island.drawOpenGL();
	glPushMatrix();
	glTranslated(41, 5, 19);
	rock.drawOpenGL();
	glPopMatrix();
	
	rock.setScale(3.0);
	island.setScale(50.0);
	Tree();
	
	//drawFPS();

}

void Tree()
{
	//drawPalmTree
	glPushMatrix();
	glTranslated(35, 4, 22);
	palmtree.drawOpenGL();
	palmtree.setScale(5.0);
	glPopMatrix();

	glPushMatrix();
	glTranslated(mx, my, mz);
	palm.setScale(2.0);
	palm.drawOpenGL();
	glPopMatrix();

}


void drawFPS()

{
	//  Load the identity matrix so that FPS string being drawn
	//  won't get animates
	glLoadIdentity();
	//  Print the FPS to the window
	printw(-0.9, -0.9, 0, "FPS: %4.2f", fps);

}

void printw(float x, float y, float z, char* format, ...)

{
	va_list args;	//  Variable argument list
	int len;		//	String length
	int i;			//  Iterator
	char * text;	//	Text
	//  Initialize a variable argument list
	va_start(args, format);
	//  Return the number of characters in the string referenced the list of arguments.
	//  _vscprintf doesn't count terminating '\0' (that's why +1)
	len = _vscprintf(format, args) + 1;
	//  Allocate memory for a string of the specified size
	text = (char *)malloc(len * sizeof(char));
	//  Write formatted output using a pointer to the list of arguments
	vsprintf_s(text, len, format, args);
	//  End using variable argument list 
	va_end(args);
	//  Specify the raster position for pixel operations.
	glRasterPos3f(x, y, z);
	glColor3f(.5, .5, .5);
	//  Draw the characters one by one
	for (i = 0; text[i] != '\0'; i++)
		glutBitmapCharacter(font_style, text[i]);
	//  Free the allocated memory for the string
	free(text);
}

void calculateFPS()

{
	//  Increase frame count
	frameCount++;
	//  Get the number of milliseconds since glutInit called 
	//  (or first call to glutGet(GLUT ELAPSED TIME)).
	currentTime = glutGet(GLUT_ELAPSED_TIME);
	//  Calculate time passed
	int timeInterval = currentTime - previousTime;
	if (timeInterval > 1000)
	{
		//  calculate the number of frames per second
		fps = frameCount / (timeInterval / 1000.0f);
		//  Set time
		previousTime = currentTime;
		//  Reset frame count
		frameCount = 0;
	}
}


void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	traverse.draw();
	traverse.setCamera();

	glPushMatrix();

	traverse.rotate();

	if (wireframe) {
		glDisable(GL_LIGHTING);
	}
	else {
		glEnable(GL_LIGHTING);
	}

	glMatrixMode(GL_MODELVIEW);
	//glColor3f(1.0, 1.0, 1.0);
	glScaled(scale, scale, scale);
	renderScene();
	glPopMatrix();

	glutSwapBuffers();
}

void reshape(int w, int h) {
	screenHeight = h;
	screenWidth = w;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	camera.setShape(45, screenWidth / (float)screenHeight, nearPlane, farPlane);
}

void keys(unsigned char theKey, int mouseX, int mouseY) {
	switch (theKey) {
	case 27:
		exit(0);
	case '1':
		traverse.setCameraPosition(1);
		//pillar.writeMesh("outputcube.3vn");
		
		break;
	case '2':
		traverse.setCameraPosition(2);
		break;
	case '3':
		traverse.setCameraPosition(3);
		break;
	case '4':
		traverse.setCameraPosition(4);
		break;
	case '5':
		traverse.setCameraPosition(5);
		break;
	case '6':
		traverse.setCameraPosition(6);
		break;
	case 'a':
		transitionSpeed *= 0.98;
		glutPostRedisplay();
		break;
	case 'A':
		transitionSpeed *= 1.02;
		glutPostRedisplay();
		break;
	case 'g':
		traverse.setMode(Traverse::MODE_GO);
		break;
	case 'p':
		camera.printPosition();
		break;
	case 'r':
		traverse.setMode(Traverse::MODE_ROTATE);
		break;
	case 'R':
		traverse.setMode(Traverse::MODE_ROLL);
		break;
	case 's':
		traverse.setMode(Traverse::MODE_SLIDE);
		break;
	case 't':
		traverse.setMode(Traverse::MODE_TILT);
		break;
	case 'w':
		if (wireframe == 1) {
			cout << "Switching mode to solid" << endl;
			wireframe = 0;
		}
		else {
			cout << "Switching mode to wireframe" << endl;
			wireframe = 1;
		}
		glutPostRedisplay();
		break;
	case 'z':
		scale *= 0.98;
		glutPostRedisplay();
		break;
	case 'Z':
		scale *= 1.02;
		glutPostRedisplay();
		break;
	case '?':
		cout << "The options are:" << endl;
		cout << "\t1: Set camera to front view" << endl;
		cout << "\t2: Set camera at entrance" << endl;
		cout << "\t3: Set camera for viewing the tube" << endl;
		cout << "\t4: Set camera for perspective view" << endl;
		cout << "\t5: Set camera for pillar closeup" << endl;
		cout << "\t6: Set camera inside pillar" << endl;
		cout << "\ta: Decrease speed of transition" << endl;
		cout << "\tA: Increase speed of transition" << endl;
		cout << "\tg: Set mode to 'Go'" << endl;
		cout << "\tr: Set mode to 'Rotate'" << endl;
		cout << "\tR: Set mode to 'Roll'" << endl;
		cout << "\ts: Set mode to 'Slide'" << endl;
		cout << "\tt: Set mode to 'tilt'" << endl;
		cout << "\tp: Print camera position and lookAt point" << endl;
		cout << "\tw: Toggle wireframe display" << endl;
		cout << "\tz: Zoom out" << endl;
		cout << "\tZ: Zoom in" << endl;
		cout << "\t?: Print this help menu" << endl;
	}
}

void handleKB(unsigned char key, int x, int y) {

	switch (key) {

	case 'q':
		exit(0);
		break;
	case 'w':
		camera.slide(0, 0, -4);
		break;
	case 's':
		camera.slide(0, 0, 4);
		break;
	case 'd':
		camera.slide(4, 0, 0);
		break;
	case 'a':
		camera.slide(-4, 0, 0);
		break;
	case 'r':
		if (right_btn) {
			camera.roll(5);
		}
		if (left_btn) {
			camera.roll(-5);
		}
		break;
	case '1':
		if (right_btn)
		{
			mx--;
		}
		else
			mx++;
		cout << "mx - " << mx << " my - " << my << " mz - " << mz << endl;
		break;
	case '2':
		if (right_btn)
		{
			my--;
		}
		else
			my++;
		cout << "mx - " << mx << " my - " << my << " mz - " << mz << endl;
		break;
	case '3':
		if (right_btn)
		{
			mz--;
		}
		else
			mz++;
		cout << "mx - " << mx << " my - " << my << " mz - " << mz << endl;
		break;

	default:;
	}
	glutPostRedisplay();
}

void mouseClickHandler(int button, int state, int x, int y)
{

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
	{
		left_btn = BUTTON_HELD;
	}

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
	{
		left_btn = BUTTON_NOT_HELD;
	}

	if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN))
	{
		right_btn = BUTTON_HELD;
	}

	if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP))
	{
		right_btn = BUTTON_NOT_HELD;
	}
}

void mouseMotionMonitor(int x, int y)
{
	if (x < camera_x)
	{
		camera.yaw(1);
	}
	if (x > camera_x)
	{
		camera.yaw(-1);
	}
	if (y < camera_y)
	{
		camera.pitch(1);
	}
	if (y > camera_y)
	{
		camera.pitch(-1);
	}
	camera_x = x;
	camera_y = y;
	glutPostRedisplay();
}
void idle() {
	if (glutGetWindow() != mainWindow)
		glutSetWindow(mainWindow);
	calculateFPS();
	glutPostRedisplay();
}

void main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(20, 20);
	mainWindow = glutCreateWindow("Island World");
	glutDisplayFunc(display);
	glutKeyboardFunc(handleKB);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouseClickHandler);
	glutMotionFunc(mouseMotionMonitor);
	glutIdleFunc(idle);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);

	init();
	glutMainLoop();
}
