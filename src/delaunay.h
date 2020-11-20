#ifndef _DELAUNAY_H
#define _DELAUNAY_H

#include "BOV.h"
#include "math.h"


typedef struct Edge {
    GLsizei orig, dest;     // indices of origin and destinations points
    struct Edge *onext, *oprev;    // next and previous edges
    struct Edge *sym;              // opposite edge

    GLsizei idx;            // index of the edge

    int discarded;          // 1 if the edge has to be discarded
} Edge;

typedef struct DelaunayTriangulation {
    // Success will be == 1 if triang. has been completed
    int success;

    // Keeping track of points
    GLsizei n_points;
    GLfloat (*points)[2];

    // Edges
    GLsizei n_edges, n_edges_discarded, n_edges_max;
    Edge *edges;

} DelaunayTriangulation;

DelaunayTriangulation* initDelaunayTriangulation(GLfloat points[][2], GLsizei n);
void freeDelaunayTriangulation(DelaunayTriangulation *delTri);
void describeDelaunayTriangulation(DelaunayTriangulation *delTri);
GLsizei getDelaunayTriangulationNumberOfLines(DelaunayTriangulation *delTri);
void getDelaunayTriangulationLines(DelaunayTriangulation *delTri, GLfloat lines[][2], GLsizei n_lines);

Edge* addEdge(DelaunayTriangulation *delTri, GLsizei orig, GLsizei dest);
void describeEdge(Edge *e);
void spliceEdges(DelaunayTriangulation *delTri, Edge *a, Edge *b);
Edge* connectEdges(DelaunayTriangulation *delTri, Edge *a, Edge *b);
void deleteEdge(DelaunayTriangulation *delTri, Edge *e);

static int compare_points(const void *a_v, const void *b_v);
GLfloat _det_3x3_(GLfloat m[3][3]);
int pointInCircle(DelaunayTriangulation *delTri, GLsizei i_p, GLsizei i_a, GLsizei i_b, GLsizei i_c);
int pointRightOfEdge(GLfloat point[2], GLfloat orig[2], GLfloat dest[2]);
int pointLeftOfEdge(GLfloat point[2], GLfloat orig[2], GLfloat dest[2]);
int pointCompareEdge(DelaunayTriangulation *delTri, GLsizei i_p, Edge *e);

void triangulateDT(DelaunayTriangulation *delTri);
void triangulate(DelaunayTriangulation *delTri, GLsizei start, GLsizei end, Edge **el, Edge **er);


void drawDelaunayTriangulation(DelaunayTriangulation *delTri, bov_window_t *window);

#endif
