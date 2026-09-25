#version 450
#extension GL_GOOGLE_include_directive : require
#include "im2d_common.glsl"

layout(set = 0, binding = 0) uniform sampler2D tex0;	// drop mask
layout(set = 1, binding = 0) uniform sampler2D tex1;	// grabbed frame

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_tex0;
layout(location = 2) in vec2 v_tex1;

layout(location = 0) out vec4 out_color;

void main()
{
	vec4 color = v_color*texture(tex0, v_tex0);
	color *= texture(tex1, v_tex1);
	out_color = color;
}
