#version 450
#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

// custom[0].xyz eye
#define u_eye (u.custom[0].xyz)

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec2 in_tex0;

layout(location = 0) out vec3 v_normal;
layout(location = 1) out vec3 v_light;
layout(location = 2) out vec2 v_tex0;
layout(location = 3) out float v_fog;

void main()
{
	vec4 Vertex = u.world * vec4(in_pos, 1.0);
	gl_Position = u.mvp * vec4(in_pos, 1.0);
	gl_Position.y = -gl_Position.y;

	v_tex0 = in_tex0;

	vec3 viewVec = normalize(u_eye - Vertex.xyz);
	vec3 Light = normalize(viewVec - u.lightDirection[0].xyz);
	v_normal = 0.5*(1.0 + vec3(0.0, 0.0, 1.0));	// compress
	v_light  = 0.5*(1.0 + Light);

	v_fog = DoFog(gl_Position.w);
}
