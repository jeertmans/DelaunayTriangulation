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
	GLsizei n_triangles_max = n % 3 > 0 ? n / 3 : (n / 3) + 1;
	delTri->n_triangles = 0;
	delTri->triangles = malloc(sizeof(delTri->triangles[0]) * n_triangles_max);

	return delTri;
}

void freeDelaunayTriangulation(DelaunayTriangulation *delTri) {
	free(delTri->triangles);
	free(delTri);
}

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
