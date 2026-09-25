#version 450
#extension GL_GOOGLE_include_directive : require
#include "im2d_common.glsl"

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_tex0;
layout(location = 3) in vec2 in_tex1;

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_tex0;
layout(location = 2) out vec2 v_tex1;

void main()
{
	gl_Position = in_pos;
	gl_Position.xy = gl_Position.xy * pc.xform.xy + pc.xform.zw;
	gl_Position.w = in_pos.w;
	gl_Position.xyz *= gl_Position.w;
	v_color = in_color * pc.matColor;
	v_tex0 = in_tex0;
	v_tex1 = in_tex1;
}
