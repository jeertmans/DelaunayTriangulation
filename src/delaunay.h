/*
 * Delaunay triangulation program, made to be fast and responsive.
 *
 * Author: 	Jerome Eertmans
 * Context: project for the LMECA2710 course, at EPL faculty UCLouvain,
 *			given by Pr. Vincent Legat and Pr. Jean-Francois Remacle.
 *
 * All sources of inspiration are clearly marked at specific places in the code.
 * Feel free to use and re-use this code! No attribution but the ones for BOV.h,
 * predicates.h and the sound files are required.
 *
 * Any contribution is welcome on the Github this project:
 * https://github.com/jeertmans/DelaunayTriangulation
 */

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

#define MIN(x,y) ((x<y)?x:y)
#define MAX(x,y) ((x>y)?x:y)
#define RAD2DEG(a) ((180*a)/M_PI)

#define ROBUST 1		// If True, will use robust predicates but will add overhead (x1.3 slowdown)
						// Non robust methods may caude problems with colinear (or close to) points
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

typedef struct DTDrawingParameters {
	DelaunayTriangulation *delTri;
	bov_window_t *window;
	GLfloat (*linesPoints)[2];
	bov_points_t *pointsDraw, *activePointsDraw, *linesDraw;
	GLfloat y_min, y_max;
	int FAST, sleep;

	GLsizei n_divides, n_divides_max;

	GLfloat (*divideLinesPoints)[2];
	char *divideLinesMask;
	bov_points_t *divideLinesDraw;
} DTDrawingParameters;

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
void getPointsBoudingBox(GLfloat points[][2], GLsizei n_points, GLfloat bounds[][2]);
int pointInCircle(DelaunayTriangulation *delTri, GLsizei i_p, GLsizei i_a, GLsizei i_b, GLsizei i_c);
void circleCenter(DelaunayTriangulation *delTri, GLsizei i_a, GLsizei i_b, GLsizei i_c, GLfloat center[2]);
int pointCompareEdge(DelaunayTriangulation *delTri, GLsizei i_p, Edge *e);
GLfloat angleBetweenContiguousEdges(DelaunayTriangulation *delTri, Edge *e, Edge *f);

void triangulateDT(DelaunayTriangulation *delTri);
void triangulate(DelaunayTriangulation *delTri, GLsizei start, GLsizei end, Edge **el, Edge **er);

void getMousePosition(bov_window_t *window, GLfloat mouse_pos[2]);
void getInfoText(DelaunayTriangulation *delTri, char *info_text_char);
void drawDelaunayTriangulation(DelaunayTriangulation *delTri, bov_window_t *window, double total_time);
DTDrawingParameters* initDTDrawingParameters(DelaunayTriangulation *delTri,
											 bov_window_t *window,
						 					 GLfloat linesPoints[][2],
						 					 bov_points_t *pointsDraw,
											 bov_points_t *activePointsDraw,
											 bov_points_t *linesDraw,
											 GLfloat bounds[][2],
						 					 int FAST, int sleep);
GLsizei addDivideLine(DTDrawingParameters *DTDparams, GLsizei pivot);
void deleteDivideLine(DTDrawingParameters *DTDparams, GLsizei index);
void freeDTDrawingParameters(DTDrawingParameters *DTDparams);
void reDrawTriangulation(DTDrawingParameters *DTDparams,
					 	 int start, int end);
void triangulateDTIllustrated(DTDrawingParameters *DTDparams);
void triangulateIllustrated(DelaunayTriangulation *delTri, GLsizei start, GLsizei end, Edge **el, Edge **er,
						    DTDrawingParameters *DTDparams);

#endif
