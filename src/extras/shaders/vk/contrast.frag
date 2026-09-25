#version 450
#extension GL_GOOGLE_include_directive : require
#include "im2d_common.glsl"

// custom[0].rgb multiplier, custom[1].rgb addend
layout(set = 0, binding = 0) uniform sampler2D tex0;	// grabbed frame

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_tex0;

layout(location = 0) out vec4 out_color;

void main()
{
	vec4 dst = texture(tex0, v_tex0);
	out_color = vec4(dst.rgb*pc.custom[0].rgb + pc.custom[1].rgb, 1.0);
}
