#include "BOV.h"
#include <math.h>

#define PI 3.14159265358979323846

void nice_colormap(float color[4], float a)
{
	color[0] = sin(0.33 * a) * 0.3 + 0.7;
	color[1] = sin(0.23 * a + 2.0) * 0.3 + 0.7;
	color[2] = sin(0.17 * a + 5.0) * 0.3 + 0.6;
	color[3] = 1.0; // solid
}

int main()
{
	bov_window_t* window = bov_window_new(800, 800, "Tutorial 1");
	bov_window_set_color(window, (GLfloat[]){0.5, 0.5, 0.5, 1.0});

	// hw is prefix for hello world :p
	bov_text_t* hw_obj = bov_text_new(
		(GLubyte[]){"Hello World !"},
		GL_STATIC_DRAW);

	float hw_color[4] = {1.0, 1.0, 1.0, 1.0};
	bov_text_set_fontsize(hw_obj, 0.2); // character is 0.2 tall (0.1 is the default)

	// a character is twice taller than it is large => width = 0.1
	// centering "Hello World !": 13/2=6.5 characters => 0.65
	bov_text_set_pos(hw_obj, (GLfloat[2]) {-0.65, 0.0});
	bov_text_set_boldness(hw_obj, 0.4); // bold
	bov_text_set_outline_width(hw_obj, 1.0); // big outline
	bov_text_set_outline_color(hw_obj, (float[4]) {0, 0, 0, 1}); // black


	while(!bov_window_should_close(window)) {
		nice_colormap(hw_color, bov_window_get_time(window));
		bov_text_set_color(hw_obj, hw_color);
		bov_text_draw(window, hw_obj);

		bov_window_update(window);
	}

	bov_text_delete(hw_obj);
	bov_window_delete(window);

	return EXIT_SUCCESS;
}