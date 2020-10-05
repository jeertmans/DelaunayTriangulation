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
  * testing trigraph ??=
  * testing newline sequence etc \n \t \r \\
  * testing quotes "  coucou " "
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

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;

out vec2 pRect;
flat out float pixelSize;
flat out float lba;

vec2 perpendicular(vec2 v) {
	return vec2(-v.y, v.x);
}

void main()
{
	float minRes = min(resolution.x, resolution.y);
	vec2 resRatio = minRes / resolution;

	// screenPos = scaling*worldPos + translation

	// localScale should not affect the width and outlineWidth
	// therefore widthScaling!=scaling

	// pixelSize is simply 2.0/resolution / widthScaling
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

	vec2 p0=gl_in[0].gl_Position.xy * localScale;
	vec2 p1=gl_in[1].gl_Position.xy * localScale;
	vec2 p2=gl_in[2].gl_Position.xy * localScale;
	vec2 p3=gl_in[3].gl_Position.xy * localScale;

	vec2 ba = p2 - p1;
	lba = length(ba);
	vec2 v1 = ba / lba;
	vec2 v0, v2;
	float endWidth0, endWidth2;
	if(p0==p1) {
		v0 = v1;
		endWidth0 = w;
	}
	else {
		v0 = normalize(p1 - p0);
		endWidth0 = 0.0;
	}

	if(p2==p3) {
		v2 = v1;
		endWidth2 = w;
	}
	else {
		v2 = normalize(p3 - p2);
		endWidth2 = 0.0;
	}

	// determine the normal of each of the 3 segments (previous, current, next)
	vec2 n0 = perpendicular(v0);
	vec2 n1 = perpendicular(v1);
	vec2 n2 = perpendicular(v2);

	// determine miter lines by averaging the normals of the 2 segments
	vec2 miter0 = normalize(n0 + n1);    // miter at start of current segment
	vec2 miter2 = normalize(n1 + n2);    // miter at end of current segment

	// determine the length of the miter by projecting it onto normal and then inverse it
	float miter0_size = w  /  dot(miter0, n1);
	float miter2_size = w  /  dot(miter2, n1);

	float miter0v = miter0_size * dot(miter0, v1);
	float miter2v = miter2_size * dot(miter2, v1);

	vec2 p1_screen = scaling * p1 + translation;
	vec2 p2_screen = scaling * p2 + translation;
	vec2 miter0_screen = miter0_size * scaling * miter0;
	vec2 miter2_screen = miter2_size * scaling * miter2;

	vec2 q0 = p1_screen + miter0_screen - endWidth0 * scaling * v1;
	vec2 q1 = p1_screen - miter0_screen - endWidth0 * scaling * v1;
	vec2 q2 = p2_screen + miter2_screen + endWidth2 * scaling * v1;
	vec2 q3 = p2_screen - miter2_screen + endWidth2 * scaling * v1;

	vec2 miter2_perpend = perpendicular(miter2_screen);

	// orientation of triangle q0-q2-q3
	float ori = dot(q0 - q3, miter2_perpend);

	// the intersection of (q0, q1) and (q2, q3):   coordinates are given by q0 + inter * (q1-q0)
	float inter = 0.5 * ori / dot(miter0_screen, miter2_perpend);


	gl_Position.zw = vec2(0.0, 1.0);

	// avoid crossing lines
	if(inter>0 && inter<1) {
		pRect = (1.0 - 2.0 * inter) * vec2(miter0v - endWidth0, w);
		gl_Position.xy = inter * (q1-q0)+q0;
		EmitVertex();

		if(ori<0) {
			// the upper part is inverted
			pRect = vec2(-miter0v - endWidth0, -w);
			gl_Position.xy = q1;
			EmitVertex();

			pRect = vec2(lba - miter2v + endWidth2, -w);
			gl_Position.xy = q3;
			EmitVertex();
		}
		else {
			// the lower part is inverted
			pRect = vec2(lba + miter2v + endWidth2, w);
			gl_Position.xy = q2;
			EmitVertex();

			pRect = vec2(miter0v - endWidth0, w);
			gl_Position.xy = q0;
			EmitVertex();
		}
	}
	else {
		pRect = vec2(miter0v - endWidth0, w);
		gl_Position.xy = q0;
		EmitVertex();

		pRect = vec2(-miter0v - endWidth0, -w);
		gl_Position.xy = q1;
		EmitVertex();

		pRect = vec2(lba + miter2v + endWidth2, w);
		gl_Position.xy = q2;
		EmitVertex();

		pRect = vec2(lba - miter2v + endWidth2, -w);
		gl_Position.xy = q3;
		EmitVertex();
	}

	EndPrimitive();
}