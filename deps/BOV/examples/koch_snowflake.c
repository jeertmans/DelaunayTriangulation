#include "BOV.h"
#include <math.h>

#define TRANSITION_TIME 1.3


#define ORIGINAL_SEGWIDTH 0.33
#define SQRT3_6 0.288675135 // sqrt(3)/6

/* update the properties (width, outline width & scaling) of the pointset
 * given the proportion of the transition `s` time and the segwidth */
static void parameters_update(bov_points_t* pointset, GLfloat segWidth, GLfloat s)
{

	/* the width of a segment during a transition
	 * must go from segWidth to segWidth/3 when
	 * s goes from 0 to 1 */
	GLfloat w = segWidth - s * segWidth * 2 / 3;

	/* we also would like the line to stay exaclty between -1.5 and 1.5
	 * The line actually stays between +-[1.0-ORIGINAL_SEGWIDTH + w]
	 * so we just have to scale by the same amount..
	 *
	 */
	GLfloat scale = 1.5 / (1.0 - ORIGINAL_SEGWIDTH + w);
	bov_points_scale(pointset, (GLfloat[2]) {scale, scale});

	/* but the scaling should also scale w, which is not the case
	 * with the BOV library, only the coordinates of the points
	 * are scaled
	 // * thus we must scale the width ourselves */
	w *= scale;
	bov_points_set_width(pointset, w);
	bov_points_set_outline_width(pointset, 0.66 * w);
}


/* create p1, p2 and p3 from the line segment p0->p4,
 * with interpolation factor 's' between 0 and 1
 */
static void divide(GLfloat* p0,
                   GLfloat* p1,
                   GLfloat* p2,
                   GLfloat* p3,
                   GLfloat* p4,
                   GLfloat s)
{
	GLfloat d[2] = {p4[0] - p0[0], p4[1] - p0[1]};

	for(int i=0; i<2; i++) {
		p1[i] = p0[i] + s * 1.0 / 3.0 * d[i] + (1-s) * 0.25 * d[i];
		p2[i] = p0[i] + 0.5 * d[i];
		p3[i] = p0[i] + s * 2.0 / 3.0 * d[i] + (1-s) * 0.75 * d[i];
	}

	p2[0] -= s * SQRT3_6 * d[1];
	p2[1] += s * SQRT3_6 * d[0];
}


/* update the coordinates, with interpolation factor 's' between 0 and 1
 * the function divide will fill the coordinate of the points
 *
 *       coords[indices[4*i+2]],
 *       coords[indices[4*i+3]],
 *       coords[indices[4*i+4]],
 *
 *  given the coordinate of
 *       coords[indices[4*i+1]],
 *       coords[indices[4*i+5]],
 */
static void coords_update(GLfloat coords[][2], GLuint* indices, size_t nSegment,
                          GLfloat s)
{
	for(size_t i=0; i<nSegment; i++) {
		divide(coords[indices[4*i+1]],
		       coords[indices[4*i+2]],
		       coords[indices[4*i+3]],
		       coords[indices[4*i+4]],
		       coords[indices[4*i+5]],
		       s);
	}
}


/* this function simply compute the right indices given the number of segment*/
static void indices_update(GLuint* indices, size_t nSegment, size_t maxSegment)
{
	size_t subStep = maxSegment/nSegment;

	indices[0] = 0;
	for(size_t i=0; i<nSegment+1; i++) {
		indices[i+1] = i * subStep;
	}
	indices[nSegment+2] = maxSegment;
}


int main(int argc, char *argv[])
{
	bov_window_t* window = bov_window_new(1024, 640, argv[0]);
	bov_window_set_color(window, (GLfloat[4]) {0.3, 0.3, 0.3, 1.0});

	const int maxIters = 5;
	const size_t maxSegment = 1ULL << (2 * maxIters); // 4^maxIter with bits operations
	const size_t maxPoints = maxSegment + 1;

	// allocations
	GLfloat (*coords)[2] = malloc(2 * sizeof(GLfloat) * maxPoints);
	GLuint* indices = malloc(sizeof(GLuint) * (maxPoints + 2));
	bov_points_t* pointset = bov_points_new(NULL, maxPoints, GL_STATIC_DRAW); // GPU mirror of coords
	bov_order_t* order = bov_order_new(NULL, maxPoints + 2, GL_DYNAMIC_DRAW); // GPU mirror of indices

	// initialization
	size_t nSegment = 1;                // nPoints = nSegment+1
	GLfloat segWidth = ORIGINAL_SEGWIDTH; // the width of each segments

	coords[0][0] = -1.0 + segWidth;
	coords[0][1] = -0.1;
	coords[maxSegment][0] = 1.0 - segWidth;
	coords[maxSegment][1] = -0.1;

	// we set those parameters once and for all, other parameters
	// will be updated during the animation
	GLfloat fillColor[4] = {1, 1, 1, 1};
	GLfloat outColor[4] = {0.7, 0.5, 0.0, 2.0};
	bov_points_set_color(pointset, outColor);
	bov_points_set_outline_color(pointset, fillColor);

	// iterations
	for(int i=0; i<maxIters-1; i++) {
		indices_update(indices, 4 * nSegment, maxSegment);
		bov_order_update(order, indices, 4 * nSegment + 3);

		// animation (here we recompute the points each times)
		double tbegin = bov_window_get_time(window);
		double tnow = tbegin;
		while(tnow - tbegin < TRANSITION_TIME) {
			// interpolation factor
			GLfloat s = (tnow - tbegin)/TRANSITION_TIME;

			coords_update(coords, indices, nSegment, s);
			bov_points_update(pointset, coords, maxPoints);
			parameters_update(pointset, segWidth, s);

			bov_curve_draw_with_order(window, pointset, order, 0, BOV_TILL_END);
			bov_window_update(window);

			tnow = bov_window_get_time(window);

			if(bov_window_should_close(window))
				goto end_of_program;
		}

		nSegment *= 4;
		segWidth *= 1.0 / 3.0;
	}

	bov_text_t* end = bov_text_new((GLubyte[]){"Max. iteration level reached"}, GL_STATIC_DRAW);
	bov_text_set_param(end, (bov_text_param_t) {
	  .spaceType = PIXEL_SPACE,
	  .fontSize = 64,
	  .pos = {64, 64},
	  .fillColor = {0},          // transparent
	  .outlineColor = {1, 1, 1, 1}, // white
	  .boldness = 0.3,
	  .outlineWidth = 1.0
	});

	parameters_update(pointset, segWidth, 0);
	bov_points_update(pointset, coords, maxPoints);

	while(!bov_window_should_close(window)) {
		bov_curve_draw_with_order(window, pointset, order, 0, BOV_TILL_END);
		bov_text_draw(window, end);
		bov_window_update_and_wait_events(window);
	}

	bov_text_delete(end);

end_of_program:

	bov_order_delete(order);
	free(indices);
	bov_points_delete(pointset);
	free(coords);
	bov_window_delete(window);

	return EXIT_SUCCESS;
}
