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

#version 150 core

layout (std140) uniform objectBlock
{
	vec4 fillColor;
	vec4 outlineColor;
	vec2 localPos;
	vec2 outlineShift;
	float fontSize;
	float boldness;
	float outlineWidth;
	int space_type; // 0: normal sizes, 1: size in pixels, 2: size in pixels without translation
};

layout (std140) uniform worldBlock
{
	vec2 resolution;
	vec2 translate;
	float zoom;
	// float rotation;
};

// position of this vertex compared to
// the bottom-left of the left-most letter
in vec2 pos;

// position of this vertex in the font atlas texture
in vec2 tex;

// just forward text to the fragment shader
out vec2 texCoord;

void main()
{
	texCoord = tex;

	vec2 resRatio = min(resolution.x, resolution.y) / resolution;

	// note: conditional based on uniform should not slow the shader down
	vec2 vertexPos = pos * fontSize;
	if(space_type==0) {
		// classical case
		gl_Position = vec4(resRatio * zoom * (localPos + translate + vertexPos),
		                   0.0, 1.0);
	}
	else if(space_type==1) {
		// no scaling of the vertexPos is applied
		gl_Position = vec4(resRatio * (zoom * (localPos + translate) + vertexPos),
		                   0.0, 1.0);
	}
	else {
		// everything is given in pixel, from the bottom left corner :-)
		vec2 pixelPos = localPos + vertexPos;
		gl_Position = vec4(2.0*pixelPos / resolution - 1.0,
		                   0.0, 1.0);
	}
}
