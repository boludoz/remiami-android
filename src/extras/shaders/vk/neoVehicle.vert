#version 450
#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

// custom[0].xyz eye, custom[1] = (fresnel, light strength),
// custom[2..6] specular light directions (w = power), custom[7..11] their colors
// pc.custom = (shininess, specularity)
#define u_eye (u.custom[0].xyz)
#define fresnel (u.custom[1].x)
#define lightStrength (u.custom[1].y)
#define shininess (pc.custom.x)
#define specularity (pc.custom.y)

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec2 in_tex0;

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec4 v_reflcolor;
layout(location = 2) out vec2 v_tex0;
layout(location = 3) out vec2 v_tex1;
layout(location = 4) out float v_fog;

vec3 DoDirLightSpec(vec3 Ldir, vec3 Lcol, vec3 N, vec3 V, float power)
{
	return pow(clamp(dot(N, normalize(V + -Ldir)), 0.0, 1.0), power)*Lcol;
}

void main()
{
	vec4 Vertex = u.world * vec4(in_pos, 1.0);
	gl_Position = u.mvp * vec4(in_pos, 1.0);
	gl_Position.y = -gl_Position.y;
	vec3 Normal = mat3(u.world) * in_normal;
	vec3 viewVec = normalize(u_eye - Vertex.xyz);

	v_tex0 = in_tex0;

	v_color = in_color;
	if(lightingOn > 0.5){
		v_color.rgb += u.ambient.rgb*surfAmbient;
		v_color.rgb += DoDynamicLight(Vertex.xyz, Normal)*surfDiffuse*lightStrength;
	}
	v_color = clamp(v_color, 0.0, 1.0);
	v_color *= pc.matColor;

	// reflect V along Normal
	vec3 uv2 = Normal*dot(viewVec, Normal)*2.0 - viewVec;
	v_tex1 = uv2.xy*0.5 + 0.5;
	float b = 1.0 - clamp(dot(viewVec, Normal), 0.0, 1.0);
	v_reflcolor = vec4(0.0, 0.0, 0.0, 1.0);
	v_reflcolor.a = mix(b*b*b*b*b, 1.0, fresnel)*shininess;

	for(int i = 0; i < 5; i++)
		v_reflcolor.rgb += DoDirLightSpec(u.custom[2+i].xyz, u.custom[7+i].rgb, Normal, viewVec, u.custom[2+i].w)*specularity*lightStrength;

	v_fog = DoFog(gl_Position.w);
}
