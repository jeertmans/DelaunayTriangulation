#include "delaunay.h"

DelaunayTriangulation* initDelaunayTriangulation(GLfloat points[][2], GLsizei n) {
    DelaunayTriangulation *delTri = (DelaunayTriangulation *) malloc(sizeof(DelaunayTriangulation));
    delTri->n_points = n;
    delTri->points = points;
    delTri->n_centers = 0;
    delTri->success = 0;
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
