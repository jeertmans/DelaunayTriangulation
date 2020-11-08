#ifndef _DELAUNAY_H
#define _DELAUNAY_H

#include "BOV.h"
#include "math.h"

typedef struct DelaunayTriangulation {
    // Allocating
    GLsizei n_centers;
    GLfloat (*centers)[2];

    // Success will be == 1 if triang. has been completed
    int success;

    // Keeping track of points
    GLsizei n_points;
    GLfloat (*points)[2];

    // Triangles index
    GLsizei n_triangles;
    GLsizei (*triangles)[3];
} DelaunayTriangulation;

DelaunayTriangulation* initDelaunayTriangulation(GLfloat points[][2], GLsizei n);
void freeDelaunayTriangulation(DelaunayTriangulation *delTri);

void drawDelaunayTriangulation(DelaunayTriangulation *delTri, bov_window_t *window);

#endif
