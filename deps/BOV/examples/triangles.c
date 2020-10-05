 /*************************************************************************
  * Triangles example program using BOV, a wrapper around OpenGL and
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


int main(int argc, char* argv[])
{
	bov_window_t* window = bov_window_new(1080,640, argv[0]);

	const GLubyte common_msg[] = "rendering points using";
	bov_text_t* common_label = bov_text_new(common_msg,
	                                        GL_STATIC_DRAW);
	bov_text_set_pos(common_label, (GLfloat[2]) {-1.0, 0.9});

	// make an array of message
	const GLubyte* label_msg[8] = {
		(GLubyte[]){"bov_points_draw()"},
		(GLubyte[]){"bov_triangles_draw()"},
		(GLubyte[]){"bov_triangle_strip_draw()"},
		(GLubyte[]){"bov_triangle_fan_draw()"},
		(GLubyte[]){"bov_fast_points_draw()"},
		(GLubyte[]){"bov_fast_triangles_draw()"},
		(GLubyte[]){"bov_fast_triangle_strip_draw()"},
		(GLubyte[]){"bov_fast_triangle_fan_draw()"},
	};

	void (*functions[8])(bov_window_t* window,
	                     const bov_points_t* pts,
	                     GLint start,
	                     GLsizei count) = {
		bov_points_draw,
		bov_triangles_draw,
		bov_triangle_strip_draw,
		bov_triangle_fan_draw,
		bov_fast_points_draw,
		bov_fast_triangles_draw,
		bov_fast_triangle_strip_draw,
		bov_fast_triangle_fan_draw,
	};

	const GLubyte* label_msg_with_order[8] = {
		(GLubyte[]){"bov_points_draw_with_order()"},
		(GLubyte[]){"bov_triangles_draw_with_order()"},
		(GLubyte[]){"bov_triangle_strip_draw_with_order()"},
		(GLubyte[]){"bov_triangle_fan_draw_with_order()"},
		(GLubyte[]){"bov_fast_points_draw_with_order()"},
		(GLubyte[]){"bov_fast_triangles_draw_with_order()"},
		(GLubyte[]){"bov_fast_triangle_strip_draw_with_order()"},
		(GLubyte[]){"bov_fast_triangle_fan_draw_with_order()"},
	};

	void (*functions_with_order[8])(bov_window_t* window,
                                    const bov_points_t* pts,
                                    const bov_order_t* order,
                                    GLint start,
                                    GLsizei count) = {
		bov_points_draw_with_order,
		bov_triangles_draw_with_order,
		bov_triangle_strip_draw_with_order,
		bov_triangle_fan_draw_with_order,
		bov_fast_points_draw_with_order,
		bov_fast_triangles_draw_with_order,
		bov_fast_triangle_strip_draw_with_order,
		bov_fast_triangle_fan_draw_with_order,
	};

	// with an array of corresponding labels
	bov_text_t* label[8];
	bov_text_t* label_with_order[8];
	for(int i=0; i<8; i++) {
		label[i] = bov_text_new(label_msg[i], GL_STATIC_DRAW);
		label_with_order[i] = bov_text_new(label_msg_with_order[i], GL_STATIC_DRAW);
		bov_text_set_pos(label[i], (GLfloat[2]) {-1.0 + 23 * 0.025, 0.9});
		bov_text_set_pos(label_with_order[i], (GLfloat[2]) {-1.0 + 23 * 0.025, 0.9});
	}

	GLfloat coord[][2] = {
		{0, 0},
		{1, 0},
		{0.6*0.8660254582502496, 0.6*0.49999990566243624},
		{0.5000001886751096, 0.8660252948527859},
		{0.6*3.2679489653813835e-07, 0.6*0.9999999999999466},
		{-0.49999962264970943, 0.8660256216476209},
		{-0.6*0.8660251314552299, 0.6*0.5000004716877294},
		{-0.9999999999997864, 6.535897930762419e-07},
		{-0.6*0.8660257850448994, -0.6*0.4999993396369297},
		{-0.5000007547002964, -0.8660249680575809},
		{-0.6*9.803846891701863e-07, -0.6*0.9999999999995194},
		{0.49999905662409616, -0.8660259484420856},
		{0.6*0.86602480465984, -0.6*0.5000010377128091},
		{1, 0},
	};

	bov_points_t* pointset = bov_points_new(coord, 14, GL_STATIC_DRAW);
	bov_points_set_color(pointset, (float[4]) {0.05, 0.1, 0.2, 0.6});
	bov_points_set_outline_width(pointset, 0.025);
	bov_points_set_width(pointset, 0.0);
	bov_points_set_outline_color(pointset, (GLfloat[4]) {0.3, 0.0, 0.0, 0.5});

	bov_order_t* order = bov_order_new((GLuint[14]) {0, 2, 4, 6, 8, 14, 12, 1, 3, 5, 7, 9, 11, 13},
	                                    10,
	                                    GL_STATIC_DRAW);

	unsigned long frameCount = 0;
	while(!bov_window_should_close(window)) {
		double wtime = bov_window_get_time(window);

		bov_text_draw(window, common_label);
		unsigned mode = (unsigned) wtime / 2 % 16;

		if(mode<8) {
			bov_text_draw(window, label[mode]);
			functions[mode](window, pointset, 0, BOV_TILL_END);
		}
		else {
			bov_text_draw(window, label_with_order[mode-8]);
			functions_with_order[mode-8](window, pointset, order, 0, BOV_TILL_END);
		}

		bov_window_update(window);

		frameCount++;
	}

	printf("Ended correctly - %.2f second, %lu frames, %.2f fps\n",
	       bov_window_get_time(window),
	       frameCount,
	       frameCount / bov_window_get_time(window));

	for(int i=0; i<8; i++) {
		bov_text_delete(label[i]);
		bov_text_delete(label_with_order[i]);
	}
	bov_text_delete(common_label);

	bov_points_delete(pointset);

	bov_window_delete(window);

	return EXIT_SUCCESS;
}
