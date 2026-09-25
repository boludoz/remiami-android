#version 450
#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

layout(set = 0, binding = 0) uniform sampler2D tex0;
layout(set = 2, binding = 0) uniform sampler2D tex1;	// environment map

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec4 v_reflcolor;
layout(location = 2) in vec2 v_tex0;
layout(location = 3) in vec2 v_tex1;
layout(location = 4) in float v_fog;

layout(location = 0) out vec4 out_color;

void main()
{
	// Vulkan textures and render targets are both top-down, so unlike the
	// GL shader neither lookup flips v.
	vec4 pass1 = v_color*texture(tex0, v_tex0);
	vec3 envmap = texture(tex1, v_tex1).rgb;
	pass1.rgb = mix(pass1.rgb, envmap, v_reflcolor.a);
	pass1.rgb = mix(u.fogColor.rgb, pass1.rgb, v_fog);

	vec3 pass2 = v_reflcolor.rgb * v_fog;

	vec4 color;
	color.rgb = pass1.rgb*pass1.a + pass2;
	color.a = pass1.a;

	DoAlphaTest(color.a);
	out_color = color;
}
