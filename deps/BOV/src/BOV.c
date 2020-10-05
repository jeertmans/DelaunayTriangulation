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

#define _CRT_SECURE_NO_WARNINGS // for MSVC to be happy

#include "BOV.h"
#include <math.h>
#include <float.h>
/***************************************
 *   FONT DEFINITION                   *
 ***************************************/
#include "big_font.h"
// #include "font.h"

/***************************************
 *   SHADERS                           *
 ***************************************/
// a la https://github.com/christophercrouzet/opengl-bootstrap
#include "points_vert.h"
#include "points_geom.h"
#include "points_frag.h"
#include "lines_geom.h"
#include "lines_frag.h"
#include "curve_geom.h"
#include "triangles_geom.h"
#include "triangles_frag.h"
#include "text_vert.h"
#include "text_frag.h"
#include "default_vert.h"
#include "default_frag.h"

#define POS_LOCATION 0
#define TEX_LOCATION 1
#define SPEED_LOCATION 1
#define DATA_LOCATION 2

#define FONT_ATLAS_TEXTURE_UNIT 0
#define FRAMEBUFFER_TEXTURE_UNIT 1

#define TEXT_PROGRAM_INDEX       0
#define POINTS_PROGRAM_INDEX     1
#define LINES_PROGRAM_INDEX      2
#define CURVE_PROGRAM_INDEX      3
#define TRIANGLES_PROGRAM_INDEX  4
#define DEFAULT_PROGRAM_INDEX    5
// #define QUAD_PROGRAM_INDEX     8




/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Struct def
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
struct bov_order_struct{
	GLuint ebo;
	GLsizei eboCapacity;
	GLsizei eboLen;
};


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Errors
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
void bov_error_log(int errorCode, const char *fmt, ...)
{
	if(errorCode>=BOV_OUT_OF_MEM_ERROR)
		perror("Current system error message: ");

	va_list vl;
	fprintf(stderr, "=X= Error %d: ", errorCode);
	va_start(vl,fmt);
	vfprintf(stderr, fmt, vl);
	va_end(vl);
	fputc('\n', stderr);
}


/* used to check the result of a memory allocation, exit on failure */
#define CHECK_MALLOC(ptr) if((ptr)==NULL) { \
		BOV_ERROR_LOG(BOV_OUT_OF_MEM_ERROR, \
		              "Memory allocation failed"); \
		exit(EXIT_FAILURE); }


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Loading shaders
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
static int checkivError(GLuint object, GLenum pname,
                        PFNGLGETPROGRAMIVPROC glGetiv,
                        PFNGLGETPROGRAMINFOLOGPROC glGetInfoLog)
{
	GLint compile_status = GL_TRUE;
	glGetiv(object, pname, &compile_status);
	if(compile_status!=GL_TRUE)
	{
		GLint logsize;
		glGetiv(object, GL_INFO_LOG_LENGTH, &logsize);

		char *log = malloc((size_t) logsize + 1);
		CHECK_MALLOC(log);

		glGetInfoLog(object, logsize, &logsize, log);
		if(pname==GL_LINK_STATUS)
			BOV_ERROR_LOG(BOV_SHADER_ERROR,
			              "%s\t-> link operation failed",
			              log);
		else if(pname==GL_COMPILE_STATUS)
			BOV_ERROR_LOG(BOV_SHADER_ERROR,
			              "%s\t-> compile operation failed",
			              log);
		else
			BOV_ERROR_LOG(BOV_SHADER_ERROR,
			              "%s\t-> Unknown object operation failed",
			              log);
		free(log);
		return BOV_SHADER_ERROR;
	}
	return 0;
}


static GLuint LoadShader(GLsizei count,
                         const GLchar* shaderSource[],
                         const GLint* lengths,
                         const char* shaderName,
                         GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	if(shader==0) {
		BOV_ERROR_LOG(BOV_SHADER_ERROR,
		              "shader '%s' creation failed",
		              shaderName);
		return 0;
	}

	glShaderSource(shader, count, shaderSource, lengths);
	glCompileShader(shader);

	if(checkivError(shader, GL_COMPILE_STATUS, glGetShaderiv,
	                glGetShaderInfoLog)){
		const char* shaderType_desc = "unknown-type";
		if(shaderType==GL_VERTEX_SHADER)
			shaderType_desc = "vertex";
		else if(shaderType==GL_FRAGMENT_SHADER)
			shaderType_desc = "fragment";
		else if(shaderType==GL_GEOMETRY_SHADER)
			shaderType_desc = "geometry";
		BOV_ERROR_LOG(BOV_SHADER_ERROR,
		              "%s shader: %s compilation failed",
		              shaderType_desc, shaderName);
		return 0;
	}

	return shader;
}


static int program_init(bov_window_t* window, int program_index, int n, ...)
{
	GLuint shaderProgram =  window->program[program_index];
	int i;
	va_list vl;

	va_start(vl,n);
	for (i=0; i<n; i++)
	{
		glAttachShader(shaderProgram, va_arg(vl,GLuint));
	}
	va_end(vl);

	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);

	if(checkivError(shaderProgram,
	                GL_LINK_STATUS,
	                glGetProgramiv,
	                glGetProgramInfoLog)) {
		BOV_ERROR_LOG(BOV_SHADER_ERROR,
		              "shader program %d creation failed",
		              program_index);
		return 1;
	}

	return 0;
}


static GLuint create_texture(GLsizei width,
                             GLsizei height,
                             const GLvoid* data,
                             GLenum internalFormat,
                             GLenum format,
                             GLint wrapParam)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format,
	             GL_UNSIGNED_BYTE, data);

	// glGenerateMipmap(GL_TEXTURE_2D);  //Generate mipmaps
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapParam);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapParam);
	// glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Rasterizers
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
static void text_rasterizer_init(bov_window_t* window)
{
	GLubyte (*image)[3] = malloc(sizeof(char) * 3 *
									   font.tex_width * font.tex_height);
	CHECK_MALLOC(image);

	// int smoothing[2] = {1, 2/*, 1*/}; // smoothing in Sobel kernel
	// int smoothing[2] = {3, 10/*, 3*/};   // in Scharr kernel
	int smoothing[2] = {47, 162/*, 47*/};// wikipedia optimal Scharr
	// int smoothing[2] = {0, 1/*, 0*/}; // no smoothing
	// int smoothing[2] = {1,1/*,1*/}; // mega smoothing

	// compute derivatives with a sort of Sobel kernel, which is
	// a combination of finite difference and smoothing
	for(unsigned y=1; y<font.tex_height-1; y++) {
		for(unsigned x=1; x<font.tex_width-1; x++) {
			unsigned index = y * font.tex_width + x;
			int gx =  font.tex_data[y-1][x-1] * ( smoothing[0]) +
			          font.tex_data[y-1][x+1] * (-smoothing[0]) +
			          font.tex_data[y][x-1]   * ( smoothing[1]) +
			          font.tex_data[y][x+1]   * (-smoothing[1]) +
			          font.tex_data[y+1][x-1] * ( smoothing[0]) +
			          font.tex_data[y+1][x+1] * (-smoothing[0]);

			int gy =  font.tex_data[y-1][x-1] * ( smoothing[0]) +
			          font.tex_data[y-1][x+0] * ( smoothing[1]) +
			          font.tex_data[y-1][x+1] * ( smoothing[0]) +
			          font.tex_data[y+1][x-1] * (-smoothing[0]) +
			          font.tex_data[y+1][x+0] * (-smoothing[1]) +
			          font.tex_data[y+1][x+1] * (-smoothing[0]);


			image[index][0] = font.tex_data[y][x];
			double ampl = sqrt(gx * gx + gy * gy);
			image[index][1] = (GLubyte) fmin(255.9, fmax(0.0, gx / ampl * 128. + 128.));
			image[index][2] = (GLubyte) fmin(255.9, fmax(0.0, gy / ampl * 128. + 128.));
		}
	}

	for(unsigned y=0; y<font.tex_height; y++) {
		unsigned index = y * font.tex_width;
		image[index][0] = font.tex_data[y][0];
		image[index][1] = 0;
		image[index][2] = 0;
	}

	for(unsigned y=0; y<font.tex_height; y++) {
		unsigned index = y * font.tex_width + font.tex_width - 1;
		image[index][0] = font.tex_data[y][font.tex_width - 1];
		image[index][1] = 0;
		image[index][2] = 0;
	}

	for(unsigned x=1; x<font.tex_width-1; x++) {
		image[x][0] = font.tex_data[0][x];
		image[x][1] = 0;
		image[x][2] = 0;
	}

	for(unsigned x=1; x<font.tex_width-1; x++) {
		unsigned index = (font.tex_height - 1) * font.tex_width + x;
		image[index][0] = font.tex_data[font.tex_height - 1][x];
		image[index][1] = 0;
		image[index][2] = 0;
	}

	glActiveTexture(GL_TEXTURE0 + FONT_ATLAS_TEXTURE_UNIT);
	window->font_atlas_texture = create_texture(font.tex_width,
	                                            font.tex_height,
	                                            image,
	                                            GL_RGB,
	                                            GL_RGB,
	                                            GL_CLAMP_TO_EDGE);
	free(image);

	GLuint program = window->program[TEXT_PROGRAM_INDEX];
	glUseProgram(program);

	unsigned worldBlockIndex = glGetUniformBlockIndex(program, "worldBlock");
	unsigned objectBlockIndex = glGetUniformBlockIndex(program, "objectBlock");
	glUniformBlockBinding(program, worldBlockIndex, 0);
	glUniformBlockBinding(program, objectBlockIndex, 1);

	// uniform retain their value for the lifetime of a program
	// see glspec41.core 2.11.7
	glUniform1i(glGetUniformLocation(program, "fontTex"),
	                                 FONT_ATLAS_TEXTURE_UNIT);
}


static void points_rasterizer_init(GLuint program)
{
	glUseProgram(program);

	unsigned worldBlockIndex = glGetUniformBlockIndex(program, "worldBlock");
	unsigned objectBlockIndex = glGetUniformBlockIndex(program, "objectBlock");
	glUniformBlockBinding(program, worldBlockIndex, 0);
	glUniformBlockBinding(program, objectBlockIndex, 1);
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %   mouse_button_callback
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
static void mouse_button_callback(GLFWwindow* self,
                                  int button, int action, int mod)
{
	bov_window_t* window = (bov_window_t*) glfwGetWindowUserPointer(self);

	if(button==GLFW_MOUSE_BUTTON_LEFT) {
		if(action==GLFW_PRESS){
			glfwSetCursor(self, window->leftClickCursor);
			window->clickTime[0] = window->wtime;
		}
		else {
			glfwSetCursor(self, NULL);
			window->clickTime[0] = -window->wtime;
		}
	}
	else if(button==GLFW_MOUSE_BUTTON_RIGHT) {
		if(action==GLFW_PRESS) {
			glfwSetInputMode(self, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			window->clickTime[1] = window->wtime;
		}
		else {
			glfwSetInputMode(self, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			window->clickTime[1] = -window->wtime;
		}
	}
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %   mouse_button_callback
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
static void scroll_callback(GLFWwindow* self, double x, double y) {
	bov_window_t* window = (bov_window_t*) glfwGetWindowUserPointer(self);

	if(y>0.0) {
		window->param.zoom *= (GLfloat)(1.0 + 0.1 * y);
	}
	else if(y<0.0) {
		window->param.zoom /= (GLfloat)(1.0 - 0.1 * y);
	}
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %   cursor_pos_callback
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
static void cursor_pos_callback(GLFWwindow* self, double x, double y)
{
	bov_window_t* window = (bov_window_t*) glfwGetWindowUserPointer(self);

	double scale[2];
	if(window->param.res[0] > window->param.res[1]) {
		scale[0] = window->param.res[1] / window->param.res[0];
		scale[1] = 1.0;
	}
	else {
		scale[0] = 1.0;
		scale[1] = window->param.res[0] / window->param.res[1];
	}
	// TODO: modify this to use the scale
	double newX = (2.0 * x / window->size[0] - 1.0) /
	                (scale[0] * window->param.zoom);
	double newY = (2.0 * (1.0 - y / window->size[1]) - 1.0) /
	                (scale[1] * window->param.zoom);

	if(window->clickTime[0]>0.0 || window->clickTime[1]>0.0) {
		window->param.translate[0] += (GLfloat) (newX - window->cursorPos[0]);
		window->param.translate[1] += (GLfloat) (newY - window->cursorPos[1]);
	}

	window->cursorPos[0] = (GLfloat) newX;
	window->cursorPos[1] = (GLfloat) newY;
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %      error_callback
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
static void error_callback(int errorCode, const char* description)
{
	BOV_ERROR_LOG(errorCode, "%s", description);
	exit(EXIT_FAILURE);
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %       key_callback
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
static void key_callback(GLFWwindow* self,
                         int key, int scancode, int action,int mods)
{
	static unsigned screenshot_nbr = 0;
	char screenshot_name[64] = "screenshot";
	bov_window_t* window = (bov_window_t*) glfwGetWindowUserPointer(self);
	if(action==GLFW_PRESS || action==GLFW_REPEAT) {
		switch(key) {
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(self,GL_TRUE);
				break;
			case GLFW_KEY_SPACE:
				if(window->running==0) {
					window->running = 1;
				}
				else {
					window->running = 0;
				}
				break;
			case GLFW_KEY_H:
			case GLFW_KEY_K:
				if(window->help_needed==0) {
					window->help_needed = 1;
				}
				else {
					window->help_needed = 0;
				}
				break;
			case GLFW_KEY_P:
				snprintf(screenshot_name + 10, 54, "%u.ppm", screenshot_nbr++);
				bov_window_screenshot(window, screenshot_name);
				break;
			case GLFW_KEY_R:
				window->param.zoom = 1.0f;
				window->param.translate[0] = 0.0f;
				window->param.translate[1] = 0.0f;
				break;
			case GLFW_KEY_UP:
				window->counter++;
				break;
			case GLFW_KEY_DOWN:
				window->counter--;
				break;
		}
	}
	if(key==GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(self,GL_TRUE);
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  FrameBuffer_callback
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
static void framebuffer_size_callback(GLFWwindow* self, int width, int height)
{
	bov_window_t* window = (bov_window_t*) glfwGetWindowUserPointer(self);
	window->param.res[0] = (GLfloat) width;
	window->param.res[1] = (GLfloat) height;
	glViewport(0, 0, width, height);
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Window_callback
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
static void window_size_callback(GLFWwindow* self, int width, int height)
{
	bov_window_t* window = (bov_window_t*) glfwGetWindowUserPointer(self);
	window->size[0] = width;
	window->size[1] = height;
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Window object
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
static void window_OpenGL_init(bov_window_t* window)
{
#ifndef NDEBUG
	{
		bov_text_param_t t;
		bov_points_param_t p;
		if(sizeof(bov_points_param_t)!=sizeof(bov_text_param_t) ||
		   ((char*) &t.fillColor - (char*) &t) != ((char*) &p.fillColor - (char*) &p) ||
		   ((char*) &t.fillColor - (char*) &t) != 0  ||
		   ((char*) &t.outlineColor - (char*) &t) != ((char*) &p.outlineColor - (char*) &p) ||
		   ((char*) &t.outlineColor - (char*) &t) != 16 ||
		   ((char*) &t.pos - (char*) &t) != ((char*) &p.pos - (char*) &p) ||
		   ((char*) &t.pos - (char*) &t) != 32 ||
		   ((char*) &t.shift - (char*) &t) != ((char*) &p.scale - (char*) &p) ||
		   ((char*) &t.shift - (char*) &t) != 40 ||
		   ((char*) &t.fontSize - (char*) &t) != ((char*) &p.width - (char*) &p) ||
		   ((char*) &t.fontSize - (char*) &t) != 48 ||
		   ((char*) &t.boldness - (char*) &t) != ((char*) &p.marker - (char*) &p) ||
		   ((char*) &t.boldness - (char*) &t) != 52 ||
		   ((char*) &t.outlineWidth - (char*) &t) != ((char*) &p.outlineWidth - (char*) &p) ||
		   ((char*) &t.outlineWidth - (char*) &t) != 56
		  ) {
			BOV_ERROR_LOG(BOV_SHADER_ERROR,
			              "bov_points_param_t and bov_text_param_t must "
			              "have identical fields");
			fprintf(stderr, "%zu\n", (char*) &t.fillColor - (char*) &t);
			fprintf(stderr, "%zu\n", (char*) &t.outlineColor - (char*) &t);
			fprintf(stderr, "%zu\n", (char*) &t.pos - (char*) &t);
			fprintf(stderr, "%zu\n", (char*) &t.shift - (char*) &t);
			fprintf(stderr, "%zu\n", (char*) &t.fontSize - (char*) &t);
			fprintf(stderr, "%zu\n", (char*) &t.boldness - (char*) &t);
			fprintf(stderr, "%zu\n", (char*) &t.outlineWidth - (char*) &t);
			exit(EXIT_FAILURE);
		}
	}
#endif

	// Create draw and object UBO
	glGenBuffers(2, window->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, window->ubo[0]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(bov_world_param_t), NULL, GL_STATIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, window->ubo[0], 0, sizeof(bov_world_param_t));

	glBindBuffer(GL_UNIFORM_BUFFER, window->ubo[1]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(bov_points_param_t), NULL, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, window->ubo[1], 0, sizeof(bov_points_param_t));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// compile shaders
	{ // text
		GLuint fontVS, fontFS;
		if((fontVS = LoadShader(1,
		                        (const GLchar* []) {text_vert},
		                        (GLint[]){ sizeof(text_vert) - 1 },
		                        "text_vert.glsl",
		                        GL_VERTEX_SHADER))==0)
			goto shader_error;
		if((fontFS = LoadShader(1,
		                        (const GLchar* []) {text_frag},
		                        (GLint[]){ sizeof(text_frag) - 1 },
		                        "text_frag.glsl",
		                        GL_FRAGMENT_SHADER))==0)
			goto shader_error;

		window->program[TEXT_PROGRAM_INDEX] = glCreateProgram();

		// Specify the layout of the vertex data
		glBindAttribLocation(window->program[TEXT_PROGRAM_INDEX],
		                     POS_LOCATION, "pos");
		glBindAttribLocation(window->program[TEXT_PROGRAM_INDEX],
		                     TEX_LOCATION, "tex");

		if(program_init(window, TEXT_PROGRAM_INDEX, 2, fontVS, fontFS))
			goto shader_error;
		glDetachShader(window->program[TEXT_PROGRAM_INDEX],fontVS);
		glDetachShader(window->program[TEXT_PROGRAM_INDEX],fontFS);

		glDeleteShader(fontVS);
		glDeleteShader(fontFS);
	}

	{ // points
		GLuint pointsVS, pointsGS, linesGS, curveGS, pointsFS, linesFS,
		       trianglesGS, trianglesFS, defaultVS, defaultFS;

		if((pointsVS = LoadShader(1,
		                          (const GLchar* []) {points_vert},
		                          (const GLint[]) {sizeof(points_vert) - 1},
		                          "points_vert.glsl",
		                          GL_VERTEX_SHADER))==0)
			goto shader_error;
		if((pointsGS = LoadShader(1,
		                          (const GLchar* []) {points_geom},
		                          (const GLint[]) {sizeof(points_geom) - 1},
		                          "points_geom.glsl",
		                          GL_GEOMETRY_SHADER))==0)
			goto shader_error;
		if((linesGS = LoadShader(1,
		                         (const GLchar* []) {lines_geom},
		                         (const GLint[]) {sizeof(lines_geom) - 1},
		                         "lines_geom.glsl",
		                         GL_GEOMETRY_SHADER))==0)
			goto shader_error;
		if((curveGS = LoadShader(1,
		                         (const GLchar* []) {curve_geom},
		                         (const GLint[]) {sizeof(curve_geom) - 1},
		                         "curve_geom.glsl",
		                         GL_GEOMETRY_SHADER))==0)
			goto shader_error;
		if((trianglesGS = LoadShader(1,
		                             (const GLchar* []) {triangles_geom},
		                             (const GLint[]) {sizeof(triangles_geom) - 1},
		                             "triangles_geom.glsl",
		                             GL_GEOMETRY_SHADER))==0)
			goto shader_error;
		if((pointsFS = LoadShader(1,
		                          (const GLchar* []) {points_frag},
		                          (const GLint[]) {sizeof(points_frag) - 1},
		                          "points_frag.glsl",
		                          GL_FRAGMENT_SHADER))==0)
			goto shader_error;
		if((linesFS = LoadShader(1,
		                         (const GLchar* []) {lines_frag},
		                         (const GLint[]) {sizeof(lines_frag) - 1},
		                         "lines_frag.glsl",
		                         GL_FRAGMENT_SHADER))==0)
			goto shader_error;
		if((trianglesFS = LoadShader(1,
		                             (const GLchar* []) {triangles_frag},
		                             (const GLint[]) {sizeof(triangles_frag) - 1},
		                             "triangles_frag.glsl",
		                             GL_FRAGMENT_SHADER))==0)
			goto shader_error;
		if((defaultVS = LoadShader(1,
		                          (const GLchar* []) {default_vert},
		                          (const GLint[]) {sizeof(default_vert) - 1},
		                          "default_vert.glsl",
		                          GL_VERTEX_SHADER))==0)
			goto shader_error;
		if((defaultFS = LoadShader(1,
		                          (const GLchar* []) {default_frag},
		                          (const GLint[]) {sizeof(default_frag) - 1},
		                          "default_frag.glsl",
		                          GL_FRAGMENT_SHADER))==0)
			goto shader_error;

		window->program[POINTS_PROGRAM_INDEX] = glCreateProgram();
		window->program[LINES_PROGRAM_INDEX] = glCreateProgram();
		window->program[CURVE_PROGRAM_INDEX] = glCreateProgram();
		window->program[TRIANGLES_PROGRAM_INDEX] = glCreateProgram();
		window->program[DEFAULT_PROGRAM_INDEX] = glCreateProgram();

		// Specify the layout of the vertex data
		glBindAttribLocation(window->program[POINTS_PROGRAM_INDEX],
		                     POS_LOCATION, "pos");
		glBindAttribLocation(window->program[LINES_PROGRAM_INDEX],
		                     POS_LOCATION, "pos");
		glBindAttribLocation(window->program[CURVE_PROGRAM_INDEX],
		                     POS_LOCATION, "pos");
		glBindAttribLocation(window->program[TRIANGLES_PROGRAM_INDEX],
		                     POS_LOCATION, "pos");
		glBindAttribLocation(window->program[DEFAULT_PROGRAM_INDEX],
		                     POS_LOCATION, "pos");

		if(program_init(window, POINTS_PROGRAM_INDEX,
		                3, pointsVS, pointsGS, pointsFS))
			goto shader_error;
		glDetachShader(window->program[POINTS_PROGRAM_INDEX], pointsVS);
		glDetachShader(window->program[POINTS_PROGRAM_INDEX], pointsGS);
		glDetachShader(window->program[POINTS_PROGRAM_INDEX], pointsFS);

		glDeleteShader(pointsGS);
		glDeleteShader(pointsFS);

		if(program_init(window, LINES_PROGRAM_INDEX,
		                3, pointsVS, linesGS, linesFS))
			goto shader_error;
		glDetachShader(window->program[LINES_PROGRAM_INDEX], pointsVS);
		glDetachShader(window->program[LINES_PROGRAM_INDEX], linesGS);
		glDetachShader(window->program[LINES_PROGRAM_INDEX], linesFS);

		glDeleteShader(linesGS);

		if(program_init(window, CURVE_PROGRAM_INDEX,
		                3, pointsVS, curveGS, linesFS))
			goto shader_error;
		glDetachShader(window->program[CURVE_PROGRAM_INDEX], pointsVS);
		glDetachShader(window->program[CURVE_PROGRAM_INDEX], curveGS);
		glDetachShader(window->program[CURVE_PROGRAM_INDEX], linesFS);

		glDeleteShader(curveGS);
		glDeleteShader(linesFS);

		if(program_init(window, TRIANGLES_PROGRAM_INDEX,
		                3, pointsVS, trianglesGS, trianglesFS))
			goto shader_error;
		glDetachShader(window->program[TRIANGLES_PROGRAM_INDEX], pointsVS);
		glDetachShader(window->program[TRIANGLES_PROGRAM_INDEX], trianglesGS);
		glDetachShader(window->program[TRIANGLES_PROGRAM_INDEX], trianglesFS);

		glDeleteShader(pointsVS);
		glDeleteShader(trianglesGS);
		glDeleteShader(trianglesFS);

		if(program_init(window, DEFAULT_PROGRAM_INDEX,
		                2, defaultVS, defaultFS))
			goto shader_error;

		glDetachShader(window->program[DEFAULT_PROGRAM_INDEX], defaultVS);
		glDetachShader(window->program[DEFAULT_PROGRAM_INDEX], defaultFS);

		glDeleteShader(defaultVS);
		glDeleteShader(defaultFS);
	}

	// Create all rasterizers
	text_rasterizer_init(window);
	points_rasterizer_init(window->program[POINTS_PROGRAM_INDEX]);
	points_rasterizer_init(window->program[CURVE_PROGRAM_INDEX]);
	points_rasterizer_init(window->program[TRIANGLES_PROGRAM_INDEX]);
	points_rasterizer_init(window->program[LINES_PROGRAM_INDEX]);

	points_rasterizer_init(window->program[DEFAULT_PROGRAM_INDEX]);
	window->last_program = DEFAULT_PROGRAM_INDEX;

	return;

shader_error:
	BOV_ERROR_LOG(BOV_SHADER_ERROR,
	              "check your driver and OpenGL capabilities");
	exit(EXIT_FAILURE);
}


bov_window_t* bov_window_new(int width, int height, const char* win_name)
{
	bov_window_t* window = malloc(sizeof(bov_window_t));
	CHECK_MALLOC(window);

	glfwSetErrorCallback(error_callback);

	if(!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_SAMPLES, 0); // we disable multisampling has we already use shader antialiasing
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if(width==0 || height==0) {
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		window->size[0] = mode->width;
		window->size[1] = mode->height;
		window->self = glfwCreateWindow(mode->width, mode->height, win_name,
		                                glfwGetPrimaryMonitor(), // fullscreen
		                                NULL);
	}
	else {
		if(height<0) {
			glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
			height = -height;
		}

		if(width<0) {
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
			width = 100;
			height = 100;
		}
		window->size[0] = width;


		window->size[1] = height;

		window->self = glfwCreateWindow(width, height, win_name, NULL, NULL);
	}

	if(!window->self) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window->self);
	glfwSetWindowUserPointer(window->self,window);

	// load opengl functions with GLAD
	if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
		BOV_ERROR_LOG(BOV_GLAD_ERROR,
		              "Failed to initialize OpenGL context");

	window->param.zoom = 1.0;
	glfwGetFramebufferSize(window->self, &width, &height);
	framebuffer_size_callback(window->self, width, height);
	window->param.translate[0] = window->param.translate[1] = 0.0;
	window->wtime = DBL_MIN;

	// white background color
	window->backgroundColor[0] = 1.0;
	window->backgroundColor[1] = 1.0;
	window->backgroundColor[2] = 1.0;
	window->backgroundColor[3] = 0.0;

	// glfwSetWindowCloseCallback(window->self,close_callback);
	glfwSetKeyCallback(window->self, key_callback);
	glfwSetFramebufferSizeCallback(window->self, framebuffer_size_callback);
	glfwSetWindowSizeCallback(window->self, window_size_callback);
	glfwSetMouseButtonCallback(window->self, mouse_button_callback);
	glfwSetCursorPosCallback(window->self, cursor_pos_callback);
	glfwSetScrollCallback(window->self, scroll_callback);

	//glfwSetInputMode(window->self,GLFW_CURSOR,GLFW_CURSOR_HIDDEN);


	glfwSwapInterval(1); // vsync
	glClearColor(window->backgroundColor[0],
	             window->backgroundColor[1],
	             window->backgroundColor[2],
	             window->backgroundColor[3]);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE); // cull face that are not counterclockwise
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glPointSize(5); // 10 pixels for a point
	glEnable( GL_LINE_SMOOTH );

	glfwSetTime(DBL_MIN); // set the time to 0

	{
		double cursorX = 0.0, cursorY = 0.0;
		glfwGetCursorPos(window->self, &cursorX, &cursorY);
		window->cursorPos[0] = 2.0 * cursorX / window->size[0] - 1.0;
		window->cursorPos[1] = 2.0 * (1.0 - cursorY / window->size[1]) - 1.0;
		window->clickTime[0] = 0.0;
		window->clickTime[1] = 0.0;
	}

	window->counter = 0;
	window->running = 1;
	window->help_needed = 0;
	window->indication_needed = 0;
	window->leftClickCursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);

	window_OpenGL_init(window);

	window->help = bov_text_new((GLubyte[]) {
		" Keyboard shortcuts:\n"
		" -------------------\n\n"
		"   [esc]   exit\n"
		"  [space]  play/pause\n"
		"     p     save .ppm screenshot\n"
		"     r     reset zoom and translation\n"
		"    h k    display/hide keyboard shortcuts\n"
	}, GL_STATIC_DRAW);
	bov_text_set_space_type(window->help, PIXEL_SPACE);
	bov_text_set_fontsize(window->help, 32.0f); // 32 pixel height
	bov_text_set_pos(window->help, (GLfloat[2]){16.0f, 7.0f * 32.0f + 64.0f});
	bov_text_set_boldness(window->help, 0.1f);
	bov_text_set_outline_width(window->help, 0.5f);

	window->indication = bov_text_new((GLubyte[]) {
		"press 'k' for keyboard shortcuts\n"
	}, GL_STATIC_DRAW);
	bov_text_set_space_type(window->indication, PIXEL_SPACE);
	bov_text_set_fontsize(window->indication, 32.0f); // 32 pixel height
	bov_text_set_pos(window->indication, (GLfloat[2]){16.0f, 16.0f});
	bov_text_set_boldness(window->indication, 0.1f);
	bov_text_set_outline_width(window->indication, 0.5f);

	return window;
}


void bov_window_update(bov_window_t* window)
{
	if(window->help_needed)
		bov_text_draw(window, window->help);
	else if(window->indication_needed)
		bov_text_draw(window, window->indication);

	// Swap front and back buffers
	glfwSwapBuffers(window->self);

	if(window->running) {
		// Poll for and process events
		glfwPollEvents();
		window->wtime = glfwGetTime();
	}
	else {
		glfwWaitEvents();
		glfwSetTime(window->wtime);
	}

	// update the world ubo (we update every frame, even if it isn't changed...)
	glBindBuffer(GL_UNIFORM_BUFFER, window->ubo[0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(bov_world_param_t), &window->param);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glClearColor(window->backgroundColor[0],
	             window->backgroundColor[1],
	             window->backgroundColor[2],
	             window->backgroundColor[3]);
	glClear(GL_COLOR_BUFFER_BIT);
}


void bov_window_update_and_wait_events(bov_window_t* window)
{
	int state = window->running;
	window->running = 0;
	bov_window_update(window);
	window->running = state;
}


void bov_window_delete(bov_window_t* window)
{
	bov_text_delete(window->help);
	bov_text_delete(window->indication);
	glDeleteTextures(1, &window->font_atlas_texture);
	glDeleteProgram(window->program[TEXT_PROGRAM_INDEX]);
	glDeleteProgram(window->program[POINTS_PROGRAM_INDEX]);
	glDeleteProgram(window->program[LINES_PROGRAM_INDEX]);
	glDeleteProgram(window->program[CURVE_PROGRAM_INDEX]);
	glDeleteProgram(window->program[DEFAULT_PROGRAM_INDEX]);
	glDeleteBuffers(2, window->ubo);
	glfwDestroyWindow(window->self);
	glfwTerminate();
	free(window);
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Order object
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
bov_order_t* bov_order_new(const GLuint* elements, GLsizei n, GLenum usage)
{
	bov_order_t* order = malloc(sizeof(bov_order_t));
	CHECK_MALLOC(order);

	order->eboLen = elements==NULL ? 0 : n;

	glGenBuffers(1, &order->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, order->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, n * sizeof(GLuint), elements, usage);
	order->eboCapacity = n;
	return order;
}


bov_order_t* bov_order_update(bov_order_t* order, const GLuint* elements, GLsizei n)
{
	order->eboLen = elements==NULL ? 0 : n;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, order->ebo);
	if(order->eboLen > order->eboCapacity) {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, n * sizeof(GLuint), elements,
		             GL_DYNAMIC_DRAW);
		order->eboCapacity = order->eboLen;
	}
	else if(elements!=NULL){
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, n * sizeof(GLuint), elements);
	}

	return order;
}


bov_order_t* bov_order_partial_update(bov_order_t* order, const GLuint* elements,
                              GLint start, GLsizei count, GLsizei newN)
{
	if(elements==NULL) {
		BOV_ERROR_LOG(BOV_PARAMETER_ERROR,
		              "Cannot do a partial update whith a NULL "
		              "pointer as array of elements");
		return NULL;
	}

	if(newN==0)
		newN = order->eboLen;

	if(start + count > newN)
		newN = start+count;
	if(start + count < count) // detect overflow
		newN = BOV_TILL_END;

	if(newN > order->eboCapacity) {
		BOV_ERROR_LOG(BOV_PARAMETER_ERROR,
		              "Cannot do a partial update when the new "
		              "size is bigger than the capacity of the "
		              "buffer");
		return NULL;
	}

	// if it is restored to a previous capacity, fill buffer with zeros
	// if(newN > order->eboLen) {
	//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, order->ebo);
	//     GLint offset = (GLint) order->eboLen;
	//     GLsizei len = newN - order->eboLen;
	//     GLuint* zeros = calloc(len, sizeof(GLuint));
	//     CHECK_MALLOC(zeros);
	//     glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sizeof(GLuint) * len, zeros);
	//     free(zeros);
	// }

	order->eboLen = newN;

	if(count==0)
		return order;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, order->ebo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
	                start,
	                sizeof(GLuint) * count,
	                elements);
	return order;
}


void bov_order_delete(bov_order_t* order)
{
	glDeleteBuffers(1, &order->ebo);
	free(order);
}


// given n pixels between 0 and 1, and 2 points a and b in between,
// round a and b so that they are in the middle of a pixel
// and return the scaling between the old b-a distance and the new one
static double_t getMiddlePixelTex(GLfloat* a, GLfloat* b, double n)
{
	double aNew = (round((*a) * n) + 0.5) / n;
	double bNew = (round((*b) * n) - 0.5) / n;
	double ratio = (bNew - aNew) / (*b - *a);
	*a = (GLfloat) aNew;
	*b = (GLfloat) bNew;
	return ratio;
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Text Object
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
static GLsizei fill_text_data(GLfloat* data,
                              const GLubyte* string,
                              GLsizei len)
{
	double pen_x = 0.5; // position in pixel
	double pen_y = 0.5;
	double x,y,w,h;

	GLsizei num = 0;
	for(GLsizei i=0; i<len; i++) {
		const texture_glyph_t *glyph = font.glyphs + string[i];

		switch(glyph->codepoint) {
			// up, semi-up, semi-down...  x01 - 06, e1 - e6, c0 -df, f0 - ff  could mean something
			case '\f':  // back to the beginning
				pen_y = 0;
				/* FALLTHRU */
			case '\n':  // newline
			case '\r':  // carriage return
				pen_x = 0;
				/* FALLTHRU */
			case ' ':   // space
			case '\a':  // goes up :-)
			case '\v':  // vertical space
			case '\b':  // back the size of a space
				break;
			case '\t':  // tab
				pen_x = ((int) (pen_x/font.size + 2) / 2) * 2 * font.size;
				break;
			default:
				x = (pen_x + glyph->offset_x)/font.size;
				y = (pen_y + glyph->offset_y)/font.size;

				GLfloat s0 = glyph->s0, s1 = glyph->s1;
				GLfloat t0 = glyph->t0, t1 = glyph->t1;

				double rx = getMiddlePixelTex(&s0, &s1, font.tex_width);
				double ry = getMiddlePixelTex(&t0, &t1, font.tex_height);

				w = glyph->width*rx/font.size;
				h = glyph->height*ry/font.size;

				data[24*num + 0] = (GLfloat) x;
				data[24*num + 1] = (GLfloat) y;
				data[24*num + 2] = s0;
				data[24*num + 3] = t0;
				data[24*num + 4] = (GLfloat) x;
				data[24*num + 5] = (GLfloat) (y - h);
				data[24*num + 6] = s0;
				data[24*num + 7] = t1;
				data[24*num + 8] = (GLfloat) (x + w);
				data[24*num + 9] = (GLfloat) (y - h);
				data[24*num + 10] = s1;
				data[24*num + 11] = t1;
				data[24*num + 12] = (GLfloat) x;
				data[24*num + 13] = (GLfloat) y;
				data[24*num + 14] = s0;
				data[24*num + 15] = t0;
				data[24*num + 16] = (GLfloat) (x + w);
				data[24*num + 17] = (GLfloat) (y - h);
				data[24*num + 18] = s1;
				data[24*num + 19] = t1;
				data[24*num + 20] = (GLfloat) (x + w);
				data[24*num + 21] = (GLfloat) y;
				data[24*num + 22] = s1;
				data[24*num + 23] = t0;

				num++;
				break;

		}
		pen_x += glyph->advance_x;
		pen_y += glyph->advance_y;
	}

	return num;
}


// size (in characters) to alloc, and usage (either GL_STATIC_DRAW or GL_DYNAMIC_DRAW)
bov_text_t* bov_text_new(const GLubyte* string, GLenum usage)
{
	bov_text_t* text = malloc(sizeof(bov_text_t));
	CHECK_MALLOC(text);

	text->param = (bov_text_param_t) {
		.fillColor = {0.0f, 0.0f, 0.0f, 1.0f},
		.outlineColor = {1.0f ,1.0f, 1.0f, 2.0f},
		.pos = {0.0f, 0.0f},
		.shift = {0.0f, 0.0f},
		.fontSize = 0.05f,
		.boldness = 0.0f,
		.outlineWidth = -1.0f,
		.spaceType = USUAL_SPACE};

	// Create Vertex Array Object
	glGenVertexArrays(1, &text->vao);
	glBindVertexArray(text->vao);

	// Vertex Buffer Object
	glGenBuffers(1, &text->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, text->vbo);

	// specify the layout of the data
	glVertexAttribPointer(POS_LOCATION,
	                      2,
	                      GL_FLOAT,
	                      GL_FALSE,
	                      4 * sizeof(GLfloat),
	                      0);
	glVertexAttribPointer(TEX_LOCATION,
	                      2,
	                      GL_FLOAT,
	                      GL_FALSE,
	                      4 * sizeof(GLfloat),
	                      (void*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(POS_LOCATION);
	glEnableVertexAttribArray(TEX_LOCATION);

	if(string!=NULL) {
		text->dataCapacity = (GLsizei) strlen((const char *)string);

		// data for text contain... 4 vertex per letter
		// for each letter, we must have
		// 2 screen coordinates      4f
		// 2 texture coordinates     4f

		text->data = malloc(sizeof(GLfloat) * 24 * text->dataCapacity);
		CHECK_MALLOC(text->data);

		// text->string = string;

		text->vboLen = fill_text_data(text->data,
		                              string,
		                              text->dataCapacity);
		glBufferData(GL_ARRAY_BUFFER,
		             sizeof(GLfloat) * 24 * text->vboLen,
		             text->data,
		             usage);
		text->vboCapacity = text->vboLen;
	}
	else {
		text->vboLen = 0;
		text->vboCapacity = 0;
		text->data = NULL;
		text->dataCapacity = 0;
	}

	// glBindBuffer(GL_ARRAY_BUFFER, 0);
	// glBindVertexArray(0);

	return text;
}


void bov_text_delete(bov_text_t* text)
{
	glDeleteBuffers(1, &text->vbo);
	glDeleteVertexArrays(1, &text->vao);
	free(text->data);
	free(text);
}


bov_text_t* bov_text_update(bov_text_t* text, const GLubyte* string)
{
	// see if the length is not longer than the original string
	GLsizei newLen = (GLsizei) strlen((const char*)string);

	if(newLen > text->dataCapacity) {
		free(text->data);
		text->data = malloc(sizeof(GLfloat) * 24 * newLen);
		CHECK_MALLOC(text->data);
		text->dataCapacity = newLen;
	}


	// text->string = string;
	text->vboLen = fill_text_data(text->data, string, newLen);

	glBindBuffer(GL_ARRAY_BUFFER, text->vbo);
	if(text->vboLen > text->vboCapacity) {
		glBufferData(GL_ARRAY_BUFFER,
		             sizeof(GLfloat) * 24 * text->vboLen,
		             text->data,
		             GL_DYNAMIC_DRAW);
		text->vboCapacity = text->vboLen;
	}
	else{
		glBufferSubData(GL_ARRAY_BUFFER,
		                0,
		                sizeof(GLfloat) * 24 * text->vboLen,
		                text->data);
	}

	return text;
}


void bov_text_draw(bov_window_t* window, const bov_text_t* text)
{
	if(text->vboLen==0)
		return;

	if(window->last_program!=TEXT_PROGRAM_INDEX) {
		glUseProgram(window->program[TEXT_PROGRAM_INDEX]);
		window->last_program = TEXT_PROGRAM_INDEX;
	}

	// update the object ubo
	glBindBuffer(GL_UNIFORM_BUFFER, window->ubo[1]);
	glBufferSubData(GL_UNIFORM_BUFFER,
	                0,
	                sizeof(bov_text_param_t),
	                &text->param);
	// glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindVertexArray(text->vao);
	glDrawArrays(GL_TRIANGLES, 0, text->vboLen * 6);
	// glBindVertexArray(0);
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%
 %  Points Object
 %%%%%%%%%%%%%%%%%%%%%%%%%*/
bov_points_t* bov_points_new(const GLfloat coords[][2],
                             GLsizei n,
                             GLenum usage)
{
	bov_points_t* points = malloc(sizeof(bov_points_t));
	CHECK_MALLOC(points);

	points->vboLen = coords==NULL ? 0 : n;

	points->param = (bov_points_param_t) {
		.fillColor = {0.0f, 0.0f, 0.0f, 1.0f}, // color
		.outlineColor = {1.0f ,1.0f, 1.0f, 1.0f}, // outlineColor
		.pos = {0.0f ,0.0f},           // other
		.scale = {1.0f, 1.0f},           // localPos
		.width = 0.025f,
		.marker = 0.0f,
		.outlineWidth = -1.0f,
		.spaceType = USUAL_SPACE
	};

	// Create Vertex Array Object
	glGenVertexArrays(1, &points->vao);
	glBindVertexArray(points->vao);

	// Vertex Buffer Object
	glGenBuffers(1, &points->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points->vbo);

	// specify the layout of the data
	glVertexAttribPointer(POS_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(POS_LOCATION);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * n, coords, usage);
	points->vboCapacity = n;

	// glBindBuffer(GL_ARRAY_BUFFER, 0);
	// glBindVertexArray(0);

	return points;
}


bov_points_t* bov_points_update(bov_points_t* points,
                                const GLfloat coords[][2],
                                GLsizei n)
{
	points->vboLen = coords==NULL ? 0 : n;

	glBindBuffer(GL_ARRAY_BUFFER, points->vbo);
	if(n > points->vboCapacity) {
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * n, coords,
		             GL_DYNAMIC_DRAW);
		points->vboCapacity = n;
	}
	else if(coords!=NULL) {
		glBufferSubData(GL_ARRAY_BUFFER,
		                0,
		                sizeof(GLfloat) * 2 * n,
		                coords);
	}
	// glBindBuffer(GL_ARRAY_BUFFER, 0);

	return points;
}


bov_points_t* bov_points_partial_update(bov_points_t* points,
                                        const GLfloat coords[][2],
                                        GLint start,
                                        GLsizei count,
                                        GLsizei newN)
{
	if(coords==NULL) {
		BOV_ERROR_LOG(BOV_PARAMETER_ERROR,
		              "Cannot do a partial update with a NULL "
		              "pointer as array of coordinates");
		return NULL;
	}

	if(newN==0)
		newN = points->vboLen;

	if(start + count > newN)
		newN = start + count;
	if(start + count < count) // detect overflow
		newN = BOV_TILL_END;

	if(newN > points->vboCapacity) {
		BOV_ERROR_LOG(BOV_PARAMETER_ERROR,
		              "Cannot do a partial update when the new "
		              "size is bigger than the capacity of the "
		              "buffer");
		return NULL;
	}

	points->vboLen = newN;

	if(count==0)
		return points;

	glBindBuffer(GL_ARRAY_BUFFER, points->vbo);
	glBufferSubData(GL_ARRAY_BUFFER,
	                start,
	                sizeof(GLfloat) * 2 * count,
	                coords);
	// glBindBuffer(GL_ARRAY_BUFFER, 0);

	return points;
}


void bov_points_delete(bov_points_t* points)
{
	glDeleteBuffers(1, &points->vbo);
	glDeleteVertexArrays(1, &points->vao);
	free(points);
}



static GLenum switch_rasterizer_with_mode(bov_window_t* window,
                                          const bov_points_t* points,
                                          bov_points_drawing_mode_t mode)
{
	int program_index;
	GLenum primitive = 0;

	switch(mode) {
		case POINTS_PROGRAM:
			program_index = POINTS_PROGRAM_INDEX;
			primitive = GL_POINTS;
			break;
		case LINES_PROGRAM:
			program_index = LINES_PROGRAM_INDEX;
			primitive = GL_LINES;
			break;
		case LINE_LOOP_PROGRAM:
			program_index = LINES_PROGRAM_INDEX;
			primitive = GL_LINE_LOOP;
			break;
		case LINE_STRIP_PROGRAM:
			program_index = LINES_PROGRAM_INDEX;
			primitive = GL_LINE_STRIP;
			break;
		case CURVE_PROGRAM:
			program_index = CURVE_PROGRAM_INDEX;
			primitive = GL_LINE_STRIP_ADJACENCY;
			break;
		case TRIANGLES_PROGRAM:
			program_index = TRIANGLES_PROGRAM_INDEX;
			primitive = GL_TRIANGLES;
			break;
		case TRIANGLE_STRIP_PROGRAM:
			program_index = TRIANGLES_PROGRAM_INDEX;
			primitive = GL_TRIANGLE_STRIP;
			break;
		case TRIANGLE_FAN_PROGRAM:
			program_index = TRIANGLES_PROGRAM_INDEX;
			primitive = GL_TRIANGLE_FAN;
			break;
		// case QUADS_PROGRAM:
		// 	program_index = QUAD_PROGRAM_INDEX;
		// 	primitive = GL_LINES_ADJACENCY;
		// 	break;
		case FAST_POINTS_PROGRAM:
			program_index = DEFAULT_PROGRAM_INDEX;
			primitive = GL_POINTS;
			break;
		case FAST_LINES_PROGRAM:
			program_index = DEFAULT_PROGRAM_INDEX;
			primitive = GL_LINES;
			break;
		case FAST_LINE_LOOP_PROGRAM:
			program_index = DEFAULT_PROGRAM_INDEX;
			primitive = GL_LINE_LOOP;
			break;
		case FAST_LINE_STRIP_PROGRAM:
			program_index = DEFAULT_PROGRAM_INDEX;
			primitive = GL_LINE_STRIP;
			break;
		case FAST_TRIANGLES_PROGRAM:
			program_index = DEFAULT_PROGRAM_INDEX;
			primitive = GL_TRIANGLES;
			break;
		case FAST_TRIANGLE_STRIP_PROGRAM:
			program_index = DEFAULT_PROGRAM_INDEX;
			primitive = GL_TRIANGLE_STRIP;
			break;
		case FAST_TRIANGLE_FAN_PROGRAM:
			program_index = DEFAULT_PROGRAM_INDEX;
			primitive = GL_TRIANGLE_FAN;
			break;
		default:
			BOV_ERROR_LOG(BOV_PARAMETER_ERROR,
			              "this function is private but was "
			              "given erroneous arguments anyway");
			exit(EXIT_FAILURE);
	}

	if(window->last_program!=program_index) {
		glUseProgram(window->program[program_index]);
		window->last_program = program_index;
	}

	// update the object ubo
	glBindBuffer(GL_UNIFORM_BUFFER, window->ubo[1]);
	glBufferSubData(GL_UNIFORM_BUFFER,
	                0,
	                sizeof(bov_points_param_t),
	                &points->param);
	// glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindVertexArray(points->vao);

	return primitive;
}


void bov_points_draw_aux(bov_window_t* window,
                         const bov_points_t* points,
                         bov_points_drawing_mode_t mode,
                         GLint start,
                         GLsizei count)
{
	if(start>=points->vboLen)
		return;

	if(count + start > points->vboLen || start + count < count) { //< don't forget to detect overflow
		count = points->vboLen - start;
	}

	GLenum primitive = switch_rasterizer_with_mode(window, points, mode);
	glDrawArrays(primitive, start, count);
	// glBindVertexArray(0);
}


void bov_points_draw_with_order_aux(bov_window_t* window,
                                    const bov_points_t* points,
                                    bov_points_drawing_mode_t mode,
                                    const bov_order_t* order,
                                    GLint start, GLsizei count)
{
	if (order == NULL) {
		bov_points_draw_aux(window, points, mode, start, count);
		return;
	}

	if(points->vboLen==0 || start>=order->eboLen)
		return;

	if(count + start > order->eboLen || start + count < count) { //< don't forget to detect overflow
		count = order->eboLen - start;
	}

	GLenum primitive = switch_rasterizer_with_mode(window, points, mode);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, order->ebo);
	glDrawElements(primitive, count, GL_UNSIGNED_INT,
	               (void*) (start * sizeof(GLuint)));
	// glBindVertexArray(0);
}


// void bov_points_draw_with_indices_aux(bov_window_t* window,
//                                       const bov_points_t* points,
//                                       bov_points_drawing_mode_t mode,
//                                       const GLuint* indices,
//                                       GLint start, GLsizei count)
// {
// 	if(indices==NULL) {
// 		bov_points_draw_aux(window, points, mode, start, count);
// 		return;
// 	}

// 	if(points->vboLen==0 || start>=count)
// 		return;

// 	GLenum primitive = switch_rasterizer_with_mode(window, points, mode);

// 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // no vbo must be bound
// 	glDrawElements(primitive, count, GL_UNSIGNED_INT, indices+start);
// }


void bov_window_screenshot(const bov_window_t* window,
                           const char* filename)
{
	static unsigned char* data = NULL;

	if(window==NULL || filename==NULL) {
		free(data);
		return;
	}

	int width = (int) window->param.res[0];
	int height = (int) window->param.res[1];

	size_t rowsize = ((size_t) width * 3 + 3) / 4 * 4; // width of a row in byte
	void* newData = realloc(data, rowsize*height); // width is rounded to multiple of four
	CHECK_MALLOC(newData);
	data = newData; // just for MSVC to be happy

	FILE* pFile;

	pFile=fopen(filename, "wb");
	if(pFile==NULL)
		exit(1);

	glReadBuffer( GL_FRONT );
	glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data );

	// Write header
	fprintf(pFile, "P6\n%d %d\n255\n", width, height);

	// Write pixel data
	for(int i=height-1; i>=0; i--) {
		unsigned char *row = data + rowsize*i;
		fwrite(row, 1, (size_t) width*3, pFile);
	}

	// Close file
	fclose(pFile);
}