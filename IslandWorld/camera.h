#ifndef _HlCamera
#define _HlCamera
#include "SDL.h" // needed for Vector3 definitions

//##################### class CAMERA ######################
class Camera{
private:
	Point3 eye;
	Vector3 u, v, n;
	float aspect, nearDist, farDist, viewAngle;
	void setModelViewMatrix();

public:
	Camera(void);
	void roll(float angle);
	void pitch(float angle);
	void yaw(float angle);
	void slide(double du, double dv, double dn);
	void set(Point3 Eye, Point3 look, Vector3 up);
	void setShape(float viewAngle, float aspect, float Near, float Far);
	void setAspect(float asp);
	void getShape(float& viewAngle, float& aspect, float& Near, float& Far);
	void rotAxes(Vector3& a, Vector3& b, float angle);
	void setDefaultCamera();
	void getAxes(Vector3 &u, Vector3 &v, Vector3 &n);
	void setAxes(Vector3 &u, Vector3 &v, Vector3 &n);
	void printPosition();
};
#endif

