#ifndef _DELAUNAY_H
#define _DELAUNAY_H

#include "BOV.h"

typedef struct DelaunayTriangulation {
    // Allocating
    GLsizei n_centers;
    GLfloat (*centers)[2];

    // Success will be == 1 if triang. has been completed
    int success;

    // Keeping track of points
    GLsizei n_points;
    GLfloat (*points)[2];
} DelaunayTriangulation;

DelaunayTriangulation* initDelaunayTriangulation(GLfloat points[][2], GLsizei n);
void freeDelaunayTriangulation(DelaunayTriangulation *delTri);

#endif
