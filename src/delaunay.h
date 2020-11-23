#ifndef _DELAUNAY_H
#define _DELAUNAY_H

#include "BOV.h"
#include "math.h"

#define MIN_DIST 1E-10 // Minimim distance between two points (used to avoid placing multiple points at the same location)


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
void resetDelaunayTriangulation(DelaunayTriangulation *delTri);
GLsizei getPointIndex(DelaunayTriangulation *delTri, GLfloat point[2]);
GLfloat getDistanceToClosestPoint(DelaunayTriangulation *delTri, GLfloat point[2]);
int addPoint(DelaunayTriangulation *delTri, GLfloat point[2]);
int deletePointAtIndex(DelaunayTriangulation *delTri, GLsizei i_p);
int deletePoint(DelaunayTriangulation *delTri, GLfloat point[2]);
void freeDelaunayTriangulation(DelaunayTriangulation *delTri);
void describeDelaunayTriangulation(DelaunayTriangulation *delTri);
GLsizei getDelaunayTriangulationNumberOfLines(DelaunayTriangulation *delTri);
void getDelaunayTriangulationLines(DelaunayTriangulation *delTri, GLfloat lines[][2], GLsizei n_lines);
GLsizei getNumberOfTriangles(DelaunayTriangulation *delTri);
void getVoronoiCentersAndNeighbors(DelaunayTriangulation *delTri, GLfloat centers[][2], GLsizei neighbors[][3], GLsizei n_triangles);
void getVoronoiLines(DelaunayTriangulation *delTri, GLfloat centers[][2], GLsizei neighbors[][3], GLfloat lines[][2], GLsizei n_triangles);

Edge* addEdge(DelaunayTriangulation *delTri, GLsizei orig, GLsizei dest);
void describeEdge(Edge *e);
void spliceEdges(DelaunayTriangulation *delTri, Edge *a, Edge *b);
Edge* connectEdges(DelaunayTriangulation *delTri, Edge *a, Edge *b);
void deleteEdge(DelaunayTriangulation *delTri, Edge *e);

static int compare_points(const void *a_v, const void *b_v);
GLfloat _det_3x3_(GLfloat m[3][3]);
int pointInCircle(DelaunayTriangulation *delTri, GLsizei i_p, GLsizei i_a, GLsizei i_b, GLsizei i_c);
void circleCenter(DelaunayTriangulation *delTri, GLsizei i_a, GLsizei i_b, GLsizei i_c, GLfloat center[2]);
int pointRightOfEdge(GLfloat point[2], GLfloat orig[2], GLfloat dest[2]);
int pointLeftOfEdge(GLfloat point[2], GLfloat orig[2], GLfloat dest[2]);
int pointCompareEdge(DelaunayTriangulation *delTri, GLsizei i_p, Edge *e);

void triangulateDT(DelaunayTriangulation *delTri);
void triangulate(DelaunayTriangulation *delTri, GLsizei start, GLsizei end, Edge **el, Edge **er);

void getMousePosition(bov_window_t *window, GLfloat mouse_pos[2]);
void drawDelaunayTriangulation(DelaunayTriangulation *delTri, bov_window_t *window);

#endif
