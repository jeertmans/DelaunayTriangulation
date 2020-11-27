#ifndef _DELAUNAY_H
#define _DELAUNAY_H

#include "BOV.h"
#include "math.h"
#include "predicates.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#define ROBUST 1		// If True, will use robust predicates
#define MIN_DIST 1E-10 	// Minimim distance between two points (used to avoid placing multiple points at the same location)


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

DelaunayTriangulation* initDelaunayTriangulation(GLfloat points[][2], GLsizei n, int remove_duplicates);
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
void fsaveDelaunayTriangulation(DelaunayTriangulation *delTri, FILE* file_out) ;

Edge* addEdge(DelaunayTriangulation *delTri, GLsizei orig, GLsizei dest);
void describeEdge(Edge *e);
void spliceEdges(DelaunayTriangulation *delTri, Edge *a, Edge *b);
Edge* connectEdges(DelaunayTriangulation *delTri, Edge *a, Edge *b);
void deleteEdge(DelaunayTriangulation *delTri, Edge *e);

static int compare_points(const void *a_v, const void *b_v);
int pointInCircle(DelaunayTriangulation *delTri, GLsizei i_p, GLsizei i_a, GLsizei i_b, GLsizei i_c);
void circleCenter(DelaunayTriangulation *delTri, GLsizei i_a, GLsizei i_b, GLsizei i_c, GLfloat center[2]);
int pointCompareEdge(DelaunayTriangulation *delTri, GLsizei i_p, Edge *e);

void triangulateDT(DelaunayTriangulation *delTri);
void triangulate(DelaunayTriangulation *delTri, GLsizei start, GLsizei end, Edge **el, Edge **er);

void getMousePosition(bov_window_t *window, GLfloat mouse_pos[2]);
void drawDelaunayTriangulation(DelaunayTriangulation *delTri, bov_window_t *window, double total_time);
void reDrawTriangulation(DelaunayTriangulation *delTri, bov_window_t *window,
						 GLfloat linesPoints[][2],
						 bov_points_t *pointsDraw, bov_points_t *linesDraw,
						 int FAST, int sleep);
void triangulateDTIllustrated(DelaunayTriangulation *delTri, bov_window_t *window,
						 GLfloat linesPoints[][2],
						 bov_points_t *pointsDraw, bov_points_t *linesDraw,
						 int FAST, int sleep);
void triangulateIllustrated(DelaunayTriangulation *delTri, GLsizei start, GLsizei end, Edge **el, Edge **er, bov_window_t *window,
						    GLfloat linesPoints[][2],
						    bov_points_t *pointsDraw, bov_points_t *linesDraw,
						    int FAST, int sleep);

#endif
