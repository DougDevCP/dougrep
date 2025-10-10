///////////////////////////////////////////////////////////////////////////////////
//  File: fmdib_math.h
//
//  Source file with the FilterMeisterVS GDI drawing functions
//
//  Copyright (c) 1997-2017 by Alex Hunter, Harald Heim & Martijn Van der Lee
// 
//  FilterMeisterVS is free software. You can redistribute and modify it
//  under the terms of the MPL2 license. See https://www.mozilla.org/en-US/MPL/2.0/
//  Any changes to the FilterMeisterVS files in the /components/filtermeister/
//  sub folder have to be published or sent to harry@thepluginsite.com. 
//  The individual filter code itself does not need to be disclosed.
//  
///////////////////////////////////////////////////////////////////////////////////

#ifndef FMMATH_H
#define FMMATH_H

#ifdef BCB
#define max(a,b)	(a) > (b)? (a) : (b)
#define min(a,b)	(a) < (b)? (a) : (b)
#include <math.h>	// sqrt
#endif

#ifndef M_PI
	#define M_PI       3.14159265358979323846
#endif

//---------------------------------------------------------------------------

double fr2x (double d, double m)
{
	return sin(d) * m;
}

double fr2y (double d, double m)
{
	return cos(d) * m;
}

double fc2m (double x, double y)
{
	return _hypot(x, y);
}

double fc2d (double x, double y)
{
	double m = _hypot(x, y);
	if (m > 0.) {
		double d = acos(y / m);
		return (x < 0.? (M_PI * 2.) - d : d);
	}
	return 0.;
}

//---------------------------------------------------------------------------
			  /*
struct Vector {
	double x, y, z;
};

__inline double vectorDotProduct (struct Vector a, struct Vector b) {
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}

__inline double vectorLength (struct Vector a) {
	return sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}

__inline double vectorAngleDiff (struct Vector a, struct Vector b) {
	double dp = vectorDotProduct(a, b);
	double la = vectorLength(a);
	double lb = vectorLength(b);
	double d = dp / (la * lb);

	if (d >= 1.) {
		return 0.;
	} else if (d <= -1.) {
		return M_PI;
	}

	return acos(d);
}

__inline struct Vector vectorRotateY(struct Vector v, double r) {
	double m = fc2m(v.x, v.z);
	double d = fc2d(v.x, v.z);
	d += r;
	v.x = fr2x(d, m);
	v.z = fr2y(d, m);
	return v;
}

__inline struct Vector vectorRotateX(struct Vector v, double r) {
	double m = fc2m(v.y, v.z);
	double d = fc2d(v.y, v.z);
	d += r;
	v.y = fr2x(d, m);
	v.z = fr2y(d, m);
	return v;
}
             */
#endif
// FMMATH_H
