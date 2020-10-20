#ifndef _GEOMETRY_STRUCTURE_H
#define _GEOMETRY_STRUCTURE_H

typedef struct Point {
	double x, y;
} Point;

struct Vector {

};


Point * newPoint(double x, double y);
void freePoint(Point *p);
bool comparePoints(Point *p1, Point *p2);

#endif
