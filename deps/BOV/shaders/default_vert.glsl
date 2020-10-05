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

in vec2 pos;

void main() {
	float minRes = min(resolution.x, resolution.y);
	vec2 resRatio = minRes / resolution;

	vec2 scaling;
	vec2 translation;
	if(space_type==0) {
		// classical case
		scaling = resRatio * zoom;
		translation = resRatio * zoom * (localPos + translate);
	}
	else if(space_type==1) {
		scaling = resRatio;      // same as 0 but no zoom
		translation = resRatio * (localPos + zoom * translate); // same as 0
	}
	else /*if(space_type==2)*/{
		scaling = 2.0 / resolution;
		translation = localPos * scaling - 1.0;
	}
	gl_Position = vec4(scaling * pos + translation, 0.0, 1.0);
}