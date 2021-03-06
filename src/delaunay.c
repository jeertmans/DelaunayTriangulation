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

#include "delaunay.h"

// Inspired from: https://github.com/alexbaryzhikov/triangulation (Python code)

//////////////////////////////////////////////////
// Begin: DelaunayTriangulation structure utils //
//////////////////////////////////////////////////

/*
 * Allocates and returns a DelaunayTriangulation structure from a set of n points.
 *
 * points: 					the n x 2 array of distinct points (x, y)
 * n:						the number of points
 * re;remove_duplicates:	if set, will remove the duplicated poits
 *
 * returns:					a new DelaunayTriangulation structure
 */
DelaunayTriangulation* initDelaunayTriangulation(GLfloat points[][2], GLsizei n, int remove_duplicates) {

#if ROBUST
	exactinit();
#endif

    DelaunayTriangulation *delTri = malloc(sizeof(DelaunayTriangulation));

    // Points
    delTri->n_points = n;
    delTri->points = malloc(sizeof(delTri->points[0]) * n);

	for (GLsizei i = 0; i < n; i++) {
		delTri->points[i][0] = points[i][0];
		delTri->points[i][1] = points[i][1];
	}

	if ((remove_duplicates) && (delTri->n_points > 1)) {
		qsort(delTri->points, delTri->n_points, 2 * sizeof(GLfloat), compare_points);

		GLsizei c = 1;
		for (GLsizei i = 1; i < delTri->n_points; i++) {
			while ((delTri->points[i][0] == delTri->points[c - 1][0]) && (delTri->points[i][1] == delTri->points[c - 1][1])) {
				if (i < delTri->n_points - 1) {
					i++;
				}
				else {
					c--;
					break;
				}
			}
			delTri->points[c][0] = delTri->points[i][0];
			delTri->points[c][1] = delTri->points[i][1];
			c++;
		}

		GLfloat (*p)[2] = malloc(sizeof(delTri->points[0]) * c);
		if (p == NULL) printf("ERROR: Couldn't reallocate memory for points after removing duplicates\n");

		for (GLsizei i = 0; i < c; i++) {
			p[i][0] = delTri->points[i][0];
			p[i][1] = delTri->points[i][1];
		}

		free(delTri->points);
		delTri->points = p;
		delTri->n_points = c;
	}

	delTri->edges = NULL;
	resetDelaunayTriangulation(delTri);

	return delTri;
}

/*
 * Resets a DelaunayTriangulation so that a new triangulation can safely be done.
 * Must be run whenever you modify the points.
 *
 * delTri:		the DelaunayTriangulation structure
 */
void resetDelaunayTriangulation(DelaunayTriangulation *delTri) {
	if (delTri->edges != NULL) {
		free(delTri->edges);
	}

	// (Half) Edges
	if (delTri->n_points == 0) {
		delTri->n_edges_max = 0;
	}
	else {
		delTri->n_edges_max = (GLsizei) ceil((log(delTri->n_points) * 3 * delTri->n_points));
	}

	delTri->n_edges = 0;
	delTri->n_edges_discarded = 0;
	delTri->edges = malloc(sizeof(Edge) * delTri->n_edges_max);

	if ((delTri->edges == NULL) && (delTri->n_points > 1)) {
		printf("ERROR: Couldn't allocate memory for edges\n");
		free(delTri);
		exit(1);
	}

	delTri->success = 0;
}

/*
 * Returns the index of the point in the DelaunayTriangulation which is the
 * closest to a given point.
 *
 * delTri:		the DelaunayTriangulation structure
 * point:		the point to find
 *
 * returns:		the index of the closest point, -1 if cannot find any point
 */
GLsizei getPointIndex(DelaunayTriangulation *delTri, GLfloat point[2]) {
	if (delTri->n_points == 0) {
		return -1;
	}

	GLsizei i;
	GLsizei closest_idx = 0;
	GLfloat dx, dy, dist;
	dx = point[0] - delTri->points[0][0];
	dy = point[1] - delTri->points[0][1];
	GLfloat closest_dist = dx * dx + dy * dy;

	for (i = 1; i < delTri->n_points; i++) {
		dx = point[0] - delTri->points[i][0];
		dy = point[1] - delTri->points[i][1];
		dist = (dx*dx) + dy*dy;
		if (dist < closest_dist) {
			closest_idx = i;
			closest_dist = dist;
		}
	}
	return closest_idx;
}

/*
 * Returns the distance to the point in the DelaunayTriangulation which is the
 * closest to a given point.
 *
 * delTri:		the DelaunayTriangulation structure
 * point:		the point to compare
 *
 * returns:		the distance to the closest point
 */
GLfloat getDistanceToClosestPoint(DelaunayTriangulation *delTri, GLfloat point[2]) {
	GLsizei idx = getPointIndex(delTri, point);
	if (idx != -1) {
		GLfloat dx, dy;
		dx = point[0] - delTri->points[idx][0];
		dy = point[1] - delTri->points[idx][1];
		return dx * dx + dy * dy;
	}
	else {
		return INFINITY;
	}
}

/*
 * Updates the value of a point in the DelaunayTriangulation.
 *
 * delTri:		the DelaunayTriangulation structure
 * i_p:			the index of the point to modify
 * point:		the new point
 */
void updatePointAtIndex(DelaunayTriangulation *delTri, GLsizei i_p, GLfloat point[2]) {
	delTri->points[i_p][0] = point[0];
	delTri->points[i_p][1] = point[1];
	resetDelaunayTriangulation(delTri);
}

/*
 * Adds a point in the DelaunayTriangulation.
 *
 * delTri:		the DelaunayTriangulation structure
 * point:		the point to add
 *
 * returns:		1 if point was correctly added, -1 otherwise (means the point is to close to an already existing point)
 */
int addPoint(DelaunayTriangulation *delTri, GLfloat point[2]) {
	if (getDistanceToClosestPoint(delTri, point) <= MIN_DIST) {
		return 0;
	}

	GLsizei idx = delTri->n_points;
	delTri->n_points += 1;

	GLfloat (*points)[2];
	if (delTri->points == NULL) {
		delTri->points = malloc(sizeof(delTri->points[0]) * delTri->n_points);
	}
	points = realloc(delTri->points, sizeof(delTri->points[0]) * delTri->n_points);
	if (points == 0) {
		printf("ERROR: Couldn't allocate more memory for points\n");
		exit(1);
	}
	delTri->points = points;
	delTri->points[idx][0] = point[0];
	delTri->points[idx][1] = point[1];

	resetDelaunayTriangulation(delTri);
	return 1;
}

/*
 * Removes a point from the DelaunayTriangulation.
 *
 * delTri:		the DelaunayTriangulation structure
 * i_p:			the index of the point to delete
 *
 * returns:		1 if point was correctly added, -1 otherwise (if no point to remove)
 */
int deletePointAtIndex(DelaunayTriangulation *delTri, GLsizei i_p) {
	if (delTri->n_points == 0) {
		return 0;
	}

	delTri->n_points -= 1;
	GLfloat (*points)[2] = malloc(sizeof(delTri->points[0]) * delTri->n_points);

	for (GLsizei i = 0; i < i_p; i++) {
		points[i][0] = delTri->points[i][0];
		points[i][1] = delTri->points[i][1];
	}
	for (GLsizei i = i_p; i < delTri->n_points; i++) {
		points[i][0] = delTri->points[i + 1][0];
		points[i][1] = delTri->points[i + 1][1];
	}

	free(delTri->points);
	delTri->points = points;

	resetDelaunayTriangulation(delTri);
	return 1;
}

/*
 * Removes a point from the DelaunayTriangulation.
 *
 * delTri:		the DelaunayTriangulation structure
 * point:		the point to delete
 *
 * returns:		1 if point was correctly added, -1 otherwise (if no point to remove)
 */
int deletePoint(DelaunayTriangulation *delTri, GLfloat point[2]) {
	GLsizei idx = getPointIndex(delTri, point);
	if (idx != -1) {
		return deletePointAtIndex(delTri, idx);
	}
	else {
		return 0;
	}
}

/*
 * Frees a DelaunayTriangulation structure with all its content that it has
 * allocated.
 *
 * delTri:		the DelaunayTriangulation structure
 */
void freeDelaunayTriangulation(DelaunayTriangulation *delTri) {
	if (delTri != NULL) {
		if (delTri->points != NULL) free(delTri->points);
		if (delTri->edges != NULL) free(delTri->edges);
		free(delTri);
	}
}

/*
 * Describes a DelaunayTriangulation structure by printing some information.
 *
 * delTri:		the DelaunayTriangulation structure
 */
void describeDelaunayTriangulation(DelaunayTriangulation *delTri) {

	printf("DelaunayTriangulation structure stored at %p.\n"
		   "\t- Number of points:           %d\n"
		   "\t- Address of points:          %p\n"
		   "\t- Number of edges:            %d\n"
		   "\t- Number of discarded edges:  %d\n"
		   "\t- Maximum number of edges:    %d\n"
		   "\t- Address of edges:           %p\n"
		   "\t- Triangulation computed?:    %d\n",
		   delTri,
		   delTri->n_points, delTri->points,
	   	   delTri->n_edges, delTri->n_edges_discarded, delTri->n_edges_max, delTri->edges,
	   	   delTri->success);
	if (delTri->success) printf("\t- Number of triangles:        %d\n", getNumberOfTriangles(delTri));
}

/*
 * Returns the total number of lines (edges) in the DelaunayTriangulation,
 * excluding the discarded edges.
 *
 * delTri:		the DelaunayTriangulation structure
 *
 * returns:		the number of lines
 */
GLsizei getDelaunayTriangulationNumberOfLines(DelaunayTriangulation *delTri) {
	return (delTri->n_edges - delTri->n_edges_discarded) / 2;
}

/*
 * Populates an array of 2 * n_lines points with the points of all the edges
 * in the DelaunayTriangulation.
 *
 * delTri:		the DelaunayTriangulation structure
 * lines:		the preallocated array that will contain the points
 * n_lines:		the number of lines
 */
void getDelaunayTriangulationLines(DelaunayTriangulation *delTri,
								   GLfloat lines[][2],
							   	   GLsizei n_lines) {

	GLsizei e_i, l_i;
	Edge *e;
	l_i = 0;
	for (e_i = 0; e_i < delTri->n_edges; e_i+=2) {
		e = &(delTri->edges[e_i]);
		if (e->discarded == 0) {
			lines[l_i    ][0] = delTri->points[e->orig][0];
			lines[l_i    ][1] = delTri->points[e->orig][1];
			lines[l_i + 1][0] = delTri->points[e->dest][0];
			lines[l_i + 1][1] = delTri->points[e->dest][1];
			l_i+=2;
		}
	}
}

/*
 * Returns the number of triangles in the DelaunayTriangulation.
 *
 * delTri:		the DelaunayTriangulation structure
 *
 * returns:		the number of of triangles
 */
GLsizei getNumberOfTriangles(DelaunayTriangulation *delTri) {
	if ((delTri->n_points < 3) || (!delTri->success)) {
		return 0;
	}
	char *visited_edges = calloc(delTri->n_edges, sizeof(char));
	GLsizei n_triangles = 0;
	Edge *e;

	for (GLsizei i = 0; i < delTri->n_edges; i++) {
		e = &(delTri->edges[i]);
		if ((!e->discarded) && (visited_edges[i] == 0)) {
			do {
				visited_edges[e->idx] = 1;
				e = e->onext->sym;
			} while (e->idx != i);
			n_triangles ++;
		}
	}

	if (visited_edges != NULL) free(visited_edges);

	return n_triangles - 1; // Remove the "outside"

}

/*
 * Returns the smallest angle in the DelaunayTriangulation.
 *
 * delTri:		the DelaunayTriangulation structure
 *
 * returns:		the smallest angle in the triangulation
 */
GLfloat getSmallestAngle(DelaunayTriangulation *delTri) {
	if ((delTri->n_points < 3) || (!delTri->success)) {
		return INFINITY;
	}
	char *visited_edges = calloc(delTri->n_edges, sizeof(char));
	GLfloat angle, smallest_angle;
	angle = smallest_angle = INFINITY;
	Edge *e;

	int outside_found = 0;

   	for (GLsizei i = 0; i < delTri->n_edges; i++) {
   		e = &(delTri->edges[i]);
   		if ((!e->discarded) && (visited_edges[i] == 0)) {
			// Only one set of edges is on the exterior
			// Once it's found, no need to check anymore
			if ((!outside_found) && (pointCompareEdge(delTri, e->onext->dest, e) == 1)) {
				do {
					// Edge has been now visited
	   				visited_edges[e->idx] = 1;
	   				e = e->onext->sym;
	   			} while (e->idx != i);
				outside_found = 1;
			}
			else {
				do {
					// Edge has been now visited
	   				visited_edges[e->idx] = 1;
					angle = angleBetweenContiguousEdges(delTri, e, e->onext->sym);
					smallest_angle = MIN(angle, smallest_angle);
	   				e = e->onext->sym;
	   			} while (e->idx != i);
			}
   		}
   	}

	if (visited_edges != NULL) free(visited_edges);

	return smallest_angle;

}

/*
 * Populates an array of n_triangles points with the center of each the
 * circumscribed circle in the DelaunayTriangulation.
 * Addionally, populates an second array with, for each triangle, the indices
 * of the neighboring triangles. -1 means that the neighbors is the "outside".
 *
 * delTri:		the DelaunayTriangulation structure
 * centers:		the preallocated array that will contain the centers
 * neighbors:	the preallocated array that will contain the neighbors
 * n_triangles:	the number of triangles (centers)
 */
void getVoronoiCentersAndNeighbors(DelaunayTriangulation *delTri,
								   GLfloat centers[][2],
								   GLsizei neighbors[][3],
							   	   GLsizei n_triangles) {

	if ((delTri->n_points < 3) || (!delTri->success)) {
		return;
	}

	// First, allocate some
   	char *visited_edges = calloc(delTri->n_edges, sizeof(char));
	GLsizei *edges_triangle = malloc(sizeof(GLsizei) * delTri->n_edges);

	GLsizei i_tri, i_e;
	i_tri = 0;

   	Edge *e;
	int outside_found = 0;

	GLsizei triPoints[3] = {0, 0, 0};

   	for (GLsizei i = 0; i < delTri->n_edges; i++) {
   		e = &(delTri->edges[i]);
   		if ((!e->discarded) && (visited_edges[i] == 0)) {
			// Only one set of edges is on the exterior
			// Once it's found, no need to check anymore
			if ((!outside_found) && (pointCompareEdge(delTri, e->onext->dest, e) == 1)) {
				do {
					// Edge has been now visited
	   				visited_edges[e->idx] = 1;
					// If the triangle is the outside, we store n_triangles + i_e
					// to later retrieve i_e without needed additional structure
					edges_triangle[e->idx] = n_triangles + e->idx ; // n_triangles = "outside"
	   				e = e->onext->sym;
	   			} while (e->idx != i);
				outside_found = 1;
			}
			else {
				i_e = 0;
				do {
					// Edge has been now visited
	   				visited_edges[e->idx] = 1;

					// This edge is inside triangles[i_tri] but has neighbor
					// given by the edge's symetrical
					neighbors[i_tri][i_e] = e->sym->idx;
					edges_triangle[e->idx] = i_tri;
					triPoints[i_e] = e->orig;
	   				e = e->onext->sym;
					i_e++;
	   			} while (e->idx != i);

				circleCenter(delTri, triPoints[0], triPoints[1], triPoints[2], centers[i_tri]);

				i_tri++;
			}
   		}
   	}

	// Each triangle will now have have its neighbors to be Voronoi centers
	for(i_tri = 0; i_tri < n_triangles; i_tri++) {
		neighbors[i_tri][0] = edges_triangle[neighbors[i_tri][0]];
		neighbors[i_tri][1] = edges_triangle[neighbors[i_tri][1]];
		neighbors[i_tri][2] = edges_triangle[neighbors[i_tri][2]];

	}

	// Free allocated memory
   	if (visited_edges != NULL) free(visited_edges);
	if (edges_triangle != NULL) free(edges_triangle);

}

/*
 * Populates an array with 3 * 2 * n_triangles points, that will be the lines
 * that link Voronoi centers with their neighbors.
 *
 * delTri:		the DelaunayTriangulation structure
 * centers:		the centers
 * neighbors:	the neighbors
 * lines:		the preallocated array that will contain the lines
 * n_triangles:	the number of triangles (centers)
 */
void getVoronoiLines(DelaunayTriangulation *delTri,
					 GLfloat centers[][2],
					 GLsizei neighbors[][3],
					 GLfloat lines[][2],
					 GLsizei n_triangles) {

	if ((delTri->n_points < 3) || (!delTri->success)) {
		return;
	}

	GLsizei i_tri, i_nei, i;
	GLsizei i_e;
	GLfloat x, y, xa, ya, xb, yb, m, p, det, xp, yp, factor, dx;
	GLfloat *a, *b;

	GLsizei l_i = 0;

	for(i_tri = 0; i_tri < n_triangles; i_tri++) {
		for (i = 0; i < 3; i++) {
			x = centers[i_tri][0];
			y = centers[i_tri][1];
			lines[l_i    ][0] = x;
			lines[l_i    ][1] = y;
			i_nei = neighbors[i_tri][i];

			if (i_nei >= n_triangles){ // neighbor is "outside"
				// Circumcenter is projected on the edge

				i_e = i_nei - n_triangles;
				a = delTri->points[delTri->edges[i_e].orig];
				xa = a[0];
				ya = a[1];
				b = delTri->points[delTri->edges[i_e].dest];
				xb = b[0];
				yb = b[1];


				dx = xb - xa;
				if (dx == 0) {
					xp = xa;
					yp = y;
				}
				else {
					m = (yb - ya) / dx;
					p = ya - xa * m;
					det = 1.0 / (1.0 + m * m);

					xp = (x + m * (y - p)) * det;
					yp = (p + m * (m * y + x)) * det;
				}

				factor = 100.0;

				// If point is on the left, then it is outside the convex hull
				// and the ligne must point outward
				if (orient2d(centers[i_tri], a, b) > 0) {
					factor = - factor;
				}

				lines[l_i + 1][0] = x + factor * (xp - x);
				lines[l_i + 1][1] = y + factor * (yp - y);
			}
			else {
				lines[l_i + 1][0] = centers[i_nei][0];
				lines[l_i + 1][1] = centers[i_nei][1];
			}
			l_i += 2;
		}
	}

}

/*
 * Save the last DelaunayTriangulation status in the file.
 * This will contains the points in the triangulation and, if success, the points
 * forming all the lines in the triangulation.
 *
 * delTri:		the DelaunayTriangulation structure
 * file_out:	the output file (already open)
 */
void fsaveDelaunayTriangulation(DelaunayTriangulation *delTri, FILE* file_out) {
	GLsizei n_lines_points = 2 * getDelaunayTriangulationNumberOfLines(delTri);

	fprintf(file_out, "%d %d\n", delTri->n_points, n_lines_points);

	GLsizei i = 0;

	for (i = 0; i < delTri->n_points; i++) {
		fprintf(file_out, "%lf %lf\n", delTri->points[i][0], delTri->points[i][1]);
	}

	GLfloat (*linesPoints)[2] = malloc(sizeof(linesPoints[0]) * n_lines_points);
	getDelaunayTriangulationLines(delTri, linesPoints, n_lines_points / 2);

	for (i = 0; i < n_lines_points; i++) {
		fprintf(file_out, "%lf %lf\n", linesPoints[i][0], linesPoints[i][1]);
	}

	if (linesPoints != NULL) free(linesPoints);
}

////////////////////////////////////////////////
// End: DelaunayTriangulation structure utils //
////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////
// Begin: Edge structure utils //
/////////////////////////////////

/*
 * Creates a new edge in the triangulation, between two points, and returns the
 * index at which it is stored. It will also polulate the next edge as the
 * opposite (symetrical) edge of the one just created.
 *
 * delTri: 		the DelaunayTriangulation structure
 * orig: 		the index of the origin point of the edge
 * dest: 		the index of the destination point of the edge
 *
 * returns:		an Edge structure pointer to the main edge
 */
Edge* addEdge(DelaunayTriangulation *delTri, GLsizei orig, GLsizei dest) {
	// Enough space ?
	if (delTri->n_edges_max <= delTri->n_edges) {
		printf("ERROR, no enough edges allocated\n");
		exit(1);
	}

	Edge *e, *s;

	// Main edge
	e = &(delTri->edges[delTri->n_edges]);
	e->idx = delTri->n_edges;
	delTri->n_edges += 1;

	e->discarded = 0;
	e->orig = orig;
	e->dest = dest;

	// Symetrical edge
	s =&(delTri->edges[delTri->n_edges]);
	s->idx = delTri->n_edges;
	delTri->n_edges += 1;

	s->discarded = 0;
	s->orig = dest;
	s->dest = orig;

	// Bind the two edges together
	e->sym = s;
	s->sym = e;

	e->onext = e;
	e->oprev = e;
	s->onext = s;
	s->oprev = s;

	//describeEdge(e);
	return e;
}

/*
 * Describes an Edge structure by printing some information.
 *
 * e:			the Edge structure
 */
void describeEdge(Edge *e) {

	printf("Edge structure stored at %p.\n"
		   "\t- Index:                      %d\n"
		   "\t- Origin index:               %d\n"
		   "\t- Destination index:          %d\n"
		   "\t- Discarded?:                 %d\n",
		   e,
		   e->idx, e->orig, e->dest,
	   	   e->discarded);
}


/*
 * Combines two distinct edges.
 *
 * delTri: 		the DelaunayTriangulation structure
 * a:	 		the first edge
 * b: 			the second edge
 */
void spliceEdges(DelaunayTriangulation *delTri, Edge *a, Edge *b) {
	if (a->idx == b->idx) {
		return;
	}

	Edge *tmp;
	a->onext->oprev = b;
	b->onext->oprev = a;

	tmp = a->onext;
	a->onext = b->onext;
	b->onext = tmp;
}

/*
 * Creates a new edge connecting the destination of the first edge to the origin
 * of the second edge
 *
 * delTri: 		the DelaunayTriangulation structure
 * a: 			the first edge
 * b: 			the second edge
 *
 * returns:		an Edge structure pointer to the new edge
 */
Edge* connectEdges(DelaunayTriangulation *delTri, Edge *a, Edge *b) {
	Edge *e = addEdge(delTri, a->dest, b->orig);
	spliceEdges(delTri, e, 		a->sym->oprev);
	spliceEdges(delTri, e->sym, b);
	return e;
}

/*
 * Deletes an edge from the DelaunayTriangulation by discarding the edge and its
 * symetrical edge.
 *
 * delTri: 		the DelaunayTriangulation structure
 * e: 			the edge
 */
void deleteEdge(DelaunayTriangulation *delTri, Edge *e) {
	spliceEdges(delTri, e, e->oprev);
	spliceEdges(delTri, e->sym, e->sym->oprev);

	// Discard the edges from the data structure
	e->discarded = 1;
	e->sym->discarded = 1;
	delTri->n_edges_discarded += 2;
}

///////////////////////////////
// End: Edge structure utils //
///////////////////////////////

////////////////////////////////////////////////////////////////////////////////

///////////////////////////
// Begin: Geometry utils //
///////////////////////////

/*
 * Compares two points with (x, y) coordinates such that they will be sorted
 * accordingly to their x component, then their y component.
 *
 * a_v:			the pointer to the first point
 * b_v:			the pointer to the second point
 *
 * returns:		the comparison between the two points
 */
static int compare_points(const void *a_v, const void *b_v)
{
	GLfloat* a = *(GLfloat(*)[2]) a_v;
	GLfloat* b = *(GLfloat(*)[2]) b_v;

	GLfloat diff = a[0] - b[0];

	if (diff == 0) {
		diff = a[1] - b[1];
	}
	return (diff>0) - (diff<0);
}

/*
 * Returns the min and max x and y coordinates in the collection of points
 *
 * points:		the points
 * n_points:	the number of points
 * bounds:		the preallocated array that will contain the points {min, max}
 */
void getPointsBoudingBox(GLfloat points[][2], GLsizei n_points, GLfloat bounds[][2]) {
	if (n_points == 0) return;

	double x_min, x_max, y_min, y_max;
	x_min = x_max = points[0][0];
	y_min = y_max = points[0][1];

	for (GLsizei i = 1; i < n_points; i++) {
		x_min = MIN(points[i][0], x_min);
		x_max = MAX(points[i][0], x_max);
		y_min = MIN(points[i][1], y_min);
		y_max = MAX(points[i][1], y_max);
	}

	bounds[0][0] = x_min;
	bounds[0][1] = y_min;
	bounds[1][0] = x_max;
	bounds[1][1] = y_max;
}

/*
 * Indicates wether a point is inside a circumscribed circle.
 *
 * delTri:		the DelaunayTriangulation structure
 * i_p:			the index of the point
 * i_a,b,c:		the index of the points of the triangle
 *
 * returns:		1 if the point lies inside the circle, 0 otherwise
 */
int pointInCircle(DelaunayTriangulation *delTri, GLsizei i_p, GLsizei i_a, GLsizei i_b, GLsizei i_c) {
    // https://www.cs.cmu.edu/~quake/robust.html
	GLfloat *point, *a, *b, *c;
	point = delTri->points[i_p];
	a = delTri->points[i_a];
	b = delTri->points[i_b];
	c = delTri->points[i_c];

#if ROBUST
	return incircle(a, b, c, point) > 0;
#else

    GLfloat a1, a2, a3, b1, b2, b3, c1, c2, c3, det;

	a1 = a[0] - point[0]; a2 = a[1] - point[1];
	b1 = b[0] - point[0]; b2 = b[1] - point[1];
	c1 = c[0] - point[0]; c2 = c[1] - point[1];

	a3 = a1 * a1 + a2 * a2;
	b3 = b1 * b1 + b2 * b2;
	c3 = c1 * c1 + c2 * c2;

	det = a1*b2*c3 + a2*b3*c1 + a3*b1*c2 - (a3*b2*c1 + a1*b3*c2 + a2*b1*c3);
    return det < 0;
#endif
}

/*
 * Determines the center of the circumscribed circle based on triangle points.
 *
 * delTri:		the DelaunayTriangulation structure
 * i_a,b,c:		the index of the points of the triangle
 * center:		the preallocated array that will contain the center
 *
 * returns:		the diameter of the circle
 */
GLfloat circleCenter(DelaunayTriangulation *delTri, GLsizei i_a, GLsizei i_b, GLsizei i_c, GLfloat center[2]) {
	// https://www.codewars.com/kata/5705785658b58f387b001ffc
	GLfloat *point, *a, *b, *c;
	a = delTri->points[i_a];
	b = delTri->points[i_b];
	c = delTri->points[i_c];
	GLfloat aa, bb, cc, dy_bc, dy_ca, dy_ab, d;

	aa = a[0] * a[0] + a[1] * a[1];
	bb = b[0] * b[0] + b[1] * b[1];
	cc = c[0] * c[0] + c[1] * c[1];

	dy_bc = b[1] - c[1];
	dy_ca = c[1] - a[1];
	dy_ab = a[1] - b[1];

	d = 2 * (a[0] * dy_bc + b[0] * dy_ca + c[0] * dy_ab);

	center[0] = (aa * dy_bc + bb * dy_ca + cc * dy_ab) / d;
	// WARNING: there is an error is the link, it is Bx - Ax (and not the opposite)
	center[1] = (aa * (c[0] - b[0]) + bb * (a[0] - c[0]) + cc * (b[0] - a[0])) / d;

	GLfloat dx, dy;

	dx = a[0] - center[0];
	dy = a[1] - center[1];

	return sqrt(dx * dx + dy * dy);
}

/*
 * Compares the position of a point relatively to an edge.
 *
 * delTri:		the DelaunayTriangulation structure
 * i_p:			the index of the point
 * e:			the edge
 * returns:		the comparison (see below)
 *			1	if point is on the right
 *			0	if point is colinear with edge
 *		   -1	if point is on the left
 */
int pointCompareEdge(DelaunayTriangulation *delTri, GLsizei i_p, Edge *e) {

	GLfloat *point, *orig, *dest, det;
	point = delTri->points[i_p];
	orig = delTri->points[e->orig];
	dest = delTri->points[e->dest];

#if ROBUST

	det = -orient2d(orig, dest, point);

#else

	det = (orig[0] - point[0]) * (dest[1] - point[1]) - (orig[1] - point[1]) * (dest[0] - point[0]);

#endif

	return (det>0) - (det<0);
}

/*
 * Computes the angle between to contiguous edges.
 *
 * delTri:		the DelaunayTriangulation structured
 * e:			the first edge
 * f:			the second edge (with f->dest == e->dest)
 * returns:		the angle
 */
GLfloat angleBetweenContiguousEdges(DelaunayTriangulation *delTri, Edge *e, Edge *f) {
	// https://www.omnicalculator.com/math/angle-between-two-vectors
	GLfloat *point, *a, *b, *c;
	GLfloat xa, xb, xc, ya, yb, yc;
	a = delTri->points[e->orig];
	b = delTri->points[e->dest];
	c = delTri->points[f->orig];

	xa = a[0]; xb = b[0]; xc = c[0];
	ya = a[1]; yb = b[1]; yc = c[1];

	GLfloat dx_e, dx_f, dy_e, dy_f;

	dx_e = xb - xa; dx_f = xc - xb;
	dy_e = yb - ya; dy_f = yc - yb;

	return acos(-(dx_e * dx_f + dy_e * dy_f) / (sqrt(dx_e * dx_e + dy_e * dy_e) * sqrt(dx_f * dx_f + dy_f * dy_f)));
}

/////////////////////////
// End: Geometry utils //
/////////////////////////

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////
// Begin: Triangulation functions //
////////////////////////////////////

/*
 * Triangulates a set of points using the DelaunayTriangulation.
 * This function should be the main function which will all the other sub-functions.
 *
 * delTri: 		the DelaunayTriangulation structure
 */
void triangulateDT(DelaunayTriangulation *delTri) {
	if (delTri->success) {
		return;
	}
	if (delTri->n_points < 2) {
		return;
	}

	// Sort points by x coordinates then by y coordinate.
	qsort(delTri->points, delTri->n_points, 2 * sizeof(GLfloat), compare_points);

	/// Starts the triangulation using a divide and conquer approach.
	Edge *l, *r;
	triangulate(delTri, 0, delTri->n_points, &l, &r);
	delTri->success = 1;
}

/*
 * Recursive function used by the triangulateDT function in order to solve
 * the triangulation problem using a divide and conquer approach.
 *
 * delTri: 		the DelaunayTriangulation structure
 * start:		the start index of the slice
 * end:			the (excluded) end index of the slice
 * el:			an Edge structure pointer for the left edge
 * er:			an Edge structure pointer for the right edge
 *
 */
void triangulate(DelaunayTriangulation *delTri, GLsizei start, GLsizei end, Edge **el, Edge **er) {
	GLsizei n = end - start;
	if (n == 2) {
		// Creates an edge connecting the two points (start), (start + 1)
		Edge *e = addEdge(delTri, start, start + 1);
		*el = e;
		*er = e->sym;
		return;
	}
	else if (n == 3) {
		Edge *a, *b, *c;

		// Creates two edges
		// - a, connecting (start), 	(start + 1)
		// - b, connecting (start + 1), (start + 2)
		a = addEdge(delTri, start, 		start + 1);
		b = addEdge(delTri, start + 1, 	start + 2);
		spliceEdges(delTri, a->sym, b);

		int cmp = pointCompareEdge(delTri, start + 2, a);

		// Now will close the triangle formed by the three points
		if (cmp == 1) {
			c = connectEdges(delTri, b, a);
			*el = a;
			*er = b->sym;
			return;
		}
		else if (cmp == -1) {
			c = connectEdges(delTri, b, a);
			*el = c->sym;
			*er = c;
			return;
		}
		else {
			*el = a;
			*er = b->sym;
			return;
		}
	}
	else {
		// Recusively calls this function on half the points
		GLsizei m = (n + 1) / 2;
		Edge *ldo, *ldi, *rdi, *rdo;
		triangulate(delTri, start, 		start + m, 	&ldo, &ldi);
		triangulate(delTri, start + m, 	end, 		&rdi, &rdo);

		// Computes the upper common tangent of left and right edges
		while (1) {
			if 		(pointCompareEdge(delTri, rdi->orig, ldi) ==  1) {
				ldi = ldi->sym->onext;
			}
			else if (pointCompareEdge(delTri, ldi->orig, rdi) == -1) {
				rdi = rdi->sym->oprev;
			}
			else {
				break;
			}
		}

		Edge *base;

		// Creates an edge between rdi.orig and ldi.orig
		base = connectEdges(delTri, ldi->sym, rdi);

		// Ajdusts ldo and rdo
		if (ldi->orig == ldo->orig) {
			ldo = base;
		}
		if (rdi->orig == rdo->orig) {
			rdo = base->sym;
		}

		Edge *lcand, *rcand, *tmp;
		int v_rcand, v_lcand;

		// We merge both parts
		while (1) {
			// Locates the first right and left points to be encountered
			// by the diving bubble

			rcand = base->sym->onext;
			lcand = base->oprev;

			v_rcand = (pointCompareEdge(delTri, rcand->dest, base) == 1);
			v_lcand = (pointCompareEdge(delTri, lcand->dest, base) == 1);
			if (!(v_rcand || v_lcand)) {
				// Merge is done
				break;
			}
			// Deletes right edges that fail the circle test
			if (v_rcand) {
				while ((pointCompareEdge(delTri, rcand->onext->dest, base) == 1) &&
					   (pointInCircle(delTri, rcand->onext->dest, base->dest, base->orig, rcand->dest))
				   )
					   {
						   tmp = rcand->onext;
						   deleteEdge(delTri, rcand);
						   rcand = tmp;
					   }
			}
			// Deletes left edges that fail the circle test
			if (v_lcand) {
				while ((pointCompareEdge(delTri, lcand->oprev->dest, base) == 1) &&
					   (pointInCircle(delTri, lcand->oprev->dest, base->dest, base->orig, lcand->dest))
				   )
					   {
						   tmp = lcand->oprev;
						   deleteEdge(delTri, lcand);
						   lcand = tmp;
					   }
			}

			if ((!v_rcand) ||
				(v_rcand && pointInCircle(delTri, lcand->dest, rcand->dest, rcand->orig, lcand->orig))
			) {

				tmp = connectEdges(delTri, lcand, base->sym);
				base = tmp;
			}
			else {
				tmp = connectEdges(delTri, base->sym, rcand->sym);
				base = tmp;
			}
		}

		*el = ldo;
		*er = rdo;
	}
}

//////////////////////////////////
// End: Triangulation functions //
//////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////
// Begin: Drawing functions //
//////////////////////////////

/*
 * Returns the position of the mouse in the x, y coordinates.
 *
 * window:		the window
 * mouse_pos:	the preallocated array that will contain the position
 */
void getMousePosition(bov_window_t *window, GLfloat mouse_pos[2]) {
	mouse_pos[0] = (window->cursorPos[0] - window->param.translate[0]);
	mouse_pos[1] = (window->cursorPos[1] - window->param.translate[1]);
}

/*
 * Fills the information text with some data.
 *
 * delTri:			the DelaunayTriangulation structure
 * info_text_char:	the preallocated array that will contain the text
 */
void getInfoText(DelaunayTriangulation *delTri, char *info_text_char) {
	sprintf(info_text_char, "Number of points: %7d / Smallest angle : %2.2f\xba", delTri->n_points, RAD2DEG(getSmallestAngle(delTri)));
}

/*
 * Provides quite a few tools to visualize the DelaunayTriangulation.
 *
 * delTri:		the DelaunayTriangulation structure
 * window:		the window
 * total_time:	the total time the animation should approximatively last
 */
void drawDelaunayTriangulation(DelaunayTriangulation *delTri, bov_window_t *window, double total_time) {
	// Information text
	bov_text_t* text = bov_text_new(
		(GLubyte[]) {"This plot is interactive!\n"
		             "\xf8 Press [A/D] to add/delete a point nearby your cursor\n"
					 "\xee Hold [SHIFT] while pressing [A/D] to repeat\n"
					 "\xf8 Hold  [S]   to select a point nearby your cursor and\n"
					 "              change its location\n"
					 "\xf8 Press [O]   to show/hide points\n"
					 "\xf8 Press [L]   to show/hide triangulation lines\n"
					 "\xf8 Press [V]   to show/hide Voronoi diagram\n"
		             "\xf8 Press [F]   to switch between fast and pretty drawing\n"
					 "\xf8 Press [I]   to illustrate each step of the triangulation\n"
					 "\xf8 Press [G]   to show/hide the SMallest Angle Game rules\n"
				     "\xf8 Press [X]   to show/hide this text\n"
				     "\xf8 Press [H]   to show/hide the default help menu\n\n"
				 	 "\x8a Features using DRAG & DROP may not work correctly\n"
					 "with a trackpad: please use a mouse."},
		GL_STATIC_DRAW);
	bov_text_set_space_type(text, PIXEL_SPACE);
	bov_text_param_t text_parameters = bov_text_get_param(text);
	text_parameters.fontSize *= .7;
	bov_text_set_param(text, text_parameters);

	GLubyte info_text_char[200] = {""};

	getInfoText(delTri, info_text_char);

	bov_text_t* info_text = bov_text_new(info_text_char, GL_STATIC_DRAW);
	bov_text_set_space_type(info_text, PIXEL_SPACE);

	// Game rules
	bov_text_t* rules = bov_text_new(
		(GLubyte[]) {"\t\t\t\t\tPlay the SMallest Angle Game (SMAG) !\n"
		 			 "\n"
					 "Delaunay triangulation is used because it maximises the smallest angle.\n"
					 "\n"
					 "Here are the rules:\n"
					 "\n"
		             "\t1. Choose a minimum angle you should never be lower than (eg.: 5\xba)\n"
		             "\t2. Delete all the points or restart with -n 0 flag\n"
					 "\t3. Try to add new points while staying above the minimum angle\n"
					 "\t4. When you break the limit, the game is over!\n"
					 "\t   Record the # of points you reached and challenge you friends to do better\n"
					 "\n"
					 "Alternatively, you can play with friends on the same computer, and\n"
					 "the one who first breaks the limit loses the game."},
		GL_STATIC_DRAW);
	bov_text_set_space_type(rules, PIXEL_SPACE);
	bov_text_param_t rules_parameters = bov_text_get_param(rules);
	rules_parameters.fontSize *= .7;
	bov_text_set_param(rules, rules_parameters);

	// Points
    bov_points_t *pointsDraw = bov_points_new(delTri->points, delTri->n_points, GL_STATIC_DRAW);
	bov_points_set_color(pointsDraw, BASIC_POINTS_COLOR);
	bov_points_set_outline_color(pointsDraw, BASIC_POINTS_OUTLINE_COLOR);
	bov_points_set_width(pointsDraw, BASIC_POINTS_WIDTH);

	GLfloat bounds[][2] = {{0.0, 0.0}, {0.0, 0.0}};

	// Lines (edges)
	GLsizei n_lines = 0;
	GLfloat (*linesPoints)[2] = NULL;
	bov_points_t *linesDraw = bov_points_new(linesPoints, n_lines, GL_STATIC_DRAW);
	bov_points_set_color(linesDraw, TRIANGULATION_LINES_COLOR);
	bov_points_set_width(linesDraw, TRIANGULATION_LINES_WIDTH);
	bov_points_set_outline_color(linesDraw, TRIANGULATION_LINES_OUTLINE_COLOR);
	bov_points_set_outline_width(linesDraw, 0.5 * TRIANGULATION_LINES_WIDTH);

	// If DelaunayTriangulation was computed, will display it
	if (delTri->success) {
		n_lines = getDelaunayTriangulationNumberOfLines(delTri);
		linesPoints = malloc(sizeof(linesPoints[0]) * 2 * n_lines);
		getDelaunayTriangulationLines(delTri, linesPoints, n_lines);
		bov_points_update(linesDraw, linesPoints, 2 * n_lines);
	}

	// Some key bindings
	// WARNING: here, I suppose you have an AZERTY layout
	int FAST = (delTri->n_points > 100);
	int VORONOI = 0;
	int REQUIRE_UPDATE = 0;
	int HIDE_TEXT = 0;
	int SHOW_POINTS = 1;
	int SHOW_LINES = 1;
	int SHOW_GAME_RULES = 0;
	int ILLUSTRATE = 0;
	int KEY_A, KEY_D, KEY_S, KEY_F, KEY_V, KEY_X, KEY_O, KEY_L, KEY_I, KEY_G;
	int KEY_SHIFT;
	int LAST_KEY_A, LAST_KEY_D, LAST_KEY_F, LAST_KEY_V, LAST_KEY_X, LAST_KEY_O, LAST_KEY_L, LAST_KEY_I, LAST_KEY_G;
	LAST_KEY_A = LAST_KEY_D = LAST_KEY_F = LAST_KEY_V = LAST_KEY_X = LAST_KEY_O = LAST_KEY_L = LAST_KEY_I = LAST_KEY_G = 0;

	int idx = -1;

	// Mouse
	GLfloat mousePoint[][2] = {{0.0, 0.0}};
	bov_points_t *mouseDraw = bov_points_new(mousePoint, 1, GL_STATIC_DRAW);
	bov_points_set_color(mouseDraw, MOUSE_POINTS_COLOR);
	bov_points_set_outline_color(mouseDraw, MOUSE_POINTS_OUTLINE_COLOR);
	bov_points_set_width(mouseDraw, MOUSE_POINTS_WIDTH);
	bov_points_set_outline_width(mouseDraw, -.1);

	// Voronoi
	GLsizei n_triangles = 0;
	GLfloat (*voronoiCenters)[2] = NULL;
	bov_points_t *voronoiCentersDraw = bov_points_new(voronoiCenters, n_triangles, GL_STATIC_DRAW);
	bov_points_set_color(voronoiCentersDraw, VORONOI_POINTS_COLOR);
	bov_points_set_outline_color(voronoiCentersDraw, VORONOI_POINTS_OUTLINE_COLOR);
	bov_points_set_width(voronoiCentersDraw, VORONOI_POINTS_WIDTH);

	GLsizei n_voronoi_lines = 0;
	GLsizei (*voronoiNeighbors)[3] = NULL;
	GLfloat (*voronoiLines)[2] = NULL;
	bov_points_t *voronoiLinesDraw = bov_points_new(linesPoints, n_lines, GL_STATIC_DRAW);
	bov_points_set_color(voronoiLinesDraw, VORONOI_LINES_COLOR);
	bov_points_set_width(voronoiLinesDraw, VORONOI_LINES_WIDTH);
	bov_points_set_outline_color(voronoiLinesDraw, VORONOI_LINES_OUTLINE_COLOR);
	bov_points_set_outline_width(voronoiLinesDraw, .5 * VORONOI_LINES_WIDTH);

	// Write keystrokes in a file

	FILE *file_out = fopen("data/.keys.txt", "w");
	if (file_out == NULL) {
		printf("Warning: could not open file data/.keys.txt.\n"
			   "Make sure to run this program from the project directory to enable keystrokes saving.\n");
	}

	// Animation parameters
	DTDrawingParameters *DTDparams;


	while(!bov_window_should_close(window)){
		// 1. Handle key bindings
		getMousePosition(window, mousePoint[0]);
		KEY_A = glfwGetKey(window->self, GLFW_KEY_Q); // QWERTY -> AZERTY layout
		KEY_D = glfwGetKey(window->self, GLFW_KEY_D);
		KEY_S = glfwGetKey(window->self, GLFW_KEY_S);
		KEY_F = glfwGetKey(window->self, GLFW_KEY_F);
		KEY_V = glfwGetKey(window->self, GLFW_KEY_V);
		KEY_X = glfwGetKey(window->self, GLFW_KEY_X);
		KEY_O = glfwGetKey(window->self, GLFW_KEY_O);
		KEY_L = glfwGetKey(window->self, GLFW_KEY_L);
		KEY_I = glfwGetKey(window->self, GLFW_KEY_I);
		KEY_G = glfwGetKey(window->self, GLFW_KEY_G);
		KEY_SHIFT = glfwGetKey(window->self, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window->self, GLFW_KEY_RIGHT_SHIFT);

		if (KEY_A) {
			if ((!LAST_KEY_A) || KEY_SHIFT) {
				REQUIRE_UPDATE = addPoint(delTri, mousePoint[0]);
				LAST_KEY_A = KEY_A;
				if (file_out != NULL) {
					fprintf(file_out, "A\n");
					fflush(file_out);
				}
			}
		}
		else {
			LAST_KEY_A = KEY_A;
		}
		if (KEY_D) {
			if ((!LAST_KEY_D) || KEY_SHIFT) {
				REQUIRE_UPDATE = deletePoint(delTri, mousePoint[0]);
				LAST_KEY_D = KEY_D;
				if (file_out != NULL) {
					fprintf(file_out, "D\n");
					fflush(file_out);
				}
			}
		}
		else {
			LAST_KEY_D = KEY_D;
		}
		if (KEY_S) {
			idx = getPointIndex(delTri, mousePoint[0]);
			updatePointAtIndex(delTri, idx, mousePoint[0]);
			REQUIRE_UPDATE = 1;
			if (file_out != NULL) {
				fprintf(file_out, "S\n");
				fflush(file_out);
			}
		}
		else {
			idx = -1;
		}
		if (KEY_F) {
			if (!LAST_KEY_F) {
				FAST = !FAST;
				LAST_KEY_F = KEY_F;
				if (file_out != NULL) {
					fprintf(file_out, "F\n");
					fflush(file_out);
				}
			}
		}
		else {
			LAST_KEY_F = KEY_F;
		}
		if (KEY_V) {
			if (!LAST_KEY_V) {
				VORONOI = !VORONOI;
				LAST_KEY_V = KEY_V;
				REQUIRE_UPDATE = 1;
				if (file_out != NULL) {
					fprintf(file_out, "V\n");
					fflush(file_out);
				}
			}
		}
		else {
			LAST_KEY_V = KEY_V;
		}
		if (KEY_X) {
			if (!LAST_KEY_X) {
				HIDE_TEXT = !HIDE_TEXT;
				LAST_KEY_X = KEY_X;
			}
		}
		else {
			LAST_KEY_X = KEY_X;
		}
		if (KEY_O) {
			if (!LAST_KEY_O) {
				SHOW_POINTS = !SHOW_POINTS;
				LAST_KEY_O = KEY_O;
			}
		}
		else {
			LAST_KEY_O = KEY_O;
		}
		if (KEY_L) {
			if (!LAST_KEY_L) {
				SHOW_LINES = !SHOW_LINES;
				LAST_KEY_L = KEY_L;
			}
		}
		else {
			LAST_KEY_L = KEY_L;
		}
		if (KEY_I) {
			if (!LAST_KEY_I) {
				LAST_KEY_I = KEY_I;
				ILLUSTRATE = 1;
			}
		}
		else {
			LAST_KEY_I = KEY_I;
		}
		if (KEY_G) {
			if (!LAST_KEY_G) {
				SHOW_GAME_RULES = !SHOW_GAME_RULES;
				LAST_KEY_G = KEY_G;
			}
		}
		else {
			LAST_KEY_G = KEY_G;
		}

		if (SHOW_GAME_RULES) {
			rules_parameters.pos[0] = bov_window_get_xres(window) / 2 - 400;
			rules_parameters.pos[1] = bov_window_get_yres(window) / 2 + 150;
			bov_text_set_param(rules, rules_parameters);

			bov_text_draw(window, rules);
			bov_window_update(window);
			continue;
		}

		// 0. We required illustration mode

		else if (ILLUSTRATE) {
			resetDelaunayTriangulation(delTri);

			getPointsBoudingBox(delTri->points, delTri->n_points, bounds);

			// Active points
		    bov_points_t *activePointsDraw = bov_points_new(delTri->points, delTri->n_points, GL_STATIC_DRAW);
			bov_points_set_color(activePointsDraw, ACTIVE_POINTS_COLOR);
			bov_points_set_outline_color(activePointsDraw, ACTIVE_POINTS_OUTLINE_COLOR);
			bov_points_set_width(activePointsDraw, ACTIVE_POINTS_WIDTH);

			// Trying to make the sleep time % to the inverse of the number of edges

			int sleep = (int) (total_time / (delTri->n_edges_max));

			DTDparams = initDTDrawingParameters(
				delTri,
				window,
				linesPoints,
				pointsDraw,
				activePointsDraw,
				linesDraw,
				bounds,
				FAST,
				sleep
			);

			triangulateDTIllustrated(DTDparams);

			freeDTDrawingParameters(DTDparams);

			bov_points_delete(activePointsDraw);

			ILLUSTRATE = 0;
		}

		// 1.A If key bindings required an update in the drawing
		if (REQUIRE_UPDATE) {
			// Recompute triangulation
			triangulateDT(delTri);
			getInfoText(delTri, info_text_char);

			getPointsBoudingBox(delTri->points, delTri->n_points, bounds);

			//printf("bounds = (%f, %f) (%f, %f)\n", bounds[0][0], bounds[0][1], bounds[1][0], bounds[1][1]);

			if (delTri->success) {

				// Free old lines
				if (linesPoints != NULL) free(linesPoints);

				// Get new lines
				n_lines = getDelaunayTriangulationNumberOfLines(delTri);
				linesPoints = malloc(sizeof(linesPoints[0]) * 2 * n_lines);
				getDelaunayTriangulationLines(delTri, linesPoints, n_lines);

				// Update lines
				bov_points_update(linesDraw, linesPoints, 2 * n_lines);

				if (VORONOI) {
					// Free old data
					if (voronoiCenters != NULL) free(voronoiCenters);
					if (voronoiNeighbors != NULL) free(voronoiNeighbors);
					if (voronoiLines != NULL) free(voronoiLines);


					// Get new Voronoi centers and lines
					n_triangles = getNumberOfTriangles(delTri);

					voronoiCenters = malloc(sizeof(voronoiCenters[0]) * n_triangles);
					voronoiNeighbors = malloc(sizeof(voronoiNeighbors[0]) * n_triangles);

					getVoronoiCentersAndNeighbors(delTri,
												  voronoiCenters,
											      voronoiNeighbors,
											      n_triangles);

					// Update Voronoi centers
					bov_points_update(voronoiCentersDraw, voronoiCenters, n_triangles);

					voronoiLines = malloc(sizeof(voronoiLines[0]) * 3 * 2 * n_triangles);

					getVoronoiLines(delTri,
									voronoiCenters,
									voronoiNeighbors,
									voronoiLines,
									n_triangles);

					bov_points_update(voronoiLinesDraw, voronoiLines, 3 * 2 * n_triangles);


				}
			}

			// Update new points
			bov_points_update(pointsDraw, delTri->points, delTri->n_points);

			// Draws a red point where the mouse was when key was pressed
			bov_points_update(mouseDraw, mousePoint, 1);
			bov_points_set_color(mouseDraw, (GLfloat[4]) {1.0, 0.0, 0.0, 1.0});
			bov_points_set_width(mouseDraw, MOUSE_POINTS_WIDTH);

			GLsizei n_triangles = getNumberOfTriangles(delTri);

			REQUIRE_UPDATE = 0;
		}
		// 1.B If no update required, a smooth fading is applied to the red point
		else {
			bov_points_param_t param = bov_points_get_param(mouseDraw);
			param.fillColor[3] *= 0.95;
			param.width -= (param.width - BASIC_POINTS_WIDTH) / 10;
			bov_points_set_param(mouseDraw, param);
		}

		// 2. Drawing
		if (delTri->success) {
			if (FAST) {
				if (SHOW_LINES) bov_fast_lines_draw(window, linesDraw, 0, BOV_TILL_END);
				if (VORONOI) {
					bov_fast_lines_draw(window, voronoiLinesDraw, 0, BOV_TILL_END);
					bov_fast_points_draw(window, voronoiCentersDraw, 0, BOV_TILL_END);
				}
			}
			else {
				if (SHOW_LINES) bov_lines_draw(window, linesDraw, 0, BOV_TILL_END);
				if (VORONOI) {
					bov_lines_draw(window, voronoiLinesDraw, 0, BOV_TILL_END);
					bov_points_draw(window, voronoiCentersDraw, 0, BOV_TILL_END);
				}
			}
		}
		if (FAST) {
			if (SHOW_POINTS) bov_fast_points_draw(window, pointsDraw, 0, BOV_TILL_END);
			bov_fast_points_draw(window, mouseDraw, 0, 1);
		}
		else {
			if (SHOW_POINTS) bov_points_draw(window, pointsDraw, 0, BOV_TILL_END);
			bov_points_draw(window, mouseDraw, 0, 1);
		}

		// 3. Adjust text place and boldness

		double wtime = bov_window_get_time(window);

		text_parameters.pos[1] = bov_window_get_yres(window) - 30 ;
		text_parameters.fillColor[0] = 0.35 * sin(2 * wtime) + 0.35;
		bov_text_set_param(text, text_parameters);

		bov_text_set_boldness(text, 0.3 * sin(2 * wtime) + 0.3);

		if (!HIDE_TEXT) {

			bov_text_update(info_text, info_text_char);

			bov_text_draw(window, text);
			bov_text_draw(window, info_text);
		}

		// 4. Update windows
		bov_window_update(window);
	}

	// Free all memory allocated
	bov_text_delete(text);
	bov_text_delete(info_text);
	bov_text_delete(rules);
	bov_points_delete(pointsDraw);
	bov_points_delete(linesDraw);
	bov_points_delete(mouseDraw);
	bov_points_delete(voronoiCentersDraw);
	bov_points_delete(voronoiLinesDraw);

	if (linesPoints != NULL) free(linesPoints);
	if (voronoiCenters != NULL) free(voronoiCenters);
	if (voronoiNeighbors != NULL) free(voronoiNeighbors);
	if (voronoiLines != NULL) free(voronoiLines);

	if (file_out != NULL) fclose(file_out);
}

/*
 * Re-Draws the DelaunayTriangulation points and lines.
 *
 * delTri:		the DelaunayTriangulation structure
 * window:		the window
 * linesPoints:	the preallocated array that will all the lines points, be sure that it contains enough space for all the lines!
 * pointsDraw:	the structure used to draw the points
 * activePointsDraw:	the structure used to draw the active points
 * linesDraw:	the structure used to draw the lines
 * FAST:		if 1, will use fast drawing
 * sleep:		the sleep time in microseconds after calling the function
 * bounds:		the bounding box of the points
 *
 * returns:		a new DTDrawingParameters structure
 */
DTDrawingParameters* initDTDrawingParameters(DelaunayTriangulation *delTri,
											 bov_window_t *window,
						 					 GLfloat linesPoints[][2],
						 					 bov_points_t *pointsDraw,
											 bov_points_t *activePointsDraw,
											 bov_points_t *linesDraw,
											 GLfloat bounds[][2],
						 					 int FAST, int sleep) {

	DTDrawingParameters *DTDparams = malloc(sizeof(DTDrawingParameters));
	DTDparams->delTri = delTri;
	DTDparams->window = window;
	DTDparams->linesPoints = linesPoints;
	DTDparams->pointsDraw = pointsDraw;
	DTDparams->activePointsDraw = activePointsDraw;
	DTDparams->linesDraw = linesDraw;
	DTDparams->FAST = FAST;
	DTDparams->sleep = sleep;
	DTDparams->draw_circle = 1;

	GLfloat dy = bounds[1][1] - bounds[0][1];

	DTDparams->y_min = bounds[0][1] - 0.25 * dy;
	DTDparams->y_max = bounds[1][1] + 0.25 * dy;

	DTDparams->n_divides = 0;
	DTDparams->n_divides_max = delTri->n_points >> 1;

	DTDparams->divideLinesPoints = malloc(sizeof(DTDparams->divideLinesPoints[0]) * DTDparams->n_divides_max * 2);
	DTDparams->divideLinesDraw = bov_points_new(DTDparams->divideLinesPoints, DTDparams->n_divides_max * 2, GL_STATIC_DRAW);
	DTDparams->divideLinesMask = calloc(DTDparams->n_divides_max, sizeof(char));

	bov_points_set_color(DTDparams->divideLinesDraw, DIVIDE_LINES_COLOR);
	bov_points_set_width(DTDparams->divideLinesDraw, DIVIDE_LINES_WIDTH);

	GLfloat dtheta = 2 * M_PI / (GLfloat) N_POINTS;
	GLfloat theta = 0.0;

	for (GLsizei i = 0; i < N_POINTS; i++) {
		DTDparams->cos_[i] = cos(theta);
		DTDparams->sin_[i] = sin(theta);

		theta += dtheta;
	}

	DTDparams->searchPointsDraw = bov_points_new(DTDparams->delTri->points, DTDparams->delTri->n_points, GL_STATIC_DRAW);
	DTDparams->circlePointsDraw = bov_points_new(DTDparams->circ, N_POINTS, GL_STATIC_DRAW);
	DTDparams->searchPointsOrder = bov_order_new(DTDparams->searchPoints, 3, GL_STATIC_DRAW);

	bov_points_set_color(DTDparams->searchPointsDraw, SEARCHED_POINTS_COLOR);
	bov_points_set_width(DTDparams->searchPointsDraw, SEARCHED_POINTS_WIDTH);
	bov_points_set_color(DTDparams->circlePointsDraw, CIRCLE_LINES_COLOR);
	bov_points_set_width(DTDparams->circlePointsDraw, CIRCLE_LINES_WIDTH);

	return DTDparams;
}

/*
 * Sets the new triplet of points to be the triangle from which the
 * cicurmcicle will be computed.
 *
 * DTDparams:	the DTDrawingParameters structure
 * i_a,b,c:		the point indices
 */
void setSearchPoints(DTDrawingParameters *DTDparams,
					 GLsizei i_a, GLsizei i_b ,GLsizei i_c) {

	DTDparams->searchPoints[0] = i_a;
	DTDparams->searchPoints[1] = i_b;
	DTDparams->searchPoints[2] = i_c;

	bov_order_update(DTDparams->searchPointsOrder, DTDparams->searchPoints, 3);

	GLfloat radius, center[2];
	radius = circleCenter(DTDparams->delTri, i_a, i_b, i_c, center);

	for (GLsizei i = 0; i < N_POINTS; i++) {
		DTDparams->circ[i][0] = center[0] + radius * DTDparams->cos_[i];
		DTDparams->circ[i][1] = center[1] + radius * DTDparams->sin_[i];
	}

	bov_points_update(DTDparams->circlePointsDraw, DTDparams->circ, N_POINTS);

}

/*
 * Adds a dividing line at pivot point.
 *
 * DTDparams:	the DTDrawingParameters structure
 * pivot:		the pivot point (start of right half)
 */
GLsizei addDivideLine(DTDrawingParameters *DTDparams, GLsizei pivot) {
	if (DTDparams->n_divides == DTDparams->n_divides_max) {
		printf("ERROR: Not enough space was allocated for divide lines\n");
		exit(EXIT_FAILURE);
	}

	GLsizei divide_index = DTDparams->n_divides;

	DTDparams->divideLinesMask[divide_index] = 1;

	GLfloat x_mean = 0.5 * (DTDparams->delTri->points[pivot - 1][0] + DTDparams->delTri->points[pivot][0]);

	DTDparams->divideLinesPoints[2 * divide_index    ][0] = x_mean;
	DTDparams->divideLinesPoints[2 * divide_index    ][1] = DTDparams->y_min;
	DTDparams->divideLinesPoints[2 * divide_index + 1][0] = x_mean;
	DTDparams->divideLinesPoints[2 * divide_index + 1][1] = DTDparams->y_max;

	DTDparams->n_divides++;

	bov_points_update(DTDparams->divideLinesDraw, DTDparams->divideLinesPoints, 2 * DTDparams->n_divides);

	return divide_index;
}

void deleteDivideLine(DTDrawingParameters *DTDparams, GLsizei index) {
	DTDparams->divideLinesMask[index] = 0;
}

/*
 * Frees a DTDrawingParameters structure and everything it has allocated.
 *
 * DTDparams:	the DTDrawingParameters structure
 */
void freeDTDrawingParameters(DTDrawingParameters *DTDparams) {
	if (DTDparams != NULL) {
		if (DTDparams->divideLinesPoints != NULL) free(DTDparams->divideLinesPoints);
		if (DTDparams->divideLinesDraw != NULL) bov_points_delete(DTDparams->divideLinesDraw);
		if (DTDparams->divideLinesMask != NULL) free(DTDparams->divideLinesMask);
		if (DTDparams->searchPointsDraw != NULL) bov_points_delete(DTDparams->searchPointsDraw);
		if (DTDparams->circlePointsDraw != NULL) bov_points_delete(DTDparams->circlePointsDraw);
		if (DTDparams->searchPointsOrder != NULL) bov_order_delete(DTDparams->searchPointsOrder);
		free(DTDparams);
	}
}

/*
 * Re-Draws the DelaunayTriangulation points and lines.
 *
 * DTDparams:	the DTDrawingParameters structure
 * start:		the start index of the slice
 * end:			the (excluded) end index of the slice
 */
void reDrawTriangulation(DTDrawingParameters *DTDparams,
					 	 int start, int end) {

	if (bov_window_should_close(DTDparams->window)) return;

	// Get new lines
	GLsizei n_lines = getDelaunayTriangulationNumberOfLines(DTDparams->delTri);


	getDelaunayTriangulationLines(DTDparams->delTri, DTDparams->linesPoints, n_lines);

	// Update lines
	bov_points_update(DTDparams->linesDraw, DTDparams->linesPoints, 2 * n_lines);

	if (DTDparams->FAST) {
		bov_fast_lines_draw(DTDparams->window, DTDparams->linesDraw, 0, BOV_TILL_END);
		bov_fast_points_draw(DTDparams->window, DTDparams->pointsDraw, 0, BOV_TILL_END);
		bov_fast_points_draw(DTDparams->window, DTDparams->activePointsDraw, start, end);

		for (GLsizei i = 0; i < DTDparams->n_divides; i++) {
			if (DTDparams->divideLinesMask[i]) {
				bov_fast_lines_draw(DTDparams->window, DTDparams->divideLinesDraw, 2 * i, 2 * i + 2);
			}
		}

		if (DTDparams->draw_circle) {
				bov_fast_line_loop_draw(DTDparams->window, DTDparams->circlePointsDraw, 0, BOV_TILL_END);
				bov_fast_points_draw_with_order(DTDparams->window, DTDparams->searchPointsDraw, DTDparams->searchPointsOrder, 0, BOV_TILL_END);
		}
	}
	else {
		bov_lines_draw(DTDparams->window, DTDparams->linesDraw, 0, BOV_TILL_END);
		bov_points_draw(DTDparams->window, DTDparams->pointsDraw, 0, BOV_TILL_END);
		bov_points_draw(DTDparams->window, DTDparams->activePointsDraw, start, end);

		for (GLsizei i = 0; i < DTDparams->n_divides; i++) {
			if (DTDparams->divideLinesMask[i]) {
				bov_lines_draw(DTDparams->window, DTDparams->divideLinesDraw, 2 * i, 2 * i + 2);
			}
		}

		if (DTDparams->draw_circle) {
				bov_line_loop_draw(DTDparams->window, DTDparams->circlePointsDraw, 0, BOV_TILL_END);
				bov_points_draw_with_order(DTDparams->window, DTDparams->searchPointsDraw, DTDparams->searchPointsOrder, 0, BOV_TILL_END);
		}
	}

	bov_window_update(DTDparams->window);
	usleep(DTDparams->sleep);
}

/*
 * Triangulates a set of points using the DelaunayTriangulation.
 * This function should be the main function which will all the other sub-functions.
 *
 * DTDparams:	the DTDrawingParameters structure
 */
void triangulateDTIllustrated(DTDrawingParameters *DTDparams) {
	if (DTDparams->delTri->success) {
	return;
	}
	if (DTDparams->delTri->n_points < 2) {
		return;
	}

	// Sort points by x coordinates then by y coordinate.
	qsort(DTDparams->delTri->points, DTDparams->delTri->n_points, 2 * sizeof(GLfloat), compare_points);

	DTDparams->draw_circle = 0;
	reDrawTriangulation(DTDparams, 0, 0);
	DTDparams->draw_circle = 1;

	/// Starts the triangulation using a divide and conquer approach.
	Edge *l, *r;
	triangulateIllustrated(DTDparams->delTri, 0, DTDparams->delTri->n_points, &l, &r, DTDparams);
	DTDparams->delTri->success = 1;
}

/*
 * Recursive function used by the triangulateDT function in order to solve
 * the triangulation problem using a divide and conquer approach.
 *
 * delTri: 		the DelaunayTriangulation structure
 * start:		the start index of the slice
 * end:			the (excluded) end index of the slice
 * el:			an Edge structure pointer for the left edge
 * er:			an Edge structure pointer for the right edge
 * DTDparams:	the DTDrawingParameters structure
 */
void triangulateIllustrated(DelaunayTriangulation *delTri, GLsizei start, GLsizei end, Edge **el, Edge **er,
						    DTDrawingParameters *DTDparams) {
	GLsizei n = end - start;
	if (n == 2) {
		// Creates an edge connecting the two points (start), (start + 1)
		Edge *e = addEdge(delTri, start, start + 1);
		*el = e;
		*er = e->sym;
		DTDparams->draw_circle = 0;
		reDrawTriangulation(DTDparams, start, end);
		DTDparams->draw_circle = 1;
		return;
	}
	else if (n == 3) {
		Edge *a, *b, *c;

		// Creates two edges
		// - a, connecting (start), 	(start + 1)
		// - b, connecting (start + 1), (start + 2)
		a = addEdge(delTri, start, 		start + 1);
		b = addEdge(delTri, start + 1, 	start + 2);
		spliceEdges(delTri, a->sym, b);

		int cmp = pointCompareEdge(delTri, start + 2, a);

		setSearchPoints(DTDparams, a->orig, a->dest, b->dest);

		// Now will close the triangle formed by the three points
		if (cmp == 1) {
			c = connectEdges(delTri, b, a);
			*el = a;
			*er = b->sym;
			reDrawTriangulation(DTDparams, start, end);
			return;
		}
		else if (cmp == -1) {
			c = connectEdges(delTri, b, a);
			*el = c->sym;
			*er = c;
			reDrawTriangulation(DTDparams, start, end);
			return;
		}
		else {
			*el = a;
			*er = b->sym;
			reDrawTriangulation(DTDparams, start, end);
			return;
		}
	}
	else {
		// Recusively calls this function on half the points
		GLsizei m = (n + 1) / 2;
		Edge *ldo, *ldi, *rdi, *rdo;

		GLsizei divide_index = addDivideLine(DTDparams, start + m);
		DTDparams->draw_circle = 0;
		reDrawTriangulation(DTDparams, start, end);
		DTDparams->draw_circle = 1;

		triangulateIllustrated(delTri, start, 		start + m, 	&ldo, &ldi, DTDparams);
		triangulateIllustrated(delTri, start + m, 	end, 		&rdi, &rdo, DTDparams);
		deleteDivideLine(DTDparams, divide_index);

		// Computes the upper common tangent of left and right edges
		while (1) {
			setSearchPoints(DTDparams, rdi->orig, rdi->dest, ldi->dest);
			if 		(pointCompareEdge(delTri, rdi->orig, ldi) ==  1) {
				ldi = ldi->sym->onext;
				reDrawTriangulation(DTDparams, start, end);
			}
			else if (pointCompareEdge(delTri, ldi->orig, rdi) == -1) {
				rdi = rdi->sym->oprev;
				reDrawTriangulation(DTDparams, start, end);
			}
			else {
				break;
			}
		}

		Edge *base;
		DTDparams->draw_circle = 0;

		// Creates an edge between rdi.orig and ldi.orig
		base = connectEdges(delTri, ldi->sym, rdi);
		reDrawTriangulation(DTDparams, start, end);

		// Ajdusts ldo and rdo
		if (ldi->orig == ldo->orig) {
			ldo = base;
			reDrawTriangulation(DTDparams, start, end);
		}
		if (rdi->orig == rdo->orig) {
			rdo = base->sym;
			reDrawTriangulation(DTDparams, start, end);
		}
		DTDparams->draw_circle = 1;

		Edge *lcand, *rcand, *tmp;
		int v_rcand, v_lcand;

		// We merge both parts
		while (1) {
			// Locates the first right and left points to be encountered
			// by the diving bubble

			rcand = base->sym->onext;
			lcand = base->oprev;

			v_rcand = (pointCompareEdge(delTri, rcand->dest, base) == 1);
			v_lcand = (pointCompareEdge(delTri, lcand->dest, base) == 1);

			if (!(v_rcand || v_lcand)) {
				// Merge is done
				break;
			}
			// Deletes right edges that fail the circle test
			if (v_rcand) {
				while ((pointCompareEdge(delTri, rcand->onext->dest, base) == 1) &&
					   (pointInCircle(delTri, rcand->onext->dest, base->dest, base->orig, rcand->dest))
				   )
					   {
						   tmp = rcand->onext;
						   deleteEdge(delTri, rcand);
						   rcand = tmp;
						   setSearchPoints(DTDparams, base->dest, base->orig, rcand->dest);
						   reDrawTriangulation(DTDparams, start, end);
					   }
			}
			// Deletes left edges that fail the circle test
			if (v_lcand) {
				while ((pointCompareEdge(delTri, lcand->oprev->dest, base) == 1) &&
					   (pointInCircle(delTri, lcand->oprev->dest, base->dest, base->orig, lcand->dest))
				   )
					   {
						   tmp = lcand->oprev;
						   deleteEdge(delTri, lcand);
						   lcand = tmp;
						   setSearchPoints(DTDparams, base->dest, base->orig, lcand->dest);
						   reDrawTriangulation(DTDparams, start, end);
					   }
			}

			if ((!v_rcand) ||
				(v_rcand && pointInCircle(delTri, lcand->dest, rcand->dest, rcand->orig, lcand->orig))
			) {

				tmp = connectEdges(delTri, lcand, base->sym);
				base = tmp;
				setSearchPoints(DTDparams, rcand->dest, rcand->orig, lcand->orig);
				reDrawTriangulation(DTDparams, start, end);
			}
			else {
				tmp = connectEdges(delTri, base->sym, rcand->sym);
				base = tmp;
				setSearchPoints(DTDparams, rcand->dest, rcand->orig, lcand->orig);
				reDrawTriangulation(DTDparams, start, end);
			}
		}
		DTDparams->draw_circle = 0;
		reDrawTriangulation(DTDparams, start, end);
		DTDparams->draw_circle = 1;
		*el = ldo;
		*er = rdo;
	}
}

////////////////////////////
// End: Drawing functions //
////////////////////////////
