#include "delaunay.h"

DelaunayTriangulation* initDelaunayTriangulation(GLfloat points[][2], GLsizei n) {
    DelaunayTriangulation *delTri = (DelaunayTriangulation *) malloc(sizeof(DelaunayTriangulation));

    // Points
    delTri->n_points = n;
    delTri->points = points;

	// Centers and radii
    delTri->n_centers = 0;
    delTri->success = 0;

	// Triangles

	return delTri;
}

void freeDelaunayTriangulation(DelaunayTriangulation *delTri) {

	free(delTri);
}

void drawDelaunayTriangulation(DelaunayTriangulation *delTri, bov_window_t *window) {
    bov_points_t *pointsDraw = bov_points_new(delTri->points, delTri->n_points, GL_STATIC_DRAW);
	bov_points_set_color(pointsDraw, (GLfloat[4]) {0.0, 0.0, 0.0, 1.0});
	bov_points_set_outline_color(pointsDraw, (GLfloat[4]) {0.3, 0.12, 0.0, 0.25});
	bov_points_t *linesDraw = NULL;

	if (delTri->success) {
		//linesDraw = bov_points_new(delTri->centers, delTri->)
	}

	while(!bov_window_should_close(window)){
		bov_points_set_width(pointsDraw, 0.003);
		bov_points_set_outline_width(pointsDraw, 0.002);
		if (delTri->success) {
			// bov_line_loop_draw(window, coordDraw, 0, nPoints);
		}
		// points_set_width(coordDraw, 0.003);
		bov_points_set_outline_width(pointsDraw, -1.);
		bov_points_draw(window, pointsDraw, 0, delTri->n_points);

		bov_window_update(window);
	}

	if (delTri->success) {
		bov_points_delete(linesDraw);
	}
	bov_points_delete(pointsDraw);
}

int pointInCircleFast(GLfloat point[2], GLfloat center[2], GLfloat radius) {
    GLfloat dx = point[0] - center[0];
    GLfloat dy = point[1] - center[1];

    return (dx * dx + dy * dy) <= radius;
}

GLfloat _det_3x3_(GLfloat m[3][3]) {
    // |A| = a(ei − fh) − b(di − fg) + c(dh − eg)
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

int pointInCircleRobust(GLfloat point[2], GLfloat triPoints[3][2]) {
    // https://www.cs.cmu.edu/~quake/robust.html

    GLfloat (*matrix)[3] = malloc(sizeof(GLfloat) * 9);

    for (int i = 0; i < 3; i++) {
        matrix[i][0] = triPoints[i][0] - point[0];
        matrix[i][1] = triPoints[i][1] - point[1];
        matrix[i][2] = (matrix[i][0] * matrix[i][0]) + (matrix[i][1] * matrix[i][1]);
    }

    return _det_3x3_(matrix) <= 0;
}
