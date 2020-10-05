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

uniform sampler2D fontTex;

in vec2 texCoord;
out vec4 outColor;


void main()
{
	// vec2 dpdx = dFdx(texCoord);
	// vec2 dpdy = dFdy(texCoord);
	// vec4 texValue = textureGrad(fontTex, texCoord, dpdx, dpdy);
	vec4 texValue = texture(fontTex, texCoord);
	float sdf = texValue.r;
	vec2 normal = texValue.gb - 0.5f;

	// Outline
	float shift = outlineWidth;

	// we add the components of the outlineShift in the direction of the gradient.
	if(outlineShift.xy!=vec2(0.0f))
		shift += dot(outlineShift.xy, normal);

	float glyph_center = 0.5f - 0.25f * boldness;
	float outline_center = glyph_center + 0.25f * shift;

	float sdfWidth = fwidth(sdf); // length(dxdy);

	// glyph_center = min(glyph_center, outline_center);
	float opacity = smoothstep(-sdfWidth, sdfWidth, sdf - glyph_center); // ~2 pixels antialising
	float mu = smoothstep(-sdfWidth, sdfWidth, sdf - outline_center);
	outColor = mix(outlineColor, fillColor, mu); // at 0: completely outlineColor, at1: completely fillColor
	outColor.a *= opacity;
}