#include "geometry_structure.h"

Point * newPoint(double x, double y) {
    Point *p = (Point *) malloc(sizeof(Point *));
    p->x = x;
    p->y = y;
    return p;
}

Point * newRandomPoint() {
    return newPoint(
        (double) rand() / RAND_MAX,
        (double) rand() / RAND_MAX
    );
}

Point * newRandomPoints(unsigned int n) {
    Point *points = (Point *) malloc(n * sizeof(Point));
    for(unsigned int i = 0; i < n; i++) {
        points[i].x = (double) rand() / RAND_MAX;
        points[i].y = (double) rand() / RAND_MAX;
    }
    return points;
}

void freePoint(Point *p) {
    free(p);
}

int compareDouble(double a, double b) {
    return a < b ? 1 : a > b ? -1 : 0;
}

int comparePoints(const void *p1, const void *p2) {
    Point *point_1 = (Point *) p1;
    Point *point_2 = (Point *) p2;

    int comp_y = compareDouble(point_1-> y, point_2->y);

    if (comp_y == 0) {
        return compareDouble(point_2->x, point_1->x);
    }
    else {
        return comp_y;
    }
}

void printPoint(Point *p) {
    printf("Point(%f, %f)\n", p->x, p->y);
}
