#include "delaunay.h"

// Inspired from: https://github.com/alexbaryzhikov/triangulation

//////////////////////////////////////////////////
// Begin: DelaunayTriangulation structure utils //
//////////////////////////////////////////////////

/*
 * Allocates and returns a DelaunayTriangulation structure from a set of n points.
 *
 * points: 		the n x 2 array of points (x, y)
 * n:			the number of points
 *
 * returns:		a new DelaunayTriangulation structure
 */
DelaunayTriangulation* initDelaunayTriangulation(GLfloat points[][2], GLsizei n) {
    DelaunayTriangulation *delTri = malloc(sizeof(DelaunayTriangulation));

    // Points
    delTri->n_points = n;
    delTri->points = malloc(sizeof(delTri->points[0]) * n);
	printf("points: %p\n", delTri->points);

	for (GLsizei i = 0; i < n; i++) {
		delTri->points[i][0] = points[i][0];
		delTri->points[i][1] = points[i][1];
	}

	delTri->edges = NULL;
	resetDelaunayTriangulation(delTri);

	return delTri;
}

void resetDelaunayTriangulation(DelaunayTriangulation *delTri) {
	if (delTri->edges != NULL) {
		free(delTri->edges);
	}

	// (Half) Edges
	delTri->n_edges_max = (GLsizei) ceil((log(delTri->n_points) * 3 * delTri->n_points));
	delTri->n_edges = 0;
	delTri->n_edges_discarded = 0;
	delTri->edges = malloc(sizeof(Edge) * delTri->n_edges_max);

	if ((delTri->edges == NULL) && (delTri->n_points > 1)) {
		printf("Could't allocate memory for edges\n");
		free(delTri);
		return;
	}

	printf("RESET done\n");

	delTri->success = 0;
}

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

void updatePointAtIndex(DelaunayTriangulation *delTri, GLsizei i_p, GLfloat point[2]) {
	delTri->points[i_p][0] = point[0];
	delTri->points[i_p][1] = point[1];
	resetDelaunayTriangulation(delTri);
}

int addPoint(DelaunayTriangulation *delTri, GLfloat point[2]) {
	if (getDistanceToClosestPoint(delTri, point) <= 1e-10) {
		return 0;
	}
	printf("ADD\n");

	GLsizei idx = delTri->n_points;
	delTri->n_points += 1;

	GLfloat (*points)[2];
	if (delTri->points == NULL) {
		printf("NO POINT\n");
		delTri->points = malloc(sizeof(delTri->points[0]) * delTri->n_points);
	}
	points = realloc(delTri->points, sizeof(delTri->points[0]) * delTri->n_points);
	// Should assert(points);
	delTri->points = points;
	delTri->points[idx][0] = point[0];
	delTri->points[idx][1] = point[1];

	printf("DONE adding\n");
	resetDelaunayTriangulation(delTri);
	return 1;
}

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

	// Main edge
	Edge *e = &(delTri->edges[delTri->n_edges]);
	e->idx = delTri->n_edges;
	delTri->n_edges += 1;

	e->discarded = 0;
	e->orig = orig;
	e->dest = dest;

	// Symetrical edge
	Edge *s =&(delTri->edges[delTri->n_edges]);
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
	if (a == b) {
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
 * Computes the determinant of a 3 x 3 matrix.
 *
 * m:			the matrix
 *
 * returns:		the determinant
 */
GLfloat _det_3x3_(GLfloat m[3][3]) {
    // |A| = a(ei − fh) − b(di − fg) + c(dh − eg)
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

/*
 * Indicates wether a point is inside a circumcircle.
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

    GLfloat a1, a2, a3, b1, b2, b3, c1, c2, c3, det;

	a1 = a[0] - point[0]; a2 = a[1] - point[1];
	b1 = b[0] - point[0]; b2 = b[1] - point[1];
	c1 = c[0] - point[0]; c2 = c[1] - point[1];

	a3 = a1 * a1 + a2 * a2;
	b3 = b1 * b1 + b2 * b2;
	c3 = c1 * c1 + c2 * c2;

	det = a1*b2*c3 + a2*b3*c1 + a3*b1*c2 - (a3*b2*c1 + a1*b3*c2 + a2*b1*c3);
    return det <= 0;
}

int pointRightOfEdge(GLfloat point[2], GLfloat orig[2], GLfloat dest[2]) {
	GLfloat det;

	det = (orig[0] - point[0]) * (dest[1] - point[1]) - (orig[1] - point[1]) * (dest[0] - point[0]);

	return det > 0;
}

int pointLeftOfEdge(GLfloat point[2], GLfloat orig[2], GLfloat dest[2]) {
	GLfloat det;

	det = (orig[0] - point[0]) * (dest[1] - point[1]) - (orig[1] - point[1]) * (dest[0] - point[0]);

	return det < 0;
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
 *
 */
int pointCompareEdge(DelaunayTriangulation *delTri, GLsizei i_p, Edge *e) {
	//printf("Comparing edges\n");
	//describeEdge(e);
	//printf("OUI: %d, (%d, %d)\n", i_p, e->orig, e->dest);

	GLfloat *point, *orig, *dest, det;
	point = delTri->points[i_p];
	//printf("MEGUSTA\n");
	orig = delTri->points[e->orig];
	dest = delTri->points[e->dest];

	//printf("LOL\n");

	det = (orig[0] - point[0]) * (dest[1] - point[1]) - (orig[1] - point[1]) * (dest[0] - point[0]);

	//printf("COMP DONE\n");

	return (det>0) - (det<0);
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

	for(GLsizei i=0; i < delTri->n_points; i++) {
		//printf("Point = (%.4f, %.4f)\n", delTri->points[i][0], delTri->points[i][1]);
	}

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
		GLsizei m = (n + 1) / 2;
		Edge *ldo, *ldi, *rdi, *rdo;
		triangulate(delTri, start, 		start + m, 	&ldo, &ldi);
		triangulate(delTri, start + m, 	end, 		&rdi, &rdo);


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

		base = connectEdges(delTri, ldi->sym, rdi);

		if (ldi->orig == ldo->orig) {
			ldo = base;
		}
		if (rdi->orig == rdo->orig) {
			rdo = base->sym;
		}

		Edge *lcand, *rcand, *tmp;
		int v_rcand, v_lcand;

		while (1) {
			rcand = base->sym->onext;
			lcand = base->oprev;

			v_rcand = (pointCompareEdge(delTri, rcand->dest, base) == 1);
			v_lcand = (pointCompareEdge(delTri, lcand->dest, base) == 1);
			if (!(v_rcand || v_lcand)) {
				break;
			}
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

void getMousePosition(bov_window_t *window, GLfloat mouse_pos[2]) {
	mouse_pos[0] = (window->cursorPos[0] - window->param.translate[0]);
	mouse_pos[1] = (window->cursorPos[1] - window->param.translate[1]);
}

void drawDelaunayTriangulation(DelaunayTriangulation *delTri, bov_window_t *window) {

	GLfloat defaultPointWidth;
	GLfloat updatedPointWidth;

	defaultPointWidth = 0.01;
	updatedPointWidth = 0.04;

    bov_points_t *pointsDraw = bov_points_new(delTri->points, delTri->n_points, GL_STATIC_DRAW);
	bov_points_set_color(pointsDraw, (GLfloat[4]) {0.0, 0.0, 0.0, 1.0});
	bov_points_set_outline_color(pointsDraw, (GLfloat[4]) {0.3, 0.12, 0.0, 0.25});
	bov_points_set_width(pointsDraw, defaultPointWidth);
	//bov_points_set_outline_width(pointsDraw, 0.1);


	GLsizei n_lines;
	GLfloat (*linesPoints)[2] = NULL;
	bov_points_t *linesDraw = bov_points_new(linesPoints, 0, GL_STATIC_DRAW);
	bov_points_update(linesDraw, linesPoints, 2 * n_lines);
	bov_points_set_color(linesDraw, (GLfloat[4]) {0.0, 0.0, 0.0, 1.0});
	bov_points_set_width(linesDraw, 0.004);
	bov_points_set_outline_color(linesDraw, (GLfloat[4]) {0.3, 0.12, 0.0, 0.25});
	bov_points_set_outline_width(linesDraw, .002);

	/*
	for(int i = 0; i < n_points; i++) {
		printf("(%.4f, %.4f)\n", points[i][0], points[i][1]);
	}*/

	if (delTri->success) {
		n_lines = getDelaunayTriangulationNumberOfLines(delTri);
		linesPoints = malloc(sizeof(linesPoints[0]) * 2 * n_lines);
		getDelaunayTriangulationLines(delTri, linesPoints, n_lines);

		for(GLsizei i =0; i<2*n_lines; i++) {
			//printf("lines[%d]=(%.4f, %.4f)\n", i, linesPoints[i][0], linesPoints[i][1]);
		}
	}

	int FAST = (delTri->n_points > 100);
	int REQUIRE_UPDATE = 0;
	int KEY_A, KEY_D, KEY_S, KEY_F;
	int LAST_KEY_A, LAST_KEY_D, LAST_KEY_F;
	LAST_KEY_A = LAST_KEY_D = LAST_KEY_F = 0;

	int idx = -1;
	GLfloat mousePoint[][2] = {{0.0, 0.0}};


	bov_points_t *mouseDraw = bov_points_new(mousePoint, 1, GL_STATIC_DRAW);
	bov_points_set_color(mouseDraw, (GLfloat[4]) {0.0, 0.0, 0.0, 0.0});
	bov_points_set_outline_color(mouseDraw, (GLfloat[4]) {0.3, 0.12, 0.0, 0.25});
	bov_points_set_width(mouseDraw, defaultPointWidth);
	bov_points_set_outline_width(mouseDraw, -.1);

	while(!bov_window_should_close(window)){
		getMousePosition(window, mousePoint[0]);

		// printf("(x, y) = (%.4f, %.4f)\n", window->cursorPos[0], window->cursorPos[1]);
		KEY_A = glfwGetKey(window->self, GLFW_KEY_Q); // QWERTY layout
		KEY_D = glfwGetKey(window->self, GLFW_KEY_D);
		KEY_S = glfwGetKey(window->self, GLFW_KEY_S);
		KEY_F = glfwGetKey(window->self, GLFW_KEY_F);

		//printf("Mouse at (%.4f, %.4f)\n", mouse_pos[0], mouse_pos[1]);
		//printf("Param: res(%.4f, %4f), translate(%.4f, %4f), zoom(%.4f)\n", window->param.res[0], window->param.res[1], window->param.translate[0], window->param.translate[1], window->param.zoom);

		if (KEY_A) {
			if (!LAST_KEY_A) {
				REQUIRE_UPDATE = addPoint(delTri, mousePoint[0]);
				LAST_KEY_A = KEY_A;
			}
		}
		else {
			LAST_KEY_A = KEY_A;
		}
		if (KEY_D) {
			if (!LAST_KEY_D) {
				REQUIRE_UPDATE = deletePoint(delTri, mousePoint[0]);
				LAST_KEY_D = KEY_D;
			}
		}
		else {
			LAST_KEY_D = KEY_D;
		}
		if (KEY_S) {
			idx = getPointIndex(delTri, mousePoint[0]);
			updatePointAtIndex(delTri, idx, mousePoint[0]);
			REQUIRE_UPDATE = 1;
		}
		else {
			idx = -1;
		}
		if (KEY_F) {
			if (!LAST_KEY_F) {
				FAST = !FAST;
				LAST_KEY_F = KEY_F;
			}
		}
		else {
			LAST_KEY_F = KEY_F;
		}
		if (REQUIRE_UPDATE) {
			triangulateDT(delTri);
			if (delTri->success) {
				bov_points_update(pointsDraw, delTri->points, delTri->n_points);
				if (linesPoints != NULL) free(linesPoints);
				n_lines = getDelaunayTriangulationNumberOfLines(delTri);
				linesPoints = malloc(sizeof(linesPoints[0]) * 2 * n_lines);
				getDelaunayTriangulationLines(delTri, linesPoints, n_lines);
				bov_points_update(linesDraw, linesPoints, 2 * n_lines);
			}

			bov_points_update(mouseDraw, mousePoint, 1);
			bov_points_set_color(mouseDraw, (GLfloat[4]) {1.0, 0.0, 0.0, 1.0});
			bov_points_set_width(mouseDraw, updatedPointWidth);
			REQUIRE_UPDATE = 0;
		}
		else {
			bov_points_param_t param = bov_points_get_param(mouseDraw);
			param.fillColor[3] *= 0.95;
			param.width -= (param.width - defaultPointWidth) / 10;
			bov_points_set_param(mouseDraw, param);
		}
		//printf("A=%d, D=%d, S=%d\n", KEY_A, KEY_D, KEY_S);
		if (delTri->success) {
			if (FAST) {
				bov_fast_lines_draw(window, linesDraw, 0, BOV_TILL_END);
			}
			else {
				bov_lines_draw(window, linesDraw, 0, BOV_TILL_END);
			}
		}
		if (FAST) {
			bov_fast_points_draw(window, pointsDraw, 0, delTri->n_points);
			bov_fast_points_draw(window, mouseDraw, 0, 1);
		}
		else {
			bov_points_draw(window, pointsDraw, 0, delTri->n_points);
			bov_points_draw(window, mouseDraw, 0, 1);
		}
		//printf("UPDATE\n");
		//bov_window_screenshot(window, "test.ppm");
		bov_window_update(window);
	}

	bov_points_delete(pointsDraw);
	bov_points_delete(mouseDraw);

	if (delTri->success) {
		bov_points_delete(linesDraw);
		free(linesPoints);
	}
}
