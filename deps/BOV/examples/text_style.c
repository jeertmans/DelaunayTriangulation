 /*************************************************************************
  * text Style example program using BOV, a wrapper around OpenGL
  * and GLFW (www.glfw.org) to draw simple 2D graphics.
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
	bov_window_t* window = bov_window_new(0, 0, argv[0]);
	bov_window_set_color(window, (GLfloat[4]) {0.8, 0.8, 0.8, 1.0});


	bov_text_t* outline = bov_text_new(
		(GLubyte[]) {"varying outline width"},
		GL_STATIC_DRAW);

	// putting more than 1 on the alpha component for radiation effect
	bov_text_param_t parameters = {
		.outlineColor = {1.0, 0.0, 0.0, 2.0},
		.pos = {-1.0, 0.66},
		.fillColor = {0},// completely transparent
		.fontSize = 0.25,
		.boldness = 0.25,
		.outlineWidth = 0.5
	};
	bov_text_set_param(outline, parameters);


	bov_text_t* width = bov_text_new(
		(GLubyte[]) {"varying width"},
		GL_STATIC_DRAW);
	bov_text_set_param(width, parameters);
	bov_text_set_outline_width(width, -1.0);
	bov_text_set_color(width, (GLfloat[4]) {0.2, 0.2, 0.2, 1});
	bov_text_set_pos(width, (GLfloat[2]) {-1.0, 0.33});


	bov_text_t* shift = bov_text_new(
		(GLubyte[]) {"varying outline shift"},
		GL_STATIC_DRAW);
	bov_text_set_param(shift, parameters);
	bov_text_set_pos(shift, (GLfloat[2]) {-1.0, 0.0});

	bov_text_t* fontSize = bov_text_new(
		(GLubyte[]) {"varying size"},
		GL_STATIC_DRAW);
	bov_text_set_param(fontSize, parameters);
	bov_text_set_pos(fontSize, (GLfloat[2]) {-1.0, -0.33});



	bov_text_t* pixel = bov_text_new(
		(GLubyte[]) {"This text is unmoovable and unzoomable."
		             " Its position and its size"
		             " must be given in pixels"},
		GL_STATIC_DRAW);
	bov_text_set_space_type(pixel, PIXEL_SPACE);


	bov_text_t* unzoomable = bov_text_new(
		(GLubyte[]) {".you can't zoom on this point"},
		GL_STATIC_DRAW);
	GLfloat pixel64 = 2.0 / bov_window_get_yres(window) * 64.0; //~64 pixels height
	bov_text_set_pos(unzoomable, (GLfloat[2]) {-1.0, 1 - 1.1 * pixel64});
	bov_text_set_fontsize(unzoomable, pixel64);
	bov_text_set_space_type(unzoomable, UNZOOMABLE_SPACE);



	while(!bov_window_should_close(window)) {
		double wtime = bov_window_get_time(window);

		bov_text_set_outline_width(outline, 0.6 * sin(wtime) + 0.6);
		bov_text_draw(window, outline);

		bov_text_set_boldness(width, 0.5 * sin(wtime) - 0.1);
		bov_text_draw(window, width);

		bov_text_set_outline_shift(shift, (GLfloat[2]){
		                                sin(3 * wtime),
		                                cos(3 * wtime)});
		bov_text_draw(window, shift);

		bov_text_set_fontsize(fontSize, 0.1 * sin(wtime) + 0.1);
		bov_text_draw(window, fontSize);

		bov_text_draw(window, pixel);
		bov_text_draw(window, unzoomable);

		bov_window_update(window);
	}

	printf("Ended correctly\n");

	bov_text_delete(outline);
	bov_text_delete(width);
	bov_text_delete(shift);

	bov_window_delete(window);

	return EXIT_SUCCESS;
}
