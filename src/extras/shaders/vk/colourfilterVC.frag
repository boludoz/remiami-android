#version 450
#extension GL_GOOGLE_include_directive : require
#include "im2d_common.glsl"

// custom[0]: blur color
#define u_blurcolor (pc.custom[0])

layout(set = 0, binding = 0) uniform sampler2D tex0;	// grabbed frame

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_tex0;

layout(location = 0) out vec4 out_color;

void main()
{
	float a = u_blurcolor.a;
	vec4 doublec = clamp(u_blurcolor*2.0, 0.0, 1.0);
	// the grabbed frame is top-down like the screen: no v flip as in GL
	vec4 dst = texture(tex0, v_tex0);
	vec4 prev = dst;
	for(int i = 0; i < 5; i++){
		vec4 tmp = dst*(1.0-a) + prev*doublec*a;
		tmp += prev*u_blurcolor;
		tmp += prev*u_blurcolor;
		prev = clamp(tmp, 0.0, 1.0);
	}
	out_color = vec4(prev.rgb, 1.0);
}
