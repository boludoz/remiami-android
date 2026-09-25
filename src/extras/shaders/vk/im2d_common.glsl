// Push constants of librw's Vulkan im2d custom shaders (setIm2DCustomShader)
layout(push_constant) uniform Push {
	vec4 xform;
	vec4 matColor;
	vec4 alphaRef;
	vec4 custom[2];
} pc;
