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
    DelaunayTriangulation *delTri = (DelaunayTriangulation *) malloc(sizeof(DelaunayTriangulation *));

    // Points
    delTri->n_points = n;
    delTri->points = points;

	// Centers and radii
    delTri->n_centers = 0;
    delTri->success = 0;

	// Triangles
	GLsizei n_triangles_max = n % 3 > 0 ? n / 3 : (n / 3) + 1;
	delTri->n_triangles = 0;
	delTri->triangles = malloc(sizeof(delTri->triangles[0]) * n_triangles_max);

	// (Half) Edges
	GLsizei n_edges_max = 3 * n;
	delTri->n_edges = 0;
	delTri->edges = malloc(sizeof(Edge *) * n_edges_max);

	return delTri;
}

/*
 * Frees a DelaunayTriangulation structure with all its content that it has
 * allocated.
 *
 * delTri:		the DelaunayTriangulation structure
 */
void freeDelaunayTriangulation(DelaunayTriangulation *delTri) {
	free(delTri->triangles);
	free(delTri->edges);
	free(delTri);
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
 * e:			an Edge structure pointer to the main edge
 *
 * returns:		the index of the main edge in the triangulation
 */
GLsizei addEdge(DelaunayTriangulation *delTri, GLsizei orig, GLsizei dest,
				Edge *e) {
	// Main edge
	e = delTri->edges[delTri->n_edges];
	e->idx = delTri->n_edges;
	GLsizei i_e = delTri->n_edges;
	delTri->n_edges += 1;

	e->discarded = 0;
	e->orig = orig;
	e->dest = dest;

	// Symetrical edge
	Edge *s = delTri->edges[delTri->n_edges];
	s->idx = delTri->n_edges;
	delTri->n_edges += 1;

	e->discarded = 0;
	s->orig = dest;
	s->dest = orig;

	// Bind the two edges together
	e->sym = s;
	q->sym = e;

	e->onext = e;
	e->oprev = e;
	s->onext = s;
	s->oprev = s;

	return i_e;
}

/*
 * Combines two distinct edges.
 *
 * delTri: 		the DelaunayTriangulation structure
 * i_a: 		the index of the first edge
 * i_b: 		the index of the second edge
 */
void spliceEdges(DelaunayTriangulation *delTri, GLsizei i_a, GLsizei i_b) {
	if (i_a == i_b) {
		return;
	}
	Edge *a = delTri->edges[i_a];
	Edge *b = delTri->edges[i_a];

	Edge *tmp;
	a->onext->oprev = b;
	b->onext->oprev = a;

	tmp = a->next;
	a->onext = b->next;
	b->next = tmp;
}

/*
 * Creates a new edge connecting the destination of the first edge to the origin
 * of the second edge
 *
 * delTri: 		the DelaunayTriangulation structure
 * i_a: 		the index of the first edge
 * i_b: 		the index of the second edge
 * e:			an Edge structure pointer to the new edge
 *
 * returns:		the index of the new edge
 */
GLsizei connectEdges(DelaunayTriangulation *delTri, GLsizei i_a, GLsizei i_b,
					 Edge *e) {

	Edge *a = delTri->edges[i_a];

	GLsizei i_e = addEdge(delTri, a->dest, b->orig, e);
	splice(delTri, e->idx, a->sym->oprev);
	splice(delTri, e->sym, i_b)

	return i_e;
}

/*
 * Delete an edge from the DelaunayTriangulation by discarding the edge and its
 * symetrical edge.
 *
 * delTri: 		the DelaunayTriangulation structure
 * i_e: 		the index of the edge
 */
void deleteEdge(DelaunayTriangulation *delTri, GLsizei i_e) {
	Edge *e = delTri->edges[i_e];
	spliceEdges(delTri, i_e, e.oprev);
	spliceEdges(delTri, e->sym, e->sym.oprev);

	// Discard the edge from the data structure
	e->discarded = 1;
	delTri->edges[e->sym]->discarded = 1;
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


int pointInCircleFast(GLfloat point[2], GLfloat center[2], GLfloat radius) {
    GLfloat dx = point[0] - center[0];
    GLfloat dy = point[1] - center[1];

    return (dx * dx + dy * dy) <= radius;
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
 * point:		the point
 * a, b & c:	the points of the triangle
 *
 * returns:		1 if the point lies inside the circle, 0 otherwise
 */
int pointInCircleRobust(GLfloat point[2], GLfloat a[2], GLfloat b[2], GLfloat c[2]) {
    // https://www.cs.cmu.edu/~quake/robust.html

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

/////////////////////////
// End: Geometry utils //
/////////////////////////

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////
// Begin: Triangulation functions //
////////////////////////////////////

void triangulateDT(DelaunayTriangulation *delTri) {
	// Sort points by x coordinates then by y coordinate.
	qsort(delTri->points, delTri->n_points, 2 * sizeof(GLfloat), compare_points);

	/// Starts the triangulation using a divide and conquer approach.
	triangulate(delTri, 0, delTri->n_points, (Edge *) NULL, (Edge *) NULL);
}

/*
 *
 */
void triangulate(DelaunayTriangulation *delTri, GLsizei start, GLsizei end, Edge *el, Edge *er) {
	GLsizei n = end - start;
	if (n == 2) {
		// a = make_edge(S[0], S[1])
		// return a, a.sym
		addEdge(delTri, start, start + 1 , el);
		er = el->sym;
		return;
	}
	else if (n == 3) {
		/*
		# Create edges a connecting p1 to p2 and b connecting p2 to p3.
        p1, p2, p3 = S[0], S[1], S[2]
        a = make_edge(p1, p2)
        b = make_edge(p2, p3)
        splice(a.sym, b)

        # Close the triangle.
        if right_of(p3, a):
            connect(b, a)
            return a, b.sym
        elif left_of(p3, a):
            c = connect(b, a)
            return c.sym, c
        else:  # the three points are collinear
            return a, b.sym
		*/
        Edge *a, *b, *c;
		GLsizei i_a, i_b;

		i_a = addEdge(delTri, start, start + 1, a);
		i_b = addEdge(delTri, start + 1, start + 2, b);
		spliceEdges(delTri, a->sym, i_b);

		GLfloat point[2], orig[2], dest[2];

		point = delTri->points[start + 2];
		orig = delTri->points[a->orig];
		dest = delTri->points[a->dest];

		if (pointRightOfEdge(point, orig, dest)) {
			connectEdges(delTri, i_b, i_a, (Edge *) NULL);
			el = a;
			er = delTri->edges[b->sym];
			return;
		}
		else if (pointLeftOfEdge(point, orig, dest)) {
			connectEdges(delTri, i_b, i_a, c);
			el = delTri->edges[c->sym];
			er = c;
			return;
		}
		else {
			el = a;
			er = delTri->edges[b->sym];
			return;
		}
	}
	else {
		GLsizei m = (n + 1) / 2;
		Edge *ldo, *ldi, *rdi, *rdo;
		triangulate(delTri, start, start + m, ldo, ldi);
		triangulate(delTri, start + m, end, ldo, ldi);

		while (1) {
			if ///
		}

		Edge *base;

		connectEdges(ldi->sym, rdi->idx);
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
	// Shorten variables
	GLfloat (*points)[2] = delTri->points;
	GLsizei n_points = delTri->n_points;

	GLsizei (*triangles)[3] = delTri->triangles;
	GLsizei n_triangles = delTri->n_triangles;

	int success = delTri->success;
	//

    bov_points_t *pointsDraw = bov_points_new(delTri->points, delTri->n_points, GL_STATIC_DRAW);
	bov_points_set_color(pointsDraw, (GLfloat[4]) {0.0, 0.0, 0.0, 1.0});
	bov_points_set_outline_color(pointsDraw, (GLfloat[4]) {0.3, 0.12, 0.0, 0.25});

	bov_points_set_width(pointsDraw, 0.03);
	bov_points_set_outline_width(pointsDraw, 0.002);

	// points_set_width(coordDraw, 0.003);
	bov_points_set_outline_width(pointsDraw, -1.);
	bov_points_t **trianglesDraw = NULL;

	/*
	for(int i = 0; i < delTri->n_points; i++) {
		printf("(%.4f, %.4f)\n", delTri->points[i][0], delTri->points[i][1]);
	}*/

	success = 1;

	if (success) {
		trianglesDraw = (bov_points_t **) malloc(sizeof(bov_points_t*) * n_triangles);

		for(GLsizei i = 0; i < n_triangles; i++) {
			GLsizei *ind = triangles[i];
			// Coordinates must be CCW !
			GLfloat coord[][2] = {
				{points[ind[0]][0], points[ind[0]][1]},
				{points[ind[1]][0], points[ind[1]][1]},
				{points[ind[2]][0], points[ind[2]][1]},
			};
			printf("Triangles[%d]: (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)\n",
					i, coord[0][0], coord[0][1],  coord[1][0], coord[1][1], coord[2][0], coord[2][1]);

			trianglesDraw[i] = bov_points_new(coord, 3, GL_STATIC_DRAW);
			bov_points_set_color(trianglesDraw[i], (float[4]) {0.05, 0.1, 0.2, 0.6});
			bov_points_set_outline_width(trianglesDraw[i], 0.025);
			bov_points_set_width(trianglesDraw[i], 0.0);
			bov_points_set_outline_color(trianglesDraw[i], (GLfloat[4]) {0.3, 0.0, 0.0, 0.5});
		}
	}

	while(!bov_window_should_close(window)){
		if (success) {
			for(GLsizei i = 0; i < n_triangles; i++) {
				bov_line_loop_draw(window, trianglesDraw[i], 0, BOV_TILL_END);
				bov_triangle_fan_draw(window, trianglesDraw[i], 0, BOV_TILL_END);
			}
		}

		bov_points_draw(window, pointsDraw, 0, n_points);
		//bov_window_screenshot(window, "test.ppm");
		bov_window_update(window);
	}

	if (success) {
		for(GLsizei i = 0; i < n_triangles; i++) {
			bov_points_delete(trianglesDraw[i]);
		}
		free(trianglesDraw);
	}
	bov_points_delete(pointsDraw);
}
