#include "common.h"

#ifdef RW_VULKAN
#include "main.h"
#include "RwHelper.h"
#include "Lights.h"
#include "Timecycle.h"
#include "FileMgr.h"
#include "Clock.h"
#include "Weather.h"
#include "TxdStore.h"
#include "Renderer.h"
#include "World.h"
#include "custompipes.h"

#ifdef EXTENDED_PIPELINES

#ifndef LIBRW
#error "Need librw for EXTENDED_PIPELINES"
#endif

// Vulkan versions of the Neo pipelines in custompipes_gl.cpp. The shaders are
// in shaders/vk and are compiled to SPIR-V by shaders/vk/make_spirv.sh.

namespace CustomPipes {

using namespace rw::vulkan;

#include "shaders/obj/vk_neoVehicle_vert_spv.inc"
#include "shaders/obj/vk_neoVehicle_frag_spv.inc"
#include "shaders/obj/vk_neoRim_vert_spv.inc"
#include "shaders/obj/vk_simple_frag_spv.inc"
#include "shaders/obj/vk_neoGloss_vert_spv.inc"
#include "shaders/obj/vk_neoGloss_frag_spv.inc"

static void
setVec4(float *dst, float x, float y, float z, float w)
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
	dst[3] = w;
}

/*
 * Neo Vehicle pipe
 */

static CustomShader *neoVehicleShader;

static void
vehicleRenderCB(rw::Atomic *atomic)
{
	using namespace rw;

	// TODO: make this less of a kludge
	if(VehiclePipeSwitch == VEHICLEPIPE_MATFX){
		matFXGlobals.pipelines[rw::platform]->render(atomic);
		return;
	}

	// custom[0] eye, [1] fresnel and light strength,
	// [2..6] specular light directions (w = power), [7..11] their colors
	float custom[12][4];
	memset(custom, 0, sizeof(custom));
	V3d eyePos = rw::engine->currentCamera->getFrame()->getLTM()->pos;
	setVec4(custom[0], eyePos.x, eyePos.y, eyePos.z, 0.0f);
	setVec4(custom[1], Fresnel.Get(), SpecColor.Get().a, 0.0f, 0.0f);

	float power = Power.Get();
	Color speccol = SpecColor.Get();
	for(int i = 0; i < 1+NUMEXTRADIRECTIONALS; i++)
		custom[2+i][3] = 1.0f;
	V3d dir = pDirect->getFrame()->getLTM()->at;
	setVec4(custom[2], dir.x, dir.y, dir.z, power);
	setVec4(custom[7], speccol.r, speccol.g, speccol.b, 0.0f);
	for(int i = 0; i < NUMEXTRADIRECTIONALS; i++){
		if(pExtraDirectionals[i]->getFlags() & rw::Light::LIGHTATOMICS){
			dir = pExtraDirectionals[i]->getFrame()->getLTM()->at;
			setVec4(custom[3+i], dir.x, dir.y, dir.z, power*2.0f);
			RGBAf c = pExtraDirectionals[i]->color;
			setVec4(custom[8+i], c.red, c.green, c.blue, c.alpha);
		}
	}

	SetRenderState(SRCBLEND, BLENDONE);
	rw::vulkan::InstanceDataHeader *header = customBeginAtomic(atomic, neoVehicleShader, &custom[0][0], 12);
	if(header){
		customSetTexture1(EnvMapTex ? EnvMapTex->raster : nil);
		InstanceData *inst = header->inst;
		for(rw::uint32 n = 0; n < header->numMeshes; n++, inst++){
			Material *m = inst->material;
			float reflProps[4];
			reflProps[0] = m->surfaceProps.specular * VehicleShininess;
			reflProps[1] = m->surfaceProps.specular == 0.0f ? 0.0f : VehicleSpecularity;
			reflProps[2] = 0.0f;
			reflProps[3] = 0.0f;
			customDrawMesh(inst, reflProps);
		}
		customEndAtomic();
	}
	SetRenderState(SRCBLEND, BLENDSRCALPHA);
}

void
CreateVehiclePipe(void)
{
	if(CFileMgr::LoadFile("neo/carTweakingTable.dat", work_buff, sizeof(work_buff), "r") <= 0)
		printf("Error: couldn't open 'neo/carTweakingTable.dat'\n");
	else{
		char *fp = (char*)work_buff;
		fp = ReadTweakValueTable(fp, Fresnel);
		fp = ReadTweakValueTable(fp, Power);
		fp = ReadTweakValueTable(fp, DiffColor);
		fp = ReadTweakValueTable(fp, SpecColor);
	}

	neoVehicleShader = createCustomShader(vk_neoVehicle_vert_spv, sizeof(vk_neoVehicle_vert_spv),
		vk_neoVehicle_frag_spv, sizeof(vk_neoVehicle_frag_spv));
	assert(neoVehicleShader);

	rw::vulkan::ObjPipeline *pipe = rw::vulkan::ObjPipeline::create();
	pipe->renderCB = vehicleRenderCB;
	vehiclePipe = pipe;
}

void
DestroyVehiclePipe(void)
{
	destroyCustomShader(neoVehicleShader);
	neoVehicleShader = nil;

	((rw::vulkan::ObjPipeline*)vehiclePipe)->destroy();
	vehiclePipe = nil;
}

/*
 * Neo World pipe
 *
 * The lightmap shader needs the second texture coordinate set, which the
 * Vulkan backend doesn't instance yet, so the world is drawn normally.
 */

void
CreateWorldPipe(void)
{
	if(CFileMgr::LoadFile("neo/worldTweakingTable.dat", work_buff, sizeof(work_buff), "r") <= 0)
		printf("Error: couldn't open 'neo/worldTweakingTable.dat'\n");
	else
		ReadTweakValueTable((char*)work_buff, WorldLightmapBlend);

	rw::vulkan::ObjPipeline *pipe = rw::vulkan::ObjPipeline::create();
	pipe->renderCB = rw::vulkan::defaultRenderCB;
	worldPipe = pipe;
}

void
DestroyWorldPipe(void)
{
	((rw::vulkan::ObjPipeline*)worldPipe)->destroy();
	worldPipe = nil;
}

/*
 * Neo Gloss pipe
 */

static CustomShader *neoGlossShader;

static void
glossRenderCB(rw::Atomic *atomic)
{
	using namespace rw;

	rw::vulkan::defaultRenderCB(atomic);
	if(!GlossEnable)
		return;

	float custom[2][4];
	V3d eyePos = rw::engine->currentCamera->getFrame()->getLTM()->pos;
	setVec4(custom[0], eyePos.x, eyePos.y, eyePos.z, 0.0f);
	setVec4(custom[1], GlossMult, 0.0f, 0.0f, 0.0f);

	SetRenderState(VERTEXALPHA, TRUE);
	SetRenderState(SRCBLEND, BLENDONE);
	SetRenderState(DESTBLEND, BLENDONE);
	SetRenderState(ZWRITEENABLE, FALSE);
	SetRenderState(ALPHATESTFUNC, ALPHAALWAYS);

	rw::vulkan::InstanceDataHeader *header = customBeginAtomic(atomic, neoGlossShader, &custom[0][0], 2);
	if(header){
		InstanceData *inst = header->inst;
		for(rw::uint32 n = 0; n < header->numMeshes; n++, inst++){
			Material *m = inst->material;
			if(m->texture == nil)
				continue;
			Texture *tex = GetGlossTex(m);
			if(tex == nil)
				continue;
			// the gloss texture replaces the material's for this pass
			Texture *orig = m->texture;
			m->texture = tex;
			customDrawMesh(inst, nil);
			m->texture = orig;
		}
		customEndAtomic();
	}

	SetRenderState(ZWRITEENABLE, TRUE);
	SetRenderState(ALPHATESTFUNC, ALPHAGREATEREQUAL);
	SetRenderState(SRCBLEND, BLENDSRCALPHA);
	SetRenderState(DESTBLEND, BLENDINVSRCALPHA);
}

void
CreateGlossPipe(void)
{
	neoGlossShader = createCustomShader(vk_neoGloss_vert_spv, sizeof(vk_neoGloss_vert_spv),
		vk_neoGloss_frag_spv, sizeof(vk_neoGloss_frag_spv));
	assert(neoGlossShader);

	rw::vulkan::ObjPipeline *pipe = rw::vulkan::ObjPipeline::create();
	pipe->renderCB = glossRenderCB;
	glossPipe = pipe;
}

void
DestroyGlossPipe(void)
{
	destroyCustomShader(neoGlossShader);
	neoGlossShader = nil;

	((rw::vulkan::ObjPipeline*)glossPipe)->destroy();
	glossPipe = nil;
}

/*
 * Neo Rim pipes
 *
 * Skinned geometry is skinned on the CPU by librw, so peds use the same
 * shader as everything else.
 */

static CustomShader *neoRimShader;

static void
rimRender(rw::Atomic *atomic, void (*fallback)(rw::Atomic*))
{
	using namespace rw;

	if(!RimlightEnable){
		fallback(atomic);
		return;
	}

	// custom[0] view vector, [1] ramp start, [2] ramp end, [3] offset, scale, strength
	float custom[4][4];
	V3d viewVec = rw::engine->currentCamera->getFrame()->getLTM()->at;
	setVec4(custom[0], viewVec.x, viewVec.y, viewVec.z, 0.0f);
	Color col = RampStart.Get();
	setVec4(custom[1], col.r, col.g, col.b, col.a);
	col = RampEnd.Get();
	setVec4(custom[2], col.r, col.g, col.b, col.a);
	bool enable = (atomic->geometry->flags & Geometry::LIGHT) != 0;
	setVec4(custom[3], Offset.Get(), Scale.Get(), enable ? Scaling.Get()*RimlightMult : 0.0f, 0.0f);

	rw::vulkan::InstanceDataHeader *header = customBeginAtomic(atomic, neoRimShader, &custom[0][0], 4);
	if(header == nil)
		return;
	InstanceData *inst = header->inst;
	for(rw::uint32 n = 0; n < header->numMeshes; n++, inst++)
		customDrawMesh(inst, nil);
	customEndAtomic();
}

static void
rimRenderCB(rw::Atomic *atomic)
{
	rimRender(atomic, rw::vulkan::defaultRenderCB);
}

static void
rimSkinRenderCB(rw::Atomic *atomic)
{
	rimRender(atomic, rw::skinGlobals.pipelines[rw::PLATFORM_VULKAN] ?
		((rw::vulkan::ObjPipeline*)rw::skinGlobals.pipelines[rw::PLATFORM_VULKAN])->renderCB :
		rw::vulkan::defaultRenderCB);
}

void
CreateRimLightPipes(void)
{
	if(CFileMgr::LoadFile("neo/rimTweakingTable.dat", work_buff, sizeof(work_buff), "r") <= 0)
		printf("Error: couldn't open 'neo/rimTweakingTable.dat'\n");
	else{
		char *fp = (char*)work_buff;
		fp = ReadTweakValueTable(fp, RampStart);
		fp = ReadTweakValueTable(fp, RampEnd);
		fp = ReadTweakValueTable(fp, Offset);
		fp = ReadTweakValueTable(fp, Scale);
		fp = ReadTweakValueTable(fp, Scaling);
	}

	neoRimShader = createCustomShader(vk_neoRim_vert_spv, sizeof(vk_neoRim_vert_spv),
		vk_simple_frag_spv, sizeof(vk_simple_frag_spv));
	assert(neoRimShader);

	rw::vulkan::ObjPipeline *pipe = rw::vulkan::ObjPipeline::create();
	pipe->renderCB = rimRenderCB;
	rimPipe = pipe;

	pipe = rw::vulkan::ObjPipeline::create();
	pipe->renderCB = rimSkinRenderCB;
	rimSkinPipe = pipe;
}

void
DestroyRimLightPipes(void)
{
	destroyCustomShader(neoRimShader);
	neoRimShader = nil;

	((rw::vulkan::ObjPipeline*)rimPipe)->destroy();
	rimPipe = nil;
	((rw::vulkan::ObjPipeline*)rimSkinPipe)->destroy();
	rimSkinPipe = nil;
}

}

#endif
#endif
