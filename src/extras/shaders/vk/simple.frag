#version 450
#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

layout(set = 0, binding = 0) uniform sampler2D tex0;

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_tex0;
layout(location = 2) in float v_fog;

layout(location = 0) out vec4 out_color;

void main()
{
	vec4 color = v_color*texture(tex0, v_tex0);
	color.rgb = mix(u.fogColor.rgb, color.rgb, v_fog);
	DoAlphaTest(color.a);
	out_color = color;
}
