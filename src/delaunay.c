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
    delTri->points = points;

	// (Half) Edges
	delTri->n_edges_max = (GLsizei) ceil((log(n) * 3 * n));
	delTri->n_edges = 0;
	delTri->n_edges_discarded = 0;
	delTri->edges = malloc(sizeof(Edge) * delTri->n_edges_max);

	if (delTri->edges == NULL) {
		printf("Could't allocate memory for edges\n");
		free(delTri);
		return NULL;
	}

	delTri->success = 0;

	return delTri;
}

/*
 * Frees a DelaunayTriangulation structure with all its content that it has
 * allocated.
 *
 * delTri:		the DelaunayTriangulation structure
 */
void freeDelaunayTriangulation(DelaunayTriangulation *delTri) {
	if (delTri != NULL) {
		if (delTri->edges != NULL) free(delTri->edges);
		printf("ICI\n");
		free(delTri);
		printf("FREE DONE\n");
	}
}

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

GLsizei getDelaunayTriangulationNumberOfLines(DelaunayTriangulation *delTri) {
	return (delTri->n_edges - delTri->n_edges_discarded) / 2;
}

void getDelaunayTriangulationLines(DelaunayTriangulation *delTri,
								   GLfloat lines[][2],
							   	   GLsizei n_lines) {

	printf("Generating triangulation lines\n");

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
	printf("Generated triangulation lines\n");
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
		/*
		delTri->n_edges_max *= 2;
		Edge *edges = realloc(delTri->edges, sizeof(Edge) * delTri->n_edges_max);
		if (!edges) {
			printf("Error while realloacting the edge: trying to double the size to %d\n", delTri->n_edges_max);
		}
		else {
			delTri->edges = edges;
		}*/
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

void drawDelaunayTriangulation(DelaunayTriangulation *delTri, bov_window_t *window) {
#if 0
	GLfloat coord[][2] = {
		{-1.0,  0.0},
		{-0.8, -0.6},
		{-0.7,  0.6},
		{-0.5,  0.0},
		{-0.2, -0.4},
		{ 0.0,  0.8},
		{ 0.3,  0.0},
		{ 0.5, -0.6},
		{ 0.7,  0.6},
		{ 0.0, -0.9},
	};
	GLsizei nPoints = 10;
	bov_points_t *coordDraw = bov_points_new(coord, nPoints, GL_STATIC_DRAW);
	bov_points_set_color(coordDraw, (GLfloat[4]) {0.0, 0.0, 0.0, 1.0});
	bov_points_set_outline_color(coordDraw, (GLfloat[4]) {0.3, 0.12, 0.0, 0.25});

	while(!bov_window_should_close(window)){
		bov_points_set_width(coordDraw, 0.003);
		bov_points_set_outline_width(coordDraw, 0.002);


		// points_set_width(coordDraw, 0.003);
		bov_points_set_outline_width(coordDraw, -1.);
		printf("LA\n");
		bov_points_draw(window, coordDraw, 0, nPoints);
		printf("ICI je suis\n");

		bov_window_update(window);
	}

	bov_points_delete(coordDraw);
#else
	// Shorten variables
	GLfloat (*points)[2] = delTri->points;
	GLsizei n_points = delTri->n_points;

	int success = delTri->success;
	//

    bov_points_t *pointsDraw = bov_points_new(points, n_points, GL_STATIC_DRAW);
	bov_points_set_color(pointsDraw, (GLfloat[4]) {0.0, 0.0, 0.0, 1.0});
	bov_points_set_outline_color(pointsDraw, (GLfloat[4]) {0.3, 0.12, 0.0, 0.25});

	bov_points_set_width(pointsDraw, 0.01);
	bov_points_set_outline_width(pointsDraw, 0.002);

	// points_set_width(coordDraw, 0.003);
	bov_points_set_outline_width(pointsDraw, -1.);
	bov_points_t *linesDraw = NULL;
	GLfloat (*linesPoints)[2];
	GLsizei n_lines;

	/*
	for(int i = 0; i < n_points; i++) {
		printf("(%.4f, %.4f)\n", points[i][0], points[i][1]);
	}*/

	success = 1;

	if (success) {
		printf("SUCCESS\n");
		n_lines = getDelaunayTriangulationNumberOfLines(delTri);
		linesPoints = malloc(sizeof(linesPoints[0]) * 2 * n_lines);
		getDelaunayTriangulationLines(delTri, linesPoints, n_lines);

		for(GLsizei i =0; i<2*n_lines; i++) {
			//printf("lines[%d]=(%.4f, %.4f)\n", i, linesPoints[i][0], linesPoints[i][1]);
		}

		linesDraw = bov_points_new(linesPoints, 2 * n_lines, GL_STATIC_DRAW);
		bov_points_set_width(linesDraw, 0.005);
	}

	while(!bov_window_should_close(window)){
		if (success) {
			//bov_fa
			bov_fast_lines_draw(window, linesDraw, 0, BOV_TILL_END);
		}
		bov_fast_points_draw(window, pointsDraw, 0, n_points);
		//printf("UPDATE\n");
		//bov_window_screenshot(window, "test.ppm");
		bov_window_update(window);
	}

	bov_points_delete(pointsDraw);
	if (success) bov_points_delete(linesDraw);
#endif
}
