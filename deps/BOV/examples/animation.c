 /*************************************************************************
  * Animation example program using BOV, a wrapper around OpenGL and
  * GLFW (www.glfw.org) to draw simple 2D graphics.
  *------------------------------------------------------------------------
  * Copyright (c) 2019-2020 Célestin Marot <marotcelestin@gmail.com>
  *
  * This software is provided 'as-is', without any express or implied
  * warranty. In no event will the authors be held liable for any damages
  * arising from the use of this software.
  *
  * Permission is granted to anyone to use this software for any purpose,
  * including commercial applications, and to alter it and redistribute it
  * freely, subject to the following restrictions:
  *
  * 1. The origin of this software must not be misrepresented; you must not
  *    claim that you wrote the original software. If you use this software
  *    in a product, an acknowledgment in the product documentation would
  *    be appreciated but is not required.
  *
  * 2. Altered source versions must be plainly marked as such, and must not
  *    be misrepresented as being the original software.
  *
  * 3. This notice may not be removed or altered from any source
  *    distribution.
  *
  *************************************************************************/

#include "BOV.h"
#include <math.h>

const float transition_time = 1.0;

// transition from a point p to another point
void transition(bov_points_t* diag, float a[2], float b[2], float x)
{
	GLfloat v[2] = {b[0] - a[0], b[1] - a[1]};
	// scale everything
	bov_points_scale(diag, (GLfloat[2]){v[0]*x, v[1]*x});
	bov_points_set_pos(diag, a);
}


int main(int argc, char* argv[])
{
	bov_window_t* window = bov_window_new(0, 0, argv[0]);
	bov_window_enable_help(window);

	// a grey background
	bov_window_set_color(window, (GLfloat[4]) {0.3, 0.3, 0.3, 1});

	// we define a style for the lines
	bov_points_param_t lineParams = {
		.fillColor = {1.0, 0.6, 0.3, 1.0},
		.scale = {1.0, 1.0},
		.width = 0.03
	};

	GLfloat coord[10][2] = {
		{-0.2, -0.4},
		{-0.5,  0.0},
		{ 0.3,  0.0},
		{ 0.0, -0.9},
		{-0.8, -0.6},
		{-1.0,  0.0},
		{-0.7,  0.6},
		{ 0.0,  0.8},
		{ 0.7,  0.6},
		{ 0.5, -0.6}
	};

	bov_points_t* pointset = bov_points_new(coord, 10, GL_STATIC_DRAW);
	bov_points_t* diag = bov_points_new(
		(GLfloat[2][2]) {{0.0, 0.0}, {1.0, 1.0}},
		2,
		GL_STATIC_DRAW);
	bov_points_set_param(pointset, lineParams);
	bov_points_set_param(diag, lineParams);

	for (int i=0; i<10; i++) {
		double tbegin = bov_window_get_time(window);
		double tnow = tbegin;

		while(tnow - tbegin < transition_time) {
			if(bov_window_should_close(window))
				goto end_of_program; // break all the loop (only valid use of goto)

			bov_line_strip_draw(window, pointset, 0, i + 1);

			transition(diag, coord[i], coord[(i + 1) % 10], (tnow-tbegin)/transition_time);
			bov_lines_draw(window, diag, 0, 2);

			bov_window_update(window);
			tnow = bov_window_get_time(window);
		}
	}

	// we want to keep the window open with everything displayed...
	while(!bov_window_should_close(window)) {
		bov_line_loop_draw(window, pointset, 0, 10);
		bov_window_update_and_wait_events(window);
	}

end_of_program:

	printf("Ended correctly - %.2f second\n", window->wtime);

	bov_points_delete(pointset);
	bov_points_delete(diag);
	bov_window_delete(window);

	return EXIT_SUCCESS;
}
