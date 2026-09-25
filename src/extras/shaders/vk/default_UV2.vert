#version 450
#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

// Lit like lit3d, passing both texture coordinate sets through (the world
// lightmap pipeline). Needs a shader created with twoTexCoords.

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec2 in_tex0;
layout(location = 4) in vec2 in_tex1;

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_tex0;
layout(location = 2) out vec2 v_tex1;
layout(location = 3) out float v_fog;

void main()
{
	vec4 Vertex = u.world * vec4(in_pos, 1.0);
	gl_Position = u.mvp * vec4(in_pos, 1.0);
	gl_Position.y = -gl_Position.y;
	vec3 Normal = mat3(u.world) * in_normal;

	v_tex0 = in_tex0;
	v_tex1 = in_tex1;

	v_color = in_color;
	if(lightingOn > 0.5){
		v_color.rgb += u.ambient.rgb*surfAmbient;
		v_color.rgb += DoDynamicLight(Vertex.xyz, Normal)*surfDiffuse;
	}
	v_color = clamp(v_color, 0.0, 1.0);
	v_color *= pc.matColor;

	v_fog = DoFog(gl_Position.w);
}
