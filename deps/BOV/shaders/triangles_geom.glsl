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
	vec2 localScale;
	float width;
	float marker;
	float outlineWidth;
	// float rotation;
	int space_type; // 0: normal sizes, 1: unzoomable, 2: unmodifable pixel size
};

layout (std140) uniform worldBlock
{
	vec2 resolution;
	vec2 translate;
	float zoom;
	// float rotation;
};

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;


out vec3 bary;
flat out float pixelSize;

void main()
{
	float minRes = min(resolution.x, resolution.y);
	vec2 resRatio = minRes / resolution;

	// screenPos = scaling*worldPos + translation

	// localScale should not affect the width and outlineWidth
	// therefore widthScaling!=scaling

	// pixelSize is simply 2.0/resolution / scaling
	vec2 scaling;
	vec2 translation;
	if(space_type==0) {
		// classical case
		scaling = resRatio * zoom;
		translation = resRatio * zoom * (localPos + translate);
		pixelSize = 2.0 / (minRes * zoom);
	}
	else if(space_type==1) {
		scaling = resRatio;      // same as 0 but no zoom
		translation = resRatio * (localPos + zoom * translate); // same as 0
		pixelSize = 2.0 / minRes;
	}
	else /*if(space_type==2)*/{
		scaling = 2.0 / resolution;
		translation = localPos * scaling - 1.0;
		pixelSize = 1.0;
	}

	float w = width + pixelSize;

	vec2 p0 = gl_in[0].gl_Position.xy * localScale;
	vec2 p1 = gl_in[1].gl_Position.xy * localScale;
	vec2 p2 = gl_in[2].gl_Position.xy * localScale;

	// direction of each segment
	vec2 d01 = normalize(p1 - p0);
	vec2 d12 = normalize(p2 - p1);
	vec2 d20 = normalize(p0 - p2);

	vec2 miter0 = normalize(d20 - d01);
	vec2 miter1 = normalize(d01 - d12);
	vec2 miter2 = normalize(d12 - d20);

	// determine the length of the miter by projecting it onto normal and then inverse it
	// the exterior normal is n01=vec2(d01.y, -d01.x), doing a dot product with the normal
	// is the same as a 2D determinant
	p0 += miter0 * w / determinant(mat2(miter0, d01));
	p1 += miter1 * w / determinant(mat2(miter1, d12));
	p2 += miter2 * w / determinant(mat2(miter2, d20));

	vec2 v01 = p1 - p0;
	vec2 v12 = p2 - p1;
	vec2 v20 = p0 - p2;

	// compute heights (simply crossprod/base...)
	float crossProd = v01.y*v20.x - v01.x*v20.y;
	
	bary = vec3(crossProd / length(v12), -pixelSize, -pixelSize);
	gl_Position = vec4(scaling * p0 + translation, 0.0, 1.0);
	EmitVertex();
	
	bary = vec3(-pixelSize, crossProd / length(v20), -pixelSize);
	gl_Position = vec4(scaling * p1 + translation, 0.0, 1.0);
	EmitVertex();
	
	bary = vec3(-pixelSize, -pixelSize, crossProd / length(v01));
	gl_Position = vec4(scaling * p2 + translation, 0.0, 1.0);
	EmitVertex();
	EndPrimitive();
}