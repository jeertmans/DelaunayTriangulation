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

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

out vec2 pRect;
flat out float pixelSize;
flat out float lba;

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
		translation = localPos*scaling - 1.0;
		pixelSize = 1.0;
	}

	float w = width + pixelSize;

	vec2 a = gl_in[0].gl_Position.xy * localScale;
	vec2 b = gl_in[1].gl_Position.xy * localScale;

	vec2 ba = b - a;
	lba = length(ba);
	vec2 v = ba / lba;         // direction of this segment
	vec2 n = vec2(-v.y, v.x);// perpendicular direction

	vec2 aScreen = scaling * a + translation;
	vec2 bScreen = scaling * b + translation;
	vec2 wScreen = scaling * w * v;
	vec2 hScreen = scaling * w * n;

	pRect = vec2(-w);
	gl_Position = vec4(aScreen - wScreen - hScreen, 0.0, 1.0);
	EmitVertex();
	pRect = vec2(lba + w, -w);
	gl_Position = vec4(bScreen + wScreen - hScreen, 0.0, 1.0);
	EmitVertex();
	pRect = vec2(-w, w);
	gl_Position = vec4(aScreen - wScreen + hScreen, 0.0, 1.0);
	EmitVertex();
	pRect = vec2(lba + w, w);
	gl_Position = vec4(bScreen + wScreen + hScreen, 0.0, 1.0);
	EmitVertex();
	EndPrimitive();
}