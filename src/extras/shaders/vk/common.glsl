// Shared declarations for the Vulkan custom pipelines. The uniform block is
// librw's lit3d block (see rwvulkan.h, customBeginAtomic) followed by 16 vec4s
// each pipeline uses for its own values; push constants carry the material.

#define MAX_LIGHTS 8

layout(set = 1, binding = 0) uniform Lit3D {
	mat4 mvp;
	mat4 world;
	vec4 ambient;
	vec4 lightParams[MAX_LIGHTS];	// x=type, y=radius, z=minusCosAngle, w=hardSpot
	vec4 lightColor[MAX_LIGHTS];
	vec4 lightPosition[MAX_LIGHTS];
	vec4 lightDirection[MAX_LIGHTS];
	mat4 texMatrix;
	vec4 colorClamp;
	vec4 envColor;
	vec4 fxParams;
	vec4 fogData;	// x=start, y=end, z=range, w=disable
	vec4 fogColor;
	vec4 custom[16];
} u;

layout(push_constant) uniform Push {
	vec4 matColor;
	vec4 surfProps;	// x=ambient, y=diffuse, z=lighting on
	vec4 alphaRef;
	vec4 custom;
} pc;

#define surfAmbient (pc.surfProps.x)
#define surfDiffuse (pc.surfProps.y)
#define lightingOn (pc.surfProps.z)

vec3 DoDynamicLight(vec3 V, vec3 N)
{
	vec3 color = vec3(0.0);
	for(int i = 0; i < MAX_LIGHTS; i++){
		if(u.lightParams[i].x == 0.0)
			break;
		if(u.lightParams[i].x == 1.0){
			// directional
			float l = max(0.0, dot(N, -u.lightDirection[i].xyz));
			color += l*u.lightColor[i].rgb;
		}else if(u.lightParams[i].x == 2.0){
			// point
			vec3 dir = V - u.lightPosition[i].xyz;
			float dist = length(dir);
			float atten = max(0.0, 1.0 - dist/max(u.lightParams[i].y, 0.0001));
			float l = max(0.0, dot(N, -normalize(dir)));
			color += l*u.lightColor[i].rgb*atten;
		}else if(u.lightParams[i].x == 3.0){
			// spot
			vec3 dir = V - u.lightPosition[i].xyz;
			float dist = length(dir);
			float atten = max(0.0, 1.0 - dist/max(u.lightParams[i].y, 0.0001));
			dir /= max(dist, 0.0001);
			float l = max(0.0, dot(N, -dir));
			float pcos = dot(dir, u.lightDirection[i].xyz);
			float ccos = -u.lightParams[i].z;
			float falloff = (pcos - ccos)/max(1.0 - ccos, 0.0001);
			if(falloff < 0.0)
				l = 0.0;
			l *= max(falloff, u.lightParams[i].w);
			color += l*u.lightColor[i].rgb*atten;
		}
	}
	return color;
}

float DoFog(float w)
{
	return clamp((w - u.fogData.y)*u.fogData.z, u.fogData.w, 1.0);
}

// macro rather than a function: discard only exists in fragment shaders
#define DoAlphaTest(a) if((a) < pc.alphaRef.x || (a) >= pc.alphaRef.y) discard
