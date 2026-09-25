#version 450
#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

// custom[1].x gloss multiplier
#define glossMult (u.custom[1].x)

layout(set = 0, binding = 0) uniform sampler2D tex0;

layout(location = 0) in vec3 v_normal;
layout(location = 1) in vec3 v_light;
layout(location = 2) in vec2 v_tex0;
layout(location = 3) in float v_fog;

layout(location = 0) out vec4 out_color;

void main()
{
	vec4 color = texture(tex0, v_tex0);
	vec3 n = 2.0*v_normal-1.0;	// unpack
	vec3 v = 2.0*v_light-1.0;

	float s = dot(n, v);
	color = s*s*s*s*s*s*s*s*color*v_fog*glossMult;

	DoAlphaTest(color.a);
	out_color = color;
}
