#ifndef _GEOMETRY_STRUCTURE_H
#define _GEOMETRY_STRUCTURE_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Point {
	double x, y;
} Point;

struct Vector {

};

Point * newPoint(double x, double y);
Point * newRandomPoint();
Point * newRandomPoints(unsigned int n);
void freePoint(Point *p);
int comparePoints(const void *p1, const void *p2);
void printPoint(Point *p);

#endif
