#include "geometry_structure.h"

Point * newPoint(double x, double y) {
    Point *p = (Point *) malloc(sizeof(Point *));
    p->x = x;
    p->y = y;
    return p;
};

void freePoint(Point *p) {
    free(p);
};

bool comparePoints(Point *p1, Point *p2) {
    if (p1->y < p2->y) {
        return true;
    }
    else if (p1->y > p2->y) {
        return false;
    }
    else {
        return p1->x > p2->x;
    }
}
