#version 450
#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

// custom[0].xyz view vector, custom[1] ramp start, custom[2] ramp end,
// custom[3] = (offset, scale, strength)
#define u_viewVec (u.custom[0].xyz)
#define u_rampStart (u.custom[1])
#define u_rampEnd (u.custom[2])
#define u_rimData (u.custom[3])

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec2 in_tex0;

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_tex0;
layout(location = 2) out float v_fog;

void main()
{
	vec4 Vertex = u.world * vec4(in_pos, 1.0);
	gl_Position = u.mvp * vec4(in_pos, 1.0);
	gl_Position.y = -gl_Position.y;
	vec3 Normal = mat3(u.world) * in_normal;

	v_tex0 = in_tex0;

	v_color = in_color;
	if(lightingOn > 0.5){
		v_color.rgb += u.ambient.rgb*surfAmbient;
		v_color.rgb += DoDynamicLight(Vertex.xyz, Normal)*surfDiffuse;
	}

	// rim light
	float f = u_rimData.x - u_rimData.y*dot(Normal, u_viewVec);
	vec4 rimlight = clamp(mix(u_rampEnd, u_rampStart, f)*u_rimData.z, 0.0, 1.0);
	v_color.rgb += rimlight.rgb;

	v_color = clamp(v_color, 0.0, 1.0);
	v_color *= pc.matColor;

	v_fog = DoFog(gl_Position.w);
}
