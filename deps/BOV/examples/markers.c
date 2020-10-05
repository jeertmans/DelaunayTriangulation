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

// Maybe there will be more marker types available in the future.
// At the moment, markers value wraps around at 25
// We cannot have more than 100 markers without chaning the way the
// string is created (putting %7.3f and lengthen the string)
#define BOV_NMARKERS 25


int main(int argc, char* argv[])
{
	/* Actually, this is not an example at all, you shouldn't draw the
	 * same f**king point 250 time, each time with a different draw
	 * call.
	 *
	 * This is more of a tool. When you launch this program, you will
	 * be able to see which marker value correspond to which shape
	 * and choose the shape that fits your need accordingly.
	 *
	 * REMINDER: DO NOT MAKE THAT MANY DRAW CALL IN YOUR CODE !!!!
	 */

	bov_window_t* window = bov_window_new(0,0, argv[0]);
	bov_window_set_color(window, (GLfloat[4]) {1.0f, 0.8f, 0.5f, 1.0f});

	const GLfloat pointWidth = 1.0f/BOV_NMARKERS;

	bov_points_t* points = bov_points_new(
		(float[2][2]) {{0.0f, 0.0f}, {1.0f, 0.0f}},
		2,
		GL_STATIC_DRAW);
	bov_points_set_outline_color(points, (float[4]) {0.3f, 0.3f, 0.3f, 1.0f});
	bov_points_set_outline_width(points, pointWidth * 0.2f);
	bov_points_set_width(points, pointWidth);

	bov_text_t* marker_text;
	{
		/* first, we will generate all the text values */
		// we will have 6 charater per number (2digit, one dot, 3 digit)
		// then 2 space, for each marker.
		// then we have 10 \n at the end of each line
		char string[9*(BOV_NMARKERS*8 + 9)+1];
		int cur = 0;

		for(int j=0; j<9; j++) {
			for(int i=0; i<BOV_NMARKERS; i++) {
				sprintf(string+cur, " %6.3f ", i + j * 0.12493);
				cur += 8;
			}

			// replace the 2 trailing space with 4 '\n'
			sprintf(string+cur-1, "\n\n\n\n\n\n\n\n\n\n");
			cur += 9;
		}


		marker_text = bov_text_new((GLubyte*) string, GL_STATIC_DRAW);
		bov_text_set_fontsize(marker_text, pointWidth * 0.5f);
		bov_text_set_pos(marker_text, (GLfloat[2]) {-1.0f, 1.0f - pointWidth});
	}


	while(!bov_window_should_close(window)) {
		double wtime = bov_window_get_time(window);

		// we change the color over time
		bov_points_set_color(points, (GLfloat[4]) {
		                 sin(0.11 * wtime) * 0.5 + 0.5,
		                 sin(0.7 * wtime) * 0.5 + 0.4,
		                 sin(0.67 * wtime) * 0.5 + 0.6,
		                 1});
		bov_points_set_outline_width(points, pointWidth * 0.2f);
		bov_points_set_width(points, pointWidth);

		// we modify wtime to go only between 0 and 1
		double fract02 = modf(0.2 * wtime, &wtime);
		wtime = fabs(2 * fract02 - 1.0);

		for(int i=0; i<BOV_NMARKERS; i++) {

			GLfloat pos[2] = {pointWidth-1.0f + 2.0f * pointWidth * i,
			                 1.0f - 2.5f * pointWidth};

			for(int j=0; j<9; j++) {
				bov_points_set_pos(points, pos);
				bov_points_set_marker(points, i + j * 0.12493f);
				bov_points_draw(window, points, 0, 1);

				pos[1] -= 5.0f * pointWidth;
			}

			bov_points_set_pos(points, pos);
			bov_points_set_marker(points, i + wtime);
			bov_points_draw(window, points, 0, 1);
		}

		bov_text_draw(window, marker_text);
		bov_points_set_pos(points, (GLfloat[2]) {-0.5, 1.0f - 9 * 5.0f * pointWidth});
		bov_points_set_width(points, pointWidth * 0.5f);
		bov_points_set_outline_width(points, pointWidth * (0.5f * 0.2f));
		bov_lines_draw(window, points, 0, 2);

		bov_window_update(window);
	}

	bov_points_delete(points);
	bov_text_delete(marker_text);
	bov_window_delete(window);

	return EXIT_SUCCESS;
}
