 /*************************************************************************
  * BOV 0.1
  * A wrapper around OpenGL and GLFW (www.glfw.org) to draw simple 2D
  * graphics.
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

#ifndef BOV_H_
#define BOV_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

/* opaque structures */
typedef struct bov_window_struct bov_window_t;
typedef struct bov_order_struct bov_order_t;
typedef struct bov_text_struct bov_text_t;
typedef struct bov_points_struct bov_points_t;


/***
 *      _____         _      _  _                   _____  _____
 *     |  __ \       | |    | |(_)           /\    |  __ \|_   _|
 *     | |__) |_   _ | |__  | | _   ___     /  \   | |__) | | |
 *     |  ___/| | | || '_ \ | || | / __|   / /\ \  |  ___/  | |
 *     | |    | |_| || |_) || || || (__   / ____ \ | |     _| |_
 *     |_|     \__,_||_.__/ |_||_| \___| /_/    \_\|_|    |_____|
 *
 *
 */

/* a bov_space_type_t can be given to the functions text_set_space_type() and
 * points_set_space_type() */
typedef enum {
	USUAL_SPACE = 0,      // you can zoom and translate the space in which
	                      // the object is embeded

	UNZOOMABLE_SPACE = 1, // zooming will not change the size of the object

	PIXEL_SPACE = 2,      // unzoomable, unmovable and positon and width/height
	                      // must be given in pixel coordinates.
	                      // In addition, the origin of the screen is the bottom
	                      // left corner and not the center
} bov_space_type_t;


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Window
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
/* creating a window is probably the first thing a program must do.
 *
 * `width` is either:
 *     - the width of the window in pixel
 *     - 0 for full screen
 *     - a negative number for a maximized window
 *
 * `height` is either:
 *     - the height of the window in pixel
 *     - 0 for full screen
 *     - negative height for a fixed size window
 *
 * win_name is the title of the window
 * return a window object
 */
bov_window_t* bov_window_new(int width,
                             int height,
                             const char* win_name);

/* Once something was drawn to the framebuffer, the window must be updated to
 * display the content of the framebuffer on the screen. Updating the window
 * also handles the mouse and keyboard inputs, thus you must redraw your scene
 * frequently even if nothing changed in order to get smooth input handling.
 */
void bov_window_update(bov_window_t* window);

/* same as bov_window_update but wait for input events before clearing the screen
 */
void bov_window_update_and_wait_events(bov_window_t* window);

/* delete the window properly */
void bov_window_delete(bov_window_t* window);


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Text
 %%%%%%%%%%%%%%%%%%%%%%%%%*/

/* Create a text object from a string. string a null-terminated C string
 *
 * `usage` is either:
 *    - GL_STATIC_DRAW if you don't intend to change the content of the text
 *      object (the string of characters)
 *    - GL_DYNAMIC_DRAW if you intend to change it with the text_update function
 */
bov_text_t* bov_text_new(const GLubyte* string,
                         GLenum usage);

/* change the content of the text object */
bov_text_t* bov_text_update(bov_text_t* text,
                            const GLubyte* string);

/* draw a text object, using a text rasterizer (note: a text rasterizer can be
 * used to draw multiple text object)*/
void bov_text_draw(bov_window_t* window,
                   const bov_text_t* text);

/* delete text properly */
void bov_text_delete(bov_text_t* text);


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Points
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
#define BOV_TILL_END (0x7FFFFFFFUL)

/* Create a points object
 *
 * `coord` is either:
 *    - an array of coordinates, interleaved : x1, y1, x2, y2, ... xn, yn
 *         => n is the number of points and the maximum capacity
 *    - NULL, in which case a points object is created with a maximum capacity
 *      of n points
 *
 * `usage` is either:
 *    - GL_STATIC_DRAW if you don't intend to update the coordinates of the
 *      points regularly
 *    - GL_DYNAMIC_DRAW if you intend to change coordinates regularly with the
 *      points_update or points_partial_update function
 */
bov_points_t* bov_points_new(const GLfloat coords[][2],
                             GLsizei n,
                             GLenum usage);

/* change the content of the points object (the maximum capacity can be increased)*/
bov_points_t* bov_points_update(bov_points_t* points,
                                const GLfloat coords[][2],
                                GLsizei n);

/* change the content of the points object, but only from start to
 * start+count excluded.
 *
 * `newN` is the new number of points contained in the points object, or 0 to keep
 * the same size.
 * `newN` can only be lower than the maximum number of points that has been
 * contained in the given points object (the capacity cannot be increased)
 */
bov_points_t* bov_points_partial_update(bov_points_t* points,
                                        const GLfloat coords[][2],
                                        GLint start,
                                        GLsizei count,
                                        GLsizei newN);

/* draw points markers to the window
 *
 * `start` is the index of the first point to draw
 * `count` is the number of points to draw
 * if `count` is greater than the number of points, all the points are drawn.
 * the value TILL_END is guaranteed to be greater than the number of points.
 */
static inline void bov_points_draw(bov_window_t* window,
                                   const bov_points_t* pts,
                                   GLint start,
                                   GLsizei count);

// using the order defined by the order object
static inline void bov_points_draw_with_order(bov_window_t* window,
                                              const bov_points_t* pts,
                                              const bov_order_t* order,
                                              GLint start,
                                              GLsizei count);

/* same argument as above, but draw lines between pairs of points,
 * (p1->p2) (p3->p4) (p5->p6)...
 *
 * line are only drawn when both points are withing [start, start+count[,
 * so only the even part of count is taken into account
 */
static inline void bov_lines_draw(bov_window_t* window,
                                  const bov_points_t* pts,
                                  GLint start,
                                  GLsizei count);

static inline void bov_lines_draw_with_order(bov_window_t* window,
                                             const bov_points_t* pts,
                                             const bov_order_t* order,
                                             GLint start,
                                             GLsizei count);

/* same as bov_lines_draw(with_order)() using default shaders => 
 * the default shaders are pretty basic, they just display thin lines of the
 * selected color, without outline, without anti-aliasing, without taking the
 * width into account.
 */
static inline void bov_fast_lines_draw(bov_window_t* window,
                                      const bov_points_t* pts,
                                      GLint start,
                                      GLsizei count);

static inline void bov_fast_lines_draw_with_order(bov_window_t* window,
                                                  const bov_points_t* pts,
                                                  const bov_order_t* order,
                                                  GLint start,
                                                  GLsizei count);

/* draw lines that connect each points p1->p2->p3->p4->p5->p6...->pn */
static inline void bov_line_strip_draw(bov_window_t* window,
                                       const bov_points_t* pts,
                                       GLint start,
                                       GLsizei count);

static inline void bov_line_strip_draw_with_order(bov_window_t* window,
                                                  const bov_points_t* pts,
                                                  const bov_order_t* order,
                                                  GLint start,
                                                  GLsizei count);

/* same as bov_line_strip_draw(with_order) using default shaders.
 * see bov_fast_lines_draw() for what the default shaders is missing. */
static inline void bov_fast_line_strip_draw(bov_window_t* window,
                                            const bov_points_t* pts,
                                            GLint start,
                                            GLsizei count);

static inline void bov_fast_line_strip_draw_with_order(bov_window_t* window,
                                                       const bov_points_t* pts,
                                                       const bov_order_t* order,
                                                       GLint start,
                                                       GLsizei count);


/* draw lines that connect each points and end with the first
 * p1->p2->p3->p4->p5->p6...->pn->p1*/
static inline void bov_line_loop_draw(bov_window_t* window,
                                      const bov_points_t* pts,
                                      GLint start,
                                      GLsizei count);

static inline void bov_line_loop_draw_with_order(bov_window_t* window,
                                                 const bov_points_t* pts,
                                                 const bov_order_t* order,
                                                 GLint start,
                                                 GLsizei count);

/* same as bov_line_loop_draw(with_order) using default shaders.
 * see bov_fast_lines_draw() for what the default shaders is missing. */
static inline void bov_fast_line_loop_draw(bov_window_t* window,
                                           const bov_points_t* pts,
                                           GLint start,
                                           GLsizei count);

static inline void bov_fast_line_loop_draw_with_order(bov_window_t* window,
                                                      const bov_points_t* pts,
                                                      const bov_order_t* order,
                                                      GLint start,
                                                      GLsizei count);

/* draw a single line that connect each points p1->p2->p3->p4->p5->p6...->pn
 * The difference with line_strip_draw can really be seen with an outline or
 * transparency */
static inline void bov_curve_draw(bov_window_t* window,
                                  const bov_points_t* pts,
                                  GLint start,
                                  GLsizei count);

static inline void bov_curve_draw_with_order(bov_window_t* window,
                                             const bov_points_t* pts,
                                             const bov_order_t* order,
                                             GLint start,
                                             GLsizei count);

static inline void bov_triangles_draw(bov_window_t* window,
                                      const bov_points_t* pts,
                                      GLint start,
                                      GLsizei count);

static inline void bov_triangles_draw_with_order(bov_window_t* window,
                                             const bov_points_t* pts,
                                             const bov_order_t* order,
                                             GLint start,
                                             GLsizei count);

/* same as bov_triangles_draw(with_order) using default shaders.
 * the default shaders are pretty basic, they just display thin triangles of the
 * selected color, without outline, without anti-aliasing, without taking the
 * width into account.
 */
static inline void bov_fast_triangles_draw(bov_window_t* window,
                                           const bov_points_t* pts,
                                           GLint start,
                                           GLsizei count);

static inline void bov_fast_triangles_draw_with_order(bov_window_t* window,
                                                      const bov_points_t* pts,
                                                      const bov_order_t* order,
                                                      GLint start,
                                                      GLsizei count);


static inline void bov_triangle_strip_draw(bov_window_t* window,
                                           const bov_points_t* pts,
                                           GLint start,
                                           GLsizei count);

static inline void bov_triangle_strip_draw_with_order(bov_window_t* window,
                                                      const bov_points_t* pts,
                                                      const bov_order_t* order,
                                                      GLint start,
                                                      GLsizei count);

/* same as bov_triangle_strip_draw(with_order) using default shaders.
 * see bov_fast_triangles_draw() for what the default shaders is missing. */
static inline void bov_fast_triangle_strip_draw(bov_window_t* window,
                                                const bov_points_t* pts,
                                                GLint start,
                                                GLsizei count);

static inline void bov_fast_triangle_strip_draw_with_order(bov_window_t* window,
                                                           const bov_points_t* pts,
                                                           const bov_order_t* order,
                                                           GLint start,
                                                           GLsizei count);

static inline void bov_triangle_fan_draw(bov_window_t* window,
                                         const bov_points_t* pts,
                                         GLint start,
                                         GLsizei count);

static inline void bov_triangle_fan_draw_with_order(bov_window_t* window,
                                                    const bov_points_t* pts,
                                                    const bov_order_t* order,
                                                    GLint start,
                                                    GLsizei count);

/* same as bov_triangle_fan_draw(with_order) using default shaders.
 * see bov_fast_triangles_draw() for what the default shaders is missing. */
static inline void bov_fast_triangle_fan_draw(bov_window_t* window,
                                              const bov_points_t* pts,
                                              GLint start,
                                              GLsizei count);

static inline void bov_fast_triangle_fan_draw_with_order(bov_window_t* window,
                                                         const bov_points_t* pts,
                                                         const bov_order_t* order,
                                                         GLint start,
                                                         GLsizei count);


/* delete a points object      WORKS FOR BOTH ! */
void bov_points_delete(bov_points_t* points);


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Order
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
/* Create an order object, that enables to draw only certain points, or to draw
 * lines that connect different points.
 *
 * `elements` is either:
 *     - an array of indices (the indices of the point that we will draw) => n
 *       is the number of indices
 *     - NULL, in which case an order object is created with a maximum capacity
 *       of n indices
 *
 * `usage` is either:
 *    - GL_STATIC_DRAW if you don't intend to update the content of the order
 *      object regularly
 *    - GL_DYNAMIC_DRAW if you intend to change it regularly with the
 *      order_update or order_partial_update function
 */
bov_order_t* bov_order_new(const GLuint* elements,
                           GLsizei n,
                           GLenum usage);

/* Change the content of the order object. */
bov_order_t* bov_order_update(bov_order_t* order,
                              const GLuint* elements,
                              GLsizei n);

/* change the content of the points object, but only from start to
 * start+count excluded.
 *
 * `newN` is the new number of indices contained in the order object, or 0 to
 * keep the same size.
 * `newN` can only be lower than the maximum number of indices that has been
 * contained in the given order object (the capacity cannot be increased)
 */
bov_order_t* bov_order_partial_update(bov_order_t* order,
                                      const GLuint* elements,
                                      GLint start,
                                      GLsizei count,
                                      GLsizei newN);

/* delete an order object */
void bov_order_delete(bov_order_t* order);


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Window parameters
 %%%%%%%%%%%%%%%%%%%%%%%%%*/

/* get the time in second associated with the window. The time is updated via
 * the bov_window_update() and bov_window_update_and_wait_events() functions.
 * The time is stopped if the user press the space bar. */
static inline double bov_window_get_time(const bov_window_t* window);


/* get the value of the counter associated to a window. The counter is set to
 * 0 at window creation. It is incremented every time the user hit the up
 * arrow key, and decremented every time the user hit the down arrow key.
 */
static inline unsigned bov_window_get_counter(const bov_window_t* window);
static inline void bov_window_set_counter(bov_window_t* window,
                                          unsigned counter);

/* get the resolution in pixel of the window */
static inline GLfloat bov_window_get_xres(const bov_window_t* window);
static inline GLfloat bov_window_get_yres(const bov_window_t* window);

/* tells if the window should close (because the user decided it) */
static inline int bov_window_should_close(const bov_window_t* window);

/* sets the background color with a 4-channel color (red, blue, green, alpha).
 * The alpha channel (transparency) is useless here */
static inline void bov_window_set_color(bov_window_t* window,
                                        const GLfloat rgba[4]);

/* translates the content of the window (similar to dragging with the mouse */
static inline void bov_window_translate(bov_window_t* window,
                                        const GLfloat pos[2]);

/* set the scaling factor of the whole window content (similar to zooming with
 * the mouse) */
static inline void bov_window_set_zoom(bov_window_t* window,
                                       GLfloat zoom);

/* get the current zooming factor */
static inline GLfloat bov_window_get_zoom(const bov_window_t* window);

/* enable/disable a little help message for keyboard shortcuts */
static inline void bov_window_enable_help(bov_window_t* window);
static inline void bov_window_disable_help(bov_window_t* window);

/* take a screenshot and save it as a PPM with the name 'filename' */
void bov_window_screenshot(const bov_window_t* window,
                           const char* filename);

/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  text parameters
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
/* set the position of the text on the screen.
 * Note that coordinates must be given in pixels if the space type is set to
 * PIXEL_SPACE*/
static inline void bov_text_set_pos(bov_text_t* text,
                                    const GLfloat pos[2]);

/* set the font size of the text object on the screen.
 * The font size is equal to the baselineskip, which is
 * the height of a line of characters.
 * The width of each character == baselineskip/2.
 * Note that the baselineskip must be given in pixels if
 * the space type is set to PIXEL_SPACE */
static inline void bov_text_set_fontsize(bov_text_t* text,
                                         GLfloat baselineskip);

/* sets the text color to a 4 channel color (red, gree, blue, alpha) */
static inline void bov_text_set_color(bov_text_t* text,
                                      const GLfloat rgba[4]);

/* boldness is a parameter, usually between -1 and 1, that define
 * how massive the font should be. -1 means light, 1 means bold */
static inline void bov_text_set_boldness(bov_text_t* text,
                                         GLfloat width);

/* sets the text outline color to a 4 channel color (red, gree, blue, alpha) */
static inline void bov_text_set_outline_color(bov_text_t* text,
                                              const GLfloat rgba[4]);

/* for text, the outline width must be between 0 and 1
 * 0 means no outline, 1 means the outline might meet in the middle of the character */
static inline void bov_text_set_outline_width(bov_text_t* text,
                                              GLfloat width);

/* the outline can be slightly shifted in some direction.
 * More specifically, if you have a normal vector to the font 'n',
 * and a shift vector 's', and an outline width 'w', then we will have
 * a new outline width 'w = w + dot(s,n)'.
 * For example, if we have an outline width of 0.2, and a shift vector of
 * (0.1, 0.0), the outline will be 0.3 on the right of each character and
 * 0.1 on the left of each character
 *
 * Note: font with an outline shift can become very bad when zoom in.
 * actually, it uses the gradient of the sdf based font to calculate
 * the normal, and this gradient approaches 0 when zoomed in...
 */
static inline void bov_text_set_outline_shift(bov_text_t* text,
                                              const GLfloat shift[2]);

/* for more detail on what this function does, see:
 * - the structure `bov_space_type_t`
 * - the function `bov_text_set_pos()` and `text_set_fontsize()`
 *
 * Changing the spaceType of a text object to PIXEL_SPACE also sets its
 * position and scaling to meaningfull values (negative positions are set to
 * zero, fontSize less than 32 are set to 32) */
static inline void bov_text_set_space_type(bov_text_t* text,
                                           bov_space_type_t spaceType);

/* a structure to hold all the parameters of a text object */
typedef struct {
	GLfloat fillColor[4];
	GLfloat outlineColor[4];
	GLfloat pos[2];
	GLfloat shift[2];
	GLfloat fontSize;
	GLfloat boldness;
	GLfloat outlineWidth;
	bov_space_type_t spaceType;
} bov_text_param_t;

/* retrieve all the parameters from a text object */
static inline bov_text_param_t bov_text_get_param(const bov_text_t* text);

/* set all the parameters of a text object at once */
static inline void bov_text_set_param(bov_text_t* text,
                                      bov_text_param_t parameters);


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  points parameters
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
/* set the position of the points on the screen.
 * Note that coordinates must be given in pixels if the space type is set to
 * PIXEL_SPACE*/
static inline void bov_points_set_pos(bov_points_t* points,
                                      const GLfloat pos[2]);

/* you can give a local scaling factor to your set of points in both x and y
 * directions. The translation given by points_set_pos() is applied AFTER the
 * local scaling, so it is not impacted by the scaling. The local scaling
 * factors is only applied to point coordinates, not to the width and outline
 * width*/
static inline void bov_points_scale(bov_points_t* points,
                                    const GLfloat scale[2]);

/* set the width of the marker/line/curve/... associated with the points
 * Note that the width must be given in pixels if the space type is set to
 * PIXEL_SPACE*/
static inline void bov_points_set_width(bov_points_t* points,
                                        GLfloat width);

/* sets color of the marker/line/curve/... associated with the points to a 4
 * channel color (red, gree, blue, alpha) */
static inline void bov_points_set_color(bov_points_t* points,
                                        const GLfloat rgba[4]);

static inline void bov_points_set_outline_color(bov_points_t* points,
                                            const GLfloat rgba[4]);

static inline void bov_points_set_outline_width(bov_points_t* points,
                                                GLfloat width);

/* TODO: document the different shapes */
static inline void bov_points_set_marker(bov_points_t* points,
                                         GLfloat marker);

/* for more detail on what this function does, see:
 * - the structure `bov_space_type_t`
 * - the function `bov_points_set_pos()` and `points_set_width()`
 *
 * Changing the spaceType of a text object to PIXEL_SPACE also sets its
 * position and scaling to meaningfull values (negative positions are set to
 * zero, width less than 32 are set to 32) */
static inline void bov_points_set_space_type(bov_points_t* points,
                                             bov_space_type_t spaceType);

/* a structure to hold all the parameters of a points object */
typedef struct {
	GLfloat fillColor[4];
	GLfloat outlineColor[4];
	GLfloat pos[2];
	GLfloat scale[2];
	GLfloat width;
	GLfloat marker;
	GLfloat outlineWidth;
	bov_space_type_t spaceType;
} bov_points_param_t;

/* retrieve all the parameters from a points object */
static inline bov_points_param_t bov_points_get_param(const bov_points_t* points);

/* set all the parameters of a points object at once */
static inline void bov_points_set_param(bov_points_t* points,
                                        bov_points_param_t parameters);




/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %      error_log
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
/* some error code in addition to those defined by GLFW (www.glfw.org/docs/latest/group__errors.html) */
#define BOV_PARAMETER_ERROR   (0x00020001)
#define BOV_SHADER_ERROR      (0x00020002)
#define BOV_FRAMEBUFFER_ERROR (0x00020003)
#define BOV_GLAD_ERROR        (0x00020004)

#define BOV_OUT_OF_MEM_ERROR  (0x00030001)
#define BOV_IO_ERROR          (0x00030002)

/* display an error with BOV_ERROR_LOG, for example :
 * if(parameter_not_valid) {
 *     BOV_ERROR_LOG(BOV_PARAMETER_ERROR, "parameter XX must be positive !");
 *     return NULL;
 */
#ifndef NDEBUG
#define BOV_ERROR_LOG(errorCode, fmt, ...) do{ \
		bov_error_log(errorCode, fmt, ## __VA_ARGS__ ); \
		fprintf(stderr, "\t(in function %s, line %d)\n", __func__, __LINE__); \
	}while(0)
#else
#define BOV_ERROR_LOG(errorCode,fmt, ...) \
  bov_error_log(errorCode, fmt, ## __VA_ARGS__ )
#endif


/***********************************************************************************/



/*       __| | ___  _ __ |/| |_  | | ___   ___ | | __ | |__   ___| | _____      __
 *      / _` |/ _ \| '_ \  | __| | |/ _ \ / _ \| |/ / | '_ \ / _ \ |/ _ \ \ /\ / /
 *     | (_| | (_) | | | | | |_  | | (_) | (_) |   <  | |_) |  __/ | (_) \ V  V /
 *      \__,_|\___/|_| |_|  \__| |_|\___/ \___/|_|\_\ |_.__/ \___|_|\___/ \_/\_/
 * (seriously)
 */








































































































































































































































































/*      _____   _____   _____ __      __    _______  ______
 *     |  __ \ |  __ \ |_   _|\ \    / //\ |__   __||  ____|
 *     | |__) || |__) |  | |   \ \  / //  \   | |   | |__
 *     |  ___/ |  _  /   | |    \ \/ // /\ \  | |   |  __|
 *     | |     | | \ \  _| |_    \  // ____ \ | |   | |____
 *     |_|     |_|  \_\|_____|    \//_/    \_\|_|   |______|
 */

typedef struct {
	GLfloat res[2];
	GLfloat translate[2];
	GLfloat zoom;
} bov_world_param_t;

struct bov_window_struct
{
	GLFWwindow* self;
	GLFWcursor* leftClickCursor;

	bov_world_param_t param;
  GLfloat backgroundColor[4];

	int size[2];         // size of window (might not be in pixel => !=framebuffer)
	double cursorPos[2]; // new position of the cursor in screen coordinates
	double clickTime[2]; // time of the click (left and right) or -1 if released
	double wtime;
  unsigned counter;

	GLuint ubo[2]; // 1 ubo for world params, 1 ubo for points and text params

	int last_program;
	GLuint program[8];
	GLuint font_atlas_texture;

	int running;

	bov_text_t* help;
	int help_needed;
	bov_text_t* indication;
	int indication_needed;
};

struct bov_text_struct {
	GLuint vao;
	GLuint vbo;
	GLsizei vboCapacity; // capacity of the vbo
	GLsizei vboLen;      // number of letter in vbo
	// const GLubyte* string;
	GLsizei dataCapacity; // length of the string
	GLfloat* data;
	bov_text_param_t param;
};

struct bov_points_struct{
	GLuint vao;
	GLuint vbo;
	GLsizei vboCapacity;
	GLsizei vboLen;
	bov_points_param_t param;
};


static inline double bov_window_get_time(const bov_window_t* window)
{
	return window->wtime;
}

static inline unsigned bov_window_get_counter(const bov_window_t* window)
{
  return window->counter;
}

static inline void bov_window_set_counter(bov_window_t* window,
                                          unsigned counter)
{
  window->counter = counter;
}

static inline GLfloat bov_window_get_xres(const bov_window_t* window)
{
	return window->param.res[0];
}

static inline GLfloat bov_window_get_yres(const bov_window_t* window)
{
	return window->param.res[1];
}


static inline int bov_window_should_close(const bov_window_t* window)
{
	return glfwWindowShouldClose(window->self);
}

static inline void bov_window_set_color(bov_window_t* window,
                                        const GLfloat rgba[4])
{
  for(int i=0; i<4; i++) {
    window->backgroundColor[i] = rgba[i];
  }
}

static inline void bov_window_translate(bov_window_t* window,
                                        const GLfloat pos[2])
{
	window->param.translate[0] += pos[0];
	window->param.translate[1] += pos[1];
}

static inline void bov_window_set_zoom(bov_window_t* window,
                                       GLfloat zoom)
{
	window->param.zoom = zoom;
}

static inline GLfloat bov_window_get_zoom(const bov_window_t* window)
{
	return window->param.zoom;
}

static inline void bov_window_enable_help(bov_window_t* window)
{
  window->indication_needed = 1;
}

static inline void bov_window_disable_help(bov_window_t* window)
{
  window->indication_needed = 0;
}


static inline void bov_text_set_pos(bov_text_t* text,
                                    const GLfloat pos[2])
{
	text->param.pos[0] = pos[0];
	text->param.pos[1] = pos[1];
}

static inline void bov_text_set_fontsize(bov_text_t* text,
                                         GLfloat baselineskip)
{
	text->param.fontSize = baselineskip;
}

static inline void bov_text_set_color(bov_text_t* text,
                                      const GLfloat rgba[4])
{
	for(int i=0; i<4; i++)
		text->param.fillColor[i] = rgba[i];
}

static inline void bov_text_set_boldness(bov_text_t* text,
                                         GLfloat boldness)
{
	text->param.boldness = boldness;
}

static inline void bov_text_set_outline_color(bov_text_t* text,
                                              const GLfloat rgba[4])
{
	for(int i=0; i<4; i++)
		text->param.outlineColor[i] = rgba[i];
}

static inline void bov_text_set_outline_width(bov_text_t* text,
                                              GLfloat width)
{
	text->param.outlineWidth = width;
}

static inline void bov_text_set_outline_shift(bov_text_t* text,
                                              const GLfloat shift[2])
{
	text->param.shift[0] = shift[0];
	text->param.shift[1] = shift[1];
}

static inline void bov_text_set_space_type(bov_text_t* text,
                                           bov_space_type_t spaceType)
{
	if(spaceType==PIXEL_SPACE && text->param.spaceType!=PIXEL_SPACE) {
		if(text->param.pos[0] < 5.0f)
			text->param.pos[0] = 5.0f;
		if(text->param.pos[1] < 5.0f)
			text->param.pos[1] = 5.0f;
		if(text->param.fontSize < 32.0f)
			text->param.fontSize = 32.0f;
	}

	text->param.spaceType = spaceType;
}

static inline bov_text_param_t bov_text_get_param(const bov_text_t* text)
{
	return text->param;
}

static inline void bov_text_set_param(bov_text_t* text,
                                      bov_text_param_t parameters)
{
	text->param = parameters;
}


static inline void bov_points_set_pos(bov_points_t* points,
                                      const GLfloat pos[2])
{
	points->param.pos[0] = pos[0];
	points->param.pos[1] = pos[1];
}

static inline void bov_points_scale(bov_points_t* points,
                                    const GLfloat scale[2])
{
	points->param.scale[0] = scale[0];
	points->param.scale[1] = scale[1];
}

static inline void bov_points_set_width(bov_points_t* points,
                                        GLfloat width)
{
	points->param.width = width;
}

static inline void bov_points_set_color(bov_points_t* points,
                                        const GLfloat rgba[4])
{
	for (int i=0; i<4; i++)
		points->param.fillColor[i] = rgba[i];
}

static inline void bov_points_set_outline_color(bov_points_t* points,
                                                const GLfloat rgba[4])
{
	for (int i=0; i<4; i++)
		points->param.outlineColor[i] = rgba[i];
}

static inline void bov_points_set_outline_width(bov_points_t* points,
                                                GLfloat width)
{
	points->param.outlineWidth = width;
}

static inline void bov_points_set_marker(bov_points_t* points,
                                         GLfloat marker)
{
	points->param.marker = marker;
}

static inline void bov_points_set_space_type(bov_points_t* points,
                                             bov_space_type_t spaceType)
{
	if(spaceType==PIXEL_SPACE && points->param.spaceType!=PIXEL_SPACE) {
		if(points->param.pos[0] < 5.0f)
			points->param.pos[0] = 5.0f;
		if(points->param.pos[1] < 5.0f)
			points->param.pos[1] = 5.0f;
		if(points->param.width < 2.0f)
			points->param.width = 2.0f;
	}
	points->param.spaceType = spaceType;
}

static inline bov_points_param_t bov_points_get_param(const bov_points_t* points)
{
	return points->param;
}

static inline void bov_points_set_param(bov_points_t* points,
                                        bov_points_param_t parameters)
{
	points->param = parameters;
}




typedef enum {
	POINTS_PROGRAM,
	LINES_PROGRAM,
	LINE_LOOP_PROGRAM,
	LINE_STRIP_PROGRAM,
	CURVE_PROGRAM,
	TRIANGLES_PROGRAM,
	TRIANGLE_STRIP_PROGRAM,
	TRIANGLE_FAN_PROGRAM,
	// QUADS_PROGRAM,
  FAST_POINTS_PROGRAM,
  FAST_LINES_PROGRAM,
  FAST_LINE_LOOP_PROGRAM,
  FAST_LINE_STRIP_PROGRAM,
  FAST_TRIANGLES_PROGRAM,
  FAST_TRIANGLE_STRIP_PROGRAM,
  FAST_TRIANGLE_FAN_PROGRAM,
} bov_points_drawing_mode_t;


void bov_points_draw_aux(bov_window_t* window,
                         const bov_points_t* points,
                         bov_points_drawing_mode_t mode,
                         GLint start,
                         GLsizei count);

void bov_points_draw_with_order_aux(bov_window_t* window,
                                    const bov_points_t* points,
                                    bov_points_drawing_mode_t mode,
                                    const bov_order_t* order,
                                    GLint start,
                                    GLsizei count);

// void bov_points_draw_with_indices_aux(bov_window_t* window,
//                                       const bov_points_t* points,
//                                       bov_points_drawing_mode_t mode,
//                                       const GLuint* indices,
//                                       GLint start,
//                                       GLsizei count);

// use cpp to preprocess a file with
#define BOVCreateDrawingFunctions(primitive, program)\
static inline void bov_##primitive##_draw(\
bov_window_t* win,const bov_points_t*pts,GLint start,GLsizei count){\
bov_points_draw_aux(win,pts,program,start,count);}\
static inline void bov_##primitive##_draw_with_order(\
bov_window_t* win,const bov_points_t*pts,const bov_order_t*order,GLint start,GLsizei count){\
bov_points_draw_with_order_aux(win,pts,program,order,start,count);}\
/* static inline void bov_##primitive##_draw_with_indices(\
 * bov_window_t* win,const bov_points_t*pts,const GLuint*indices,GLint start,GLsizei count){\
 * bov_points_draw_with_indices_aux(win,pts,program,indices,start,count);} */

BOVCreateDrawingFunctions(points, POINTS_PROGRAM)
BOVCreateDrawingFunctions(lines, LINES_PROGRAM)
BOVCreateDrawingFunctions(line_strip, LINE_STRIP_PROGRAM)
BOVCreateDrawingFunctions(line_loop, LINE_LOOP_PROGRAM)
BOVCreateDrawingFunctions(curve, CURVE_PROGRAM)
BOVCreateDrawingFunctions(triangles, TRIANGLES_PROGRAM)
BOVCreateDrawingFunctions(triangle_strip, TRIANGLE_STRIP_PROGRAM)
BOVCreateDrawingFunctions(triangle_fan, TRIANGLE_FAN_PROGRAM)
// BOVCreateDrawingFunctions(quads, QUADS_PROGRAM)

// alternative shaders, using default primitive rendering
// they understand only fillColor and the space_type for the position
// of points
BOVCreateDrawingFunctions(fast_points, FAST_POINTS_PROGRAM)
BOVCreateDrawingFunctions(fast_lines, FAST_LINES_PROGRAM)
BOVCreateDrawingFunctions(fast_line_strip, FAST_LINE_STRIP_PROGRAM)
BOVCreateDrawingFunctions(fast_line_loop, FAST_LINE_LOOP_PROGRAM)
BOVCreateDrawingFunctions(fast_triangles, FAST_TRIANGLES_PROGRAM)
BOVCreateDrawingFunctions(fast_triangle_strip, FAST_TRIANGLE_STRIP_PROGRAM)
BOVCreateDrawingFunctions(fast_triangle_fan, FAST_TRIANGLE_FAN_PROGRAM)

#ifdef __GNUC__
void bov_error_log(int errorCode, const char *fmt, ...)__attribute__((format (printf, 2, 3)));
#else
void bov_error_log(int errorCode, const char *fmt, ...);
#endif

#endif

