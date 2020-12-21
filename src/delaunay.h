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
#define N_POINTS 100	// Number of points to draw in a circle

// Define some colors

#define RED_COLOR (GLfloat[4]) {1.0, 0.0, 0.0, 1.0}
#define GREEN_COLOR (GLfloat[4]) {0.0, 1.0, 0.0, 1.0}
#define BLUE_COLOR (GLfloat[4]) {0.0, 0.0, 1.0, 1.0}
#define BLACK_COLOR (GLfloat[4]) {0.0, 0.0, 0.0, 1.0}
#define WHITE_COLOR (GLfloat[4]) {1.0, 1.0, 1.0, 1.0}
#define NONE_COLOR (GLfloat[4]) {0.0, 0.0, 0.0, 0.0}
#define DARK_RED_COLOR (GLfloat[4]) {0.722, 0.163, 0.0, 1.0}
#define MED_BLUE_COLOR (GLfloat[4]) {0.0, 0.565, 0.901, 1.0}
#define MED_BLUE_COLOR_25 (GLfloat[4]) {0.0, 0.565, 0.901, 0.25}
#define SEA_GREEN_COLOR (GLfloat[4]) {0.016, 0.73, 0.677, 1.0}
#define SEA_GREEN_COLOR_75 (GLfloat[4]) {0.016, 0.73, 0.677, 0.75}

#define BASIC_POINTS_COLOR BLACK_COLOR
#define BASIC_POINTS_OUTLINE_COLOR (GLfloat[4]) {0.3, 0.12, 0.0, 0.25}
#define VORONOI_POINTS_COLOR DARK_RED_COLOR
#define VORONOI_POINTS_OUTLINE_COLOR (GLfloat[4]) {0.3, 0.12, 0.0, 0.25}
#define ACTIVE_POINTS_COLOR DARK_RED_COLOR
#define ACTIVE_POINTS_OUTLINE_COLOR (GLfloat[4]) {0.3, 1., 0.0, 0.25}
#define SEARCHED_POINTS_COLOR MED_BLUE_COLOR
#define MOUSE_POINTS_COLOR NONE_COLOR
#define MOUSE_POINTS_OUTLINE_COLOR (GLfloat[4]) {0.3, 0.12, 0.0, 0.25}

#define TRIANGULATION_LINES_COLOR BLACK_COLOR
#define TRIANGULATION_LINES_OUTLINE_COLOR (GLfloat[4]) {0.3, 0.12, 0.0, 0.25}
#define VORONOI_LINES_COLOR MED_BLUE_COLOR
#define VORONOI_LINES_OUTLINE_COLOR MED_BLUE_COLOR_25
#define DIVIDE_LINES_COLOR (GLfloat[4]) {1.0, 0.0, 0.0, 0.8}
#define CIRCLE_LINES_COLOR MED_BLUE_COLOR

// Define some widths

#define BASIC_POINTS_WIDTH 0.01
#define VORONOI_POINTS_WIDTH BASIC_POINTS_WIDTH
#define ACTIVE_POINTS_WIDTH (2.0 * BASIC_POINTS_WIDTH)
#define SEARCHED_POINTS_WIDTH (1.5 * BASIC_POINTS_WIDTH)
#define MOUSE_POINTS_WIDTH 0.04

#define TRIANGULATION_LINES_WIDTH 0.004
#define VORONOI_LINES_WIDTH TRIANGULATION_LINES_WIDTH
#define DIVIDE_LINES_WIDTH 0.002
#define CIRCLE_LINES_WIDTH 0.004


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
	GLfloat cos_[N_POINTS];
	GLfloat sin_[N_POINTS];
	GLfloat circ[N_POINTS][2];
	GLsizei searchPoints[3];
	bov_points_t *pointsDraw, *activePointsDraw, *linesDraw;
	bov_points_t *searchPointsDraw, *circlePointsDraw;
	bov_order_t *searchPointsOrder;
	GLfloat y_min, y_max;
	int FAST, sleep, draw_circle;

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
GLfloat circleCenter(DelaunayTriangulation *delTri, GLsizei i_a, GLsizei i_b, GLsizei i_c, GLfloat center[2]);
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
void setSearchPoints(DTDrawingParameters *DTDparams,
					 GLsizei i_a, GLsizei i_b ,GLsizei i_c);
GLsizei addDivideLine(DTDrawingParameters *DTDparams, GLsizei pivot);
void deleteDivideLine(DTDrawingParameters *DTDparams, GLsizei index);
void freeDTDrawingParameters(DTDrawingParameters *DTDparams);
void reDrawTriangulation(DTDrawingParameters *DTDparams,
					 	 int start, int end);
void triangulateDTIllustrated(DTDrawingParameters *DTDparams);
void triangulateIllustrated(DelaunayTriangulation *delTri, GLsizei start, GLsizei end, Edge **el, Edge **er,
						    DTDrawingParameters *DTDparams);

#endif
