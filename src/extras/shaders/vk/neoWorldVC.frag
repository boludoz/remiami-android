#version 450
#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

// pc.custom: lightmap blend (rgb) and material alpha (a)
#define u_lightMap (pc.custom)

layout(set = 0, binding = 0) uniform sampler2D tex0;
layout(set = 2, binding = 0) uniform sampler2D tex1;	// lightmap (MatFX dual texture)

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_tex0;
layout(location = 2) in vec2 v_tex1;
layout(location = 3) in float v_fog;

layout(location = 0) out vec4 out_color;

void main()
{
	vec4 t0 = texture(tex0, v_tex0);
	vec4 t1 = texture(tex1, v_tex1);

	vec4 color;
	color = t0*v_color*(1.0 + u_lightMap*(t1-1.0));
	color.a = v_color.a*t0.a*u_lightMap.a;

	color.rgb = mix(u.fogColor.rgb, color.rgb, v_fog);
	DoAlphaTest(color.a);
	out_color = color;
}
