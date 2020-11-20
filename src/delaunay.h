#ifndef _DELAUNAY_H
#define _DELAUNAY_H

#include "BOV.h"
#include "math.h"


typedef struct Edge {
    GLsizei orig, dest;     // indices of origin and destinations points
    GLsizei onext, oprev;   // indices of next and previous edge
    GLsizei sym;            // index of opposite edge

    GLsizei idx;            // index of the edge

    int discarded;          // 1 if the edge has to be discarded
} Edges;

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

    // Edges
    GLsizei n_edges;
    Edges *edges;

} DelaunayTriangulation;

DelaunayTriangulation* initDelaunayTriangulation(GLfloat points[][2], GLsizei n);
void freeDelaunayTriangulation(DelaunayTriangulation *delTri);

GLsizei addEdge(DelaunayTriangulation *delTri, GLsizei orig, GLsizei dest, Edge *e);
void spliceEdges(DelaunayTriangulation *delTri, GLsizei i_a, GLsizei i_b);
GLsizei connectEdges(DelaunayTriangulation *delTri, GLsizei i_a, GLsizei i_b, Edge *e);
void deleteEdge(DelaunayTriangulation *delTri, GLsizei i_e);

static int compare_points(const void *a_v, const void *b_v);
int pointInCircleFast(GLfloat point[2], GLfloat center[2], GLfloat radius);
GLfloat _det_3x3_(GLfloat m[3][3]);
int pointInCircleRobust(GLfloat point[2], GLfloat a[2], GLfloat b[2], GLfloat c[2]);
int pointRightOfEdge(GLfloat point[2], GLfloat orig[2], GLfloat dest[2]);
int pointLeftOfEdge(GLfloat point[2], GLfloat orig[2], GLfloat dest[2]);

void triangulateDT(DelaunayTriangulation *delTri);
void triangulate(DelaunayTriangulation *delTri, GLsizei start, GLsizei end, Edge *el, Edge *er);


void drawDelaunayTriangulation(DelaunayTriangulation *delTri, bov_window_t *window);

#endif
