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
	int space_type; // 0: normal sizes, 1: unzoomable, 2: unmodifable pixel size
};

in vec2 pRect;
flat in float lba;
flat in float pixelSize;

out vec4 outColor;

void main()
{
	vec2 v = vec2(pRect.x - clamp(pRect.x, 0.0f, lba), pRect.y);
	vec2 sdf = vec2(width, width - outlineWidth + step(outlineWidth, 0.0f)) -
	             mix(length(v), max(abs(v.x), abs(v.y)), fract(marker));

	vec2 alpha = smoothstep(-pixelSize, pixelSize, sdf);
	outColor = mix(outlineColor, fillColor, alpha.y); // at 0: completely outlineColor, at1: completely fillColor
	outColor.a *= alpha.x;
}