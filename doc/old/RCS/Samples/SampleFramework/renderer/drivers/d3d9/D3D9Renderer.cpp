/*
 * Copyright 2008-2012 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */

#include <RendererConfig.h>

#if defined(RENDERER_ENABLE_DIRECT3D9)

#include "D3D9Renderer.h"

#include <RendererDesc.h>

#include <RendererProjection.h>

#include <RendererVertexBufferDesc.h>
#include "D3D9RendererVertexBuffer.h"

#include <RendererIndexBufferDesc.h>
#include "D3D9RendererIndexBuffer.h"

#include <RendererInstanceBufferDesc.h>
#include "D3D9RendererInstanceBuffer.h"

#include <RendererMeshDesc.h>
#include <RendererMeshContext.h>
#include "D3D9RendererMesh.h"

#include <RendererMaterialDesc.h>
#include <RendererMaterialInstance.h>
#include "D3D9RendererMaterial.h"

#include <RendererLightDesc.h>
#include <RendererDirectionalLightDesc.h>
#include "D3D9RendererDirectionalLight.h"
#include <RendererSpotLightDesc.h>
#include "D3D9RendererSpotLight.h"

#include <RendererTexture2DDesc.h>
#include "D3D9RendererTexture2D.h"

#include <RendererTargetDesc.h>
#include "D3D9RendererTarget.h"

#include <SamplePlatform.h>

using namespace SampleRenderer;

void SampleRenderer::convertToD3D9(D3DCOLOR &dxcolor, const RendererColor &color)
{
	const float inv255 = 1.0f / 255.0f;
	dxcolor = D3DXCOLOR(color.r*inv255, color.g*inv255, color.b*inv255, color.a*inv255);
}

void SampleRenderer::convertToD3D9(float *dxvec, const PxVec3 &vec)
{
	dxvec[0] = vec.x;
	dxvec[1] = vec.y;
	dxvec[2] = vec.z;
}

void SampleRenderer::convertToD3D9(D3DMATRIX &dxmat, const physx::PxMat44 &mat)
{
	PxMat44 mat44 = mat.getTranspose();
	memcpy(&dxmat._11, mat44.front(), 4 * 4 * sizeof (float));
}

void SampleRenderer::convertToD3D9(D3DMATRIX &dxmat, const RendererProjection &mat)
{
	float temp[16];
	mat.getColumnMajor44(temp);
	for(PxU32 r=0; r<4; r++)
		for(PxU32 c=0; c<4; c++)
		{
			dxmat.m[r][c] = temp[c*4+r];
		}
}

/******************************
* D3D9Renderer::D3DXInterface *
******************************/ 

D3D9Renderer::D3DXInterface::D3DXInterface(void)
{
	memset(this, 0, sizeof(*this));
#if defined(RENDERER_WINDOWS)
#define D3DX_DLL "d3dx9_" RENDERER_TEXT2(D3DX_SDK_VERSION) ".dll"




	m_library = LoadLibraryA(D3DX_DLL);
	if(!m_library)
	{
		MessageBoxA(0, "Unable to load " D3DX_DLL ". Please install the latest DirectX End User Runtime available at www.microsoft.com/directx.", "Renderer Error.", MB_OK);
	}
	if(m_library)
	{
#define FIND_D3DX_FUNCTION(_name)                             \
m_##_name = (LP##_name)GetProcAddress(m_library, #_name); \
RENDERER_ASSERT(m_##_name, "Unable to find D3DX9 Function " #_name " in " D3DX_DLL ".");

		FIND_D3DX_FUNCTION(D3DXCompileShaderFromFileA)
		FIND_D3DX_FUNCTION(D3DXGetVertexShaderProfile)
		FIND_D3DX_FUNCTION(D3DXGetPixelShaderProfile)
		FIND_D3DX_FUNCTION(D3DXSaveSurfaceToFileA)

#undef FIND_D3DX_FUNCTION
	}

#define D3DCOMPILER_DLL "D3DCompiler_" RENDERER_TEXT2(D3DX_SDK_VERSION) ".dll"
	m_compiler_library = LoadLibraryA(D3DCOMPILER_DLL);
	if(!m_library)
	{
		MessageBoxA(0, "Unable to load " D3DCOMPILER_DLL ". Please install the latest DirectX End User Runtime available at www.microsoft.com/directx.", "Renderer Error.", MB_OK);
	}


#undef D3DX_DLL
#endif
}

D3D9Renderer::D3DXInterface::~D3DXInterface(void)
{
#if defined(RENDERER_WINDOWS)
	if(m_library)
	{
		FreeLibrary(m_library);
		m_library = 0;
		FreeLibrary(m_compiler_library);
		m_compiler_library = 0;
	}
#endif
}

#if defined(RENDERER_WINDOWS)
#define CALL_D3DX_FUNCTION(_name, _params)   if(m_##_name) result = m_##_name _params;
#else
#define CALL_D3DX_FUNCTION(_name, _params)   result = _name _params;
#endif

HRESULT D3D9Renderer::D3DXInterface::CompileShaderFromFileA(LPCSTR srcFile, CONST D3DXMACRO *defines, LPD3DXINCLUDE include, LPCSTR functionName, LPCSTR profile, DWORD flags, LPD3DXBUFFER *shader, LPD3DXBUFFER *errorMsgs, LPD3DXCONSTANTTABLE *constantTable)
{

	HRESULT result = D3DERR_NOTAVAILABLE;
	CALL_D3DX_FUNCTION(D3DXCompileShaderFromFileA, (srcFile, defines, include, functionName, profile, flags, shader, errorMsgs, constantTable));
	return result;

}

HRESULT D3D9Renderer::D3DXInterface::SaveSurfaceToFileA( LPCSTR pDestFile, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DSURFACE9 pSrcSurface, CONST PALETTEENTRY* pSrcPalette, CONST RECT*  pSrcRect)
{

	HRESULT result = D3DERR_NOTAVAILABLE;
	CALL_D3DX_FUNCTION(D3DXSaveSurfaceToFileA, (pDestFile, DestFormat, pSrcSurface, pSrcPalette, pSrcRect));
	return result;

}
LPCSTR D3D9Renderer::D3DXInterface::GetVertexShaderProfile(LPDIRECT3DDEVICE9 device)
{

	LPCSTR result = 0;
	CALL_D3DX_FUNCTION(D3DXGetVertexShaderProfile, (device));
	return result;

}

LPCSTR D3D9Renderer::D3DXInterface::GetPixelShaderProfile(LPDIRECT3DDEVICE9 device)
{
	LPCSTR result = 0;
	CALL_D3DX_FUNCTION(D3DXGetPixelShaderProfile, (device));
	return result;
}

#undef CALL_D3DX_FUNCTION

/**********************************
* D3D9Renderer::ShaderEnvironment *
**********************************/ 

D3D9Renderer::ShaderEnvironment::ShaderEnvironment(void)
{
	memset(this, 0, sizeof(*this));
}

/***************
* D3D9Renderer *
***************/ 

D3D9Renderer::D3D9Renderer(const RendererDesc &desc, const char* assetDir) :
Renderer	(DRIVER_DIRECT3D9, assetDir)
{
	m_textVDecl						= NULL;
	m_useShadersForTextRendering	= true;
	m_pixelCenterOffset      = 0.5f;
	m_displayWidth           = 0;
	m_displayHeight          = 0;
	m_d3d                    = 0;
	m_d3dDevice              = 0;
	m_d3dDepthStencilSurface = 0;

	m_viewMatrix				= PxMat44::createIdentity();

	SampleFramework::SamplePlatform* m_platform = SampleFramework::SamplePlatform::platform();
	m_d3d = static_cast<IDirect3D9*>(m_platform->initializeD3D9());
	RENDERER_ASSERT(m_d3d, "Could not create Direct3D9 Interface.");
	if(m_d3d)
	{
		memset(&m_d3dPresentParams, 0, sizeof(m_d3dPresentParams));
		m_d3dPresentParams.Windowed               = 1;
		m_d3dPresentParams.SwapEffect             = D3DSWAPEFFECT_DISCARD;
		m_d3dPresentParams.BackBufferFormat       = D3DFMT_X8R8G8B8;
		m_d3dPresentParams.EnableAutoDepthStencil = 0;
		m_d3dPresentParams.AutoDepthStencilFormat = D3DFMT_D24S8;
		m_d3dPresentParams.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE; // turns off V-sync;

		HRESULT res = m_platform->initializeD3D9Display(&m_d3dPresentParams, 
														m_deviceName, 
														m_displayWidth, 
														m_displayHeight, 
														&m_d3dDevice);
		RENDERER_ASSERT(res==D3D_OK, "Failed to create Direct3D9 Device.");
		if(res==D3D_OK)
		{
			checkResize(false);
			onDeviceReset();
		}
	}
}

D3D9Renderer::~D3D9Renderer(void)
{
	assert(!m_textVDecl);
	SampleFramework::SamplePlatform* m_platform = SampleFramework::SamplePlatform::platform();

	if(m_d3dDepthStencilSurface)
	{
		m_d3dDevice->SetDepthStencilSurface(NULL);
		m_platform->D3D9BlockUntilNotBusy(m_d3dDepthStencilSurface);
		m_d3dDepthStencilSurface->Release();
	}
	m_platform->D3D9DeviceBlockUntilIdle();
	if(m_d3dDevice)              m_d3dDevice->Release();
	if(m_d3d)                    m_d3d->Release();
}

bool D3D9Renderer::checkResize(bool isDeviceLost)
{
	bool isDeviceReset = false;
#if defined(RENDERER_WINDOWS)
	if(SampleFramework::SamplePlatform::platform()->getWindowHandle() && m_d3dDevice)
	{
		PxU32 width  = 0;
		PxU32 height = 0;
		SampleFramework::SamplePlatform::platform()->getWindowSize(width, height);
		if(width && height && (width != m_displayWidth || height != m_displayHeight) || isDeviceLost)
		{
			bool needsReset = (m_displayWidth&&m_displayHeight ? true : false);
			m_displayWidth  = width;
			m_displayHeight = height;

			D3DVIEWPORT9 viewport = {0};
			m_d3dDevice->GetViewport(&viewport);
			viewport.Width  = (DWORD)m_displayWidth;
			viewport.Height = (DWORD)m_displayHeight;
			viewport.MinZ =  0.0f;
			viewport.MaxZ =  1.0f;

			if(needsReset)
			{
				physx::PxU64 res = m_d3dDevice->TestCooperativeLevel();
				if(res == D3D_OK || res == D3DERR_DEVICENOTRESET)	//if device is lost, device has to be ready for reset
				{
					onDeviceLost();
					m_d3dPresentParams.BackBufferWidth  = (UINT)m_displayWidth;
					m_d3dPresentParams.BackBufferHeight = (UINT)m_displayHeight;
					res = m_d3dDevice->Reset(&m_d3dPresentParams);
					RENDERER_ASSERT(res == D3D_OK, "Failed to reset Direct3D9 Device.");
					if(res == D3D_OK)
					{
						isDeviceReset = true;
					}
					m_d3dDevice->SetViewport(&viewport);
					onDeviceReset();
				}
			}
			else
			{
				m_d3dDevice->SetViewport(&viewport);
			}
		}
	}
#endif
	return isDeviceReset;
}

void D3D9Renderer::onDeviceLost(void)
{
	notifyResourcesLostDevice();
	if(m_d3dDepthStencilSurface)
	{
		m_d3dDepthStencilSurface->Release();
		m_d3dDepthStencilSurface = 0;
	}
}

void D3D9Renderer::onDeviceReset(void)
{
	if(m_d3dDevice)
	{
		// set out initial states...
		m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
#if defined(RENDERER_WINDOWS)
		m_d3dDevice->SetRenderState(D3DRS_LIGHTING, 0);
#endif
		m_d3dDevice->SetRenderState(D3DRS_ZENABLE,  1);
		buildDepthStencilSurface();
	}
	notifyResourcesResetDevice();
}

void D3D9Renderer::buildDepthStencilSurface(void)
{
	if(m_d3dDevice)
	{
		PxU32 width  = m_displayWidth;
		PxU32 height = m_displayHeight;
		if(m_d3dDepthStencilSurface)
		{
			D3DSURFACE_DESC dssdesc;
			m_d3dDepthStencilSurface->GetDesc(&dssdesc);
			if(width != (PxU32)dssdesc.Width || height != (PxU32)dssdesc.Height)
			{
				m_d3dDepthStencilSurface->Release();
				m_d3dDepthStencilSurface = 0;
			}
		}
		if(!m_d3dDepthStencilSurface)
		{
			const D3DFORMAT           depthFormat        = D3DFMT_D24S8;
			const D3DMULTISAMPLE_TYPE multisampleType    = D3DMULTISAMPLE_NONE;
			const DWORD               multisampleQuality = 0;
			const BOOL                discard            = 0;
			IDirect3DSurface9 *depthSurface = 0;
			HRESULT result = m_d3dDevice->CreateDepthStencilSurface((UINT)width, (UINT)height, depthFormat, multisampleType, multisampleQuality, discard, &depthSurface, 0);
			RENDERER_ASSERT(result == D3D_OK, "Failed to create Direct3D9 DepthStencil Surface.");
			if(result == D3D_OK)
			{
				m_d3dDepthStencilSurface = depthSurface;
			}
		}
		m_d3dDevice->SetDepthStencilSurface(m_d3dDepthStencilSurface);
	}
}

// clears the offscreen buffers.
void D3D9Renderer::clearBuffers(void)
{
	if(m_d3dDevice)
	{
		const DWORD flags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL;
		m_d3dDevice->Clear(0, NULL, flags, D3DCOLOR_RGBA(getClearColor().r, getClearColor().g, getClearColor().b, getClearColor().a), 1.0f, 0);
	}
}

// presents the current color buffer to the screen.
// returns true on device reset and if buffers need to be rewritten.
bool D3D9Renderer::swapBuffers(void)
{
	bool isDeviceReset = false;
	if(m_d3dDevice)
	{
		HRESULT result = SampleFramework::SamplePlatform::platform()->D3D9Present();
		RENDERER_ASSERT(result == D3D_OK || result == D3DERR_DEVICELOST, "Unknown Direct3D9 error when swapping buffers.");
		if(result == D3D_OK || result == D3DERR_DEVICELOST)
		{
			isDeviceReset = checkResize(result == D3DERR_DEVICELOST);
		}
	}
	return isDeviceReset;
}

void D3D9Renderer::getWindowSize(PxU32 &width, PxU32 &height) const
{
	RENDERER_ASSERT(m_displayHeight * m_displayWidth > 0, "variables not initialized properly");
	width = m_displayWidth;
	height = m_displayHeight;
}

RendererVertexBuffer *D3D9Renderer::createVertexBuffer(const RendererVertexBufferDesc &desc)
{
	RENDERER_PERFZONE(D3D9Renderer_createVertexBuffer);
	D3D9RendererVertexBuffer *vb = 0;
	if(m_d3dDevice)
	{
		RENDERER_ASSERT(desc.isValid(), "Invalid Vertex Buffer Descriptor.");
		if(desc.isValid())
		{
			vb = new D3D9RendererVertexBuffer(*m_d3dDevice, desc, m_deferredVBUnlock);
		}
	}
	if(vb) addResource(*vb);
	return vb;
}

RendererIndexBuffer *D3D9Renderer::createIndexBuffer(const RendererIndexBufferDesc &desc)
{
	RENDERER_PERFZONE(D3D9Renderer_createIndexBuffer);
	D3D9RendererIndexBuffer *ib = 0;
	if(m_d3dDevice)
	{
		RENDERER_ASSERT(desc.isValid(), "Invalid Index Buffer Descriptor.");
		if(desc.isValid())
		{
			ib = new D3D9RendererIndexBuffer(*m_d3dDevice, desc);
		}
	}
	if(ib) addResource(*ib);
	return ib;
}

RendererInstanceBuffer *D3D9Renderer::createInstanceBuffer(const RendererInstanceBufferDesc &desc)
{
	RENDERER_PERFZONE(D3D9Renderer_createInstanceBuffer);
	D3D9RendererInstanceBuffer *ib = 0;
	if(m_d3dDevice)
	{
		RENDERER_ASSERT(desc.isValid(), "Invalid Instance Buffer Descriptor.");
		if(desc.isValid())
		{
			ib = new D3D9RendererInstanceBuffer(*m_d3dDevice, desc);
		}
	}
	if(ib) addResource(*ib);
	return ib;
}

RendererTexture2D *D3D9Renderer::createTexture2D(const RendererTexture2DDesc &desc)
{
	RENDERER_PERFZONE(D3D9Renderer_createTexture2D);
	D3D9RendererTexture2D *texture = 0;
	if(m_d3dDevice)
	{
		RENDERER_ASSERT(desc.isValid(), "Invalid Texture 2D Descriptor.");
		if(desc.isValid())
		{
			texture = new D3D9RendererTexture2D(*m_d3dDevice, desc);
		}
	}
	if(texture) addResource(*texture);
	return texture;
}

RendererTarget *D3D9Renderer::createTarget(const RendererTargetDesc &desc)
{
	RENDERER_PERFZONE(D3D9Renderer_createTarget);
	RendererTarget *target = 0;
#if defined(RENDERER_ENABLE_DIRECT3D9_TARGET)
	D3D9RendererTarget *d3dTarget = 0;
	RENDERER_ASSERT(desc.isValid(), "Invalid Target Descriptor.");
	if(desc.isValid())
	{
		d3dTarget = new D3D9RendererTarget(*m_d3dDevice, desc);
	}
	if(d3dTarget) addResource(*d3dTarget);
	target = d3dTarget;
#endif
	return target;
}

RendererMaterial *D3D9Renderer::createMaterial(const RendererMaterialDesc &desc)
{
	RendererMaterial *mat = hasMaterialAlready(desc);
	RENDERER_ASSERT(desc.isValid(), "Invalid Material Descriptor.");
	if(!mat && desc.isValid())
	{
		RENDERER_PERFZONE(D3D9Renderer_createMaterial);
		mat = new D3D9RendererMaterial(*this, desc);

		registerMaterial(desc, mat);
	}
	return mat;
}

RendererMesh *D3D9Renderer::createMesh(const RendererMeshDesc &desc)
{
	RENDERER_PERFZONE(D3D9Renderer_createMesh);
	D3D9RendererMesh *mesh = 0;
	RENDERER_ASSERT(desc.isValid(), "Invalid Mesh Descriptor.");
	if(desc.isValid())
	{
		mesh = new D3D9RendererMesh(*this, desc);
	}
	return mesh;
}

RendererLight *D3D9Renderer::createLight(const RendererLightDesc &desc)
{
	RENDERER_PERFZONE(D3D9Renderer_createLight);
	RendererLight *light = 0;
	if(m_d3dDevice)
	{
		RENDERER_ASSERT(desc.isValid(), "Invalid Light Descriptor.");
		if(desc.isValid())
		{
			switch(desc.type)
			{
			case RendererLight::TYPE_DIRECTIONAL:
				light = new D3D9RendererDirectionalLight(*this, *(RendererDirectionalLightDesc*)&desc);
				break;
			case RendererLight::TYPE_SPOT:
				light = new D3D9RendererSpotLight(*this, *(RendererSpotLightDesc*)&desc);
				break;
			default:
				RENDERER_ASSERT(0, "Not implemented!");
			}
		}
	}
	return light;
}

bool D3D9Renderer::beginRender(void)
{
	bool ok = false;
	if(m_d3dDevice)
	{
		ok = m_d3dDevice->BeginScene() == D3D_OK;
	}
	return ok;
}

void D3D9Renderer::endRender(void)
{
	if(m_d3dDevice)
	{
		m_d3dDevice->EndScene();
	}
}

void D3D9Renderer::bindViewProj(const physx::PxMat44 &eye, const RendererProjection &proj)
{
	m_viewMatrix = eye.inverseRT();
	convertToD3D9(m_environment.viewMatrix, m_viewMatrix);
	convertToD3D9(m_environment.projMatrix, proj);

	const PxVec3 eyePosition  =  eye.getPosition();
	const PxVec3 eyeDirection = -eye.getBasis(2);
	memcpy(m_environment.eyePosition,  &eyePosition.x,  sizeof(float)*3);
	memcpy(m_environment.eyeDirection, &eyeDirection.x, sizeof(float)*3);
}

void D3D9Renderer::bindFogState(const RendererColor &fogColor, float fogDistance)
{
	const float inv255 = 1.0f / 255.0f;

	m_environment.fogColorAndDistance[0] = fogColor.r*inv255;
	m_environment.fogColorAndDistance[1] = fogColor.g*inv255;
	m_environment.fogColorAndDistance[2] = fogColor.b*inv255;
	m_environment.fogColorAndDistance[3] = fogDistance;
}

void D3D9Renderer::bindAmbientState(const RendererColor &ambientColor)
{
	convertToD3D9(m_environment.ambientColor, ambientColor);
}

void D3D9Renderer::bindDeferredState(void)
{
	RENDERER_ASSERT(0, "Not implemented!");
}

void D3D9Renderer::bindMeshContext(const RendererMeshContext &context)
{
	physx::PxMat44 model;
	physx::PxMat44 modelView;
	if(context.transform) model = *context.transform;
	else                  model = PxMat44::createIdentity();
	modelView = m_viewMatrix * model;

	convertToD3D9(m_environment.modelMatrix,     model);
	convertToD3D9(m_environment.modelViewMatrix, modelView);

	// it appears that D3D winding is backwards, so reverse them...
	DWORD cullMode = D3DCULL_CCW;
	switch(context.cullMode)
	{
	case RendererMeshContext::CLOCKWISE: 
		cullMode = D3DCULL_CCW;
		break;
	case RendererMeshContext::COUNTER_CLOCKWISE: 
		cullMode = D3DCULL_CW;
		break;
	case RendererMeshContext::NONE: 
		cullMode = D3DCULL_NONE;
		break;
	default:
		RENDERER_ASSERT(0, "Invalid Cull Mode");
	}

	m_d3dDevice->SetRenderState(D3DRS_CULLMODE, cullMode);

	DWORD fillMode = D3DFILL_SOLID;
	switch(context.fillMode)
	{
	case RendererMeshContext::SOLID:
		fillMode = D3DFILL_SOLID;
		break;
	case RendererMeshContext::LINE:
		fillMode = D3DFILL_WIREFRAME;
		break;
	case RendererMeshContext::POINT:
		fillMode = D3DFILL_POINT;
		break;
	}
	m_d3dDevice->SetRenderState(D3DRS_FILLMODE, fillMode);

	RENDERER_ASSERT(context.numBones <= RENDERER_MAX_BONES, "Too many bones.");
	if(context.boneMatrices && context.numBones>0 && context.numBones <= RENDERER_MAX_BONES)
	{
		for(PxU32 i=0; i<context.numBones; i++)
		{
			convertToD3D9(m_environment.boneMatrices[i], context.boneMatrices[i]);
		}
		m_environment.numBones = context.numBones;
	}
}

void D3D9Renderer::beginMultiPass(void)
{
	if(m_d3dDevice)
	{
		m_d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,  1);
		m_d3dDevice->SetRenderState(D3DRS_SRCBLEND,          D3DBLEND_ONE);
		m_d3dDevice->SetRenderState(D3DRS_DESTBLEND,         D3DBLEND_ONE);
		m_d3dDevice->SetRenderState(D3DRS_ZFUNC,             D3DCMP_EQUAL);
	}
}

void D3D9Renderer::endMultiPass(void)
{
	if(m_d3dDevice)
	{
		m_d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,  0);
		m_d3dDevice->SetRenderState(D3DRS_ZFUNC,             D3DCMP_LESS);
	}
}

void D3D9Renderer::renderDeferredLight(const RendererLight &light)
{
	RENDERER_ASSERT(0, "Not implemented!");
}

PxU32 D3D9Renderer::convertColor(const RendererColor& color) const
{
	D3DCOLOR outColor;
	convertToD3D9(outColor, color);

	return outColor;
}

bool D3D9Renderer::isOk(void) const
{
	bool ok = true;
	if(!m_d3d)            ok = false;
	if(!m_d3dDevice)      ok = false;
#if defined(RENDERER_WINDOWS)
	ok = SampleFramework::SamplePlatform::platform()->isD3D9ok();
	if(!m_d3dx.m_library) ok = false;
	// note: we could assert m_compiler_library here too, but actually loading that one is optional, so the app still may work without it.
#endif
	return ok;
}

void D3D9Renderer::addResource(D3D9RendererResource &resource)
{
	RENDERER_ASSERT(resource.m_d3dRenderer==0, "Resource already in added to the Renderer!");
	if(resource.m_d3dRenderer==0)
	{
		resource.m_d3dRenderer = this;
		m_resources.push_back(&resource);
	}
}

void D3D9Renderer::removeResource(D3D9RendererResource &resource)
{
	RENDERER_ASSERT(resource.m_d3dRenderer==this, "Resource not part of this Renderer!");
	if(resource.m_d3dRenderer==this)
	{
		resource.m_d3dRenderer = 0;
		const PxU32 numResources  = (PxU32)m_resources.size();
		PxU32       foundResource = numResources;
		for(PxU32 i=0; i<numResources; i++)
		{
			if(m_resources[i] == &resource)
			{
				foundResource = i;
				break;
			}
		}
		if(foundResource < numResources)
		{
			m_resources[foundResource] = m_resources.back();
			m_resources.pop_back();
		}
	}
}

void D3D9Renderer::notifyResourcesLostDevice(void)
{
	const PxU32 numResources  = (PxU32)m_resources.size();
	for(PxU32 i=0; i<numResources; i++)
	{
		m_resources[i]->onDeviceLost();
	}
}

void D3D9Renderer::notifyResourcesResetDevice(void)
{
	const PxU32 numResources  = (PxU32)m_resources.size();
	for(PxU32 i=0; i<numResources; i++)
	{
		m_resources[i]->onDeviceReset();
	}
}


///////////////////////////////////////////////////////////////////////////////

static DWORD gCullMode;
static DWORD gAlphaBlendEnable;
static DWORD gSrcBlend;
static DWORD gDstBlend;
static DWORD gFillMode;
static DWORD gZWrite;

bool D3D9Renderer::initTexter()
{
	if(!Renderer::initTexter())
		return false;

	if(!m_textVDecl)
	{
		D3DVERTEXELEMENT9 vdecl[4];
		vdecl[0].Stream		= 0;
		vdecl[0].Offset		= 0;
		vdecl[0].Type		= D3DDECLTYPE_FLOAT4;
		vdecl[0].Method		= D3DDECLMETHOD_DEFAULT;
#if defined(RENDERER_XBOX360)
		vdecl[0].Usage		= D3DDECLUSAGE_POSITION;	// PT: D3DDECLUSAGE_POSITIONT is not available on Xbox
#endif
#if defined(RENDERER_WINDOWS)
		vdecl[0].Usage		= D3DDECLUSAGE_POSITIONT;
#endif
		vdecl[0].UsageIndex	= 0;

		vdecl[1].Stream		= 0;
		vdecl[1].Offset		= 4*4;
		vdecl[1].Type		= D3DDECLTYPE_D3DCOLOR;
		vdecl[1].Method		= D3DDECLMETHOD_DEFAULT;
		vdecl[1].Usage		= D3DDECLUSAGE_COLOR;
		vdecl[1].UsageIndex	= 0;

		vdecl[2].Stream		= 0;
		vdecl[2].Offset		= 4*4 + 4;
		vdecl[2].Type		= D3DDECLTYPE_FLOAT2;
		vdecl[2].Method		= D3DDECLMETHOD_DEFAULT;
		vdecl[2].Usage		= D3DDECLUSAGE_TEXCOORD;
		vdecl[2].UsageIndex	= 0;

		vdecl[3].Stream		= 0xFF;
		vdecl[3].Offset		= 0;
		vdecl[3].Type		= (DWORD)D3DDECLTYPE_UNUSED;
		vdecl[3].Method		= 0;
		vdecl[3].Usage		= 0;
		vdecl[3].UsageIndex	= 0;

		m_d3dDevice->CreateVertexDeclaration(vdecl, &m_textVDecl);
		if(!m_textVDecl)
		{
			closeTexter();
			return false;
		}
	}

	return true;
}

void D3D9Renderer::closeTexter()
{
	if(m_textVDecl)
	{
		m_textVDecl->Release();
		m_textVDecl = NULL;
	}

	Renderer::closeTexter();
}


void D3D9Renderer::setupTextRenderStates()
{
	// PT: save render states. Let's just hope this isn't a pure device, else the Get method won't work
	m_d3dDevice->GetRenderState(D3DRS_CULLMODE, &gCullMode);
	m_d3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &gAlphaBlendEnable);	
	m_d3dDevice->GetRenderState(D3DRS_SRCBLEND, &gSrcBlend);
	m_d3dDevice->GetRenderState(D3DRS_DESTBLEND, &gDstBlend);
	m_d3dDevice->GetRenderState(D3DRS_FILLMODE, &gFillMode);
	m_d3dDevice->GetRenderState(D3DRS_ZWRITEENABLE, &gZWrite);

	// PT: setup render states for text rendering
	m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

#if defined(RENDERER_XBOX360)
	// PT: D3DDECLUSAGE_POSITIONT is not available on Xbox, this is the workaround
	m_d3dDevice->SetRenderState(D3DRS_VIEWPORTENABLE, false);
#endif
}

void D3D9Renderer::resetTextRenderStates()
{
	// PT: restore render states. We want text rendering not to interfere with existing render states.
	// For example the text should never be rendered in wireframe, even if the scene is.
	m_d3dDevice->SetRenderState(D3DRS_CULLMODE, gCullMode);
	m_d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, gAlphaBlendEnable);
	m_d3dDevice->SetRenderState(D3DRS_SRCBLEND, gSrcBlend);
	m_d3dDevice->SetRenderState(D3DRS_DESTBLEND, gDstBlend);
	m_d3dDevice->SetRenderState(D3DRS_FILLMODE, gFillMode);
	m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, gZWrite);
#if defined(RENDERER_XBOX360)
	m_d3dDevice->SetRenderState(D3DRS_VIEWPORTENABLE, true);
#endif
	m_d3dDevice->SetVertexDeclaration(NULL);
}

void D3D9Renderer::renderTextBuffer(const void* vertices, PxU32 nbVerts, const PxU16* indices, PxU32 nbIndices, RendererMaterial* material)
{
	// PT: font texture must have been selected prior to calling this function
	const int PrimCount = nbIndices/3;
	const int Stride = sizeof(TextVertex);

	if(m_textVDecl && FAILED(m_d3dDevice->SetVertexDeclaration(m_textVDecl)))
	{
		assert(0);
		return;
	}

	DWORD hr = m_d3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, nbVerts, PrimCount, indices, D3DFMT_INDEX16, vertices, Stride);
	if(FAILED(hr))
	{
		//		printf("Error!\n");
	}
}

void D3D9Renderer::renderLines2D(const void* vertices, PxU32 nbVerts)
{
	const int PrimCount = nbVerts-1;
	const int Stride = sizeof(TextVertex);

	if(m_textVDecl && FAILED(m_d3dDevice->SetVertexDeclaration(m_textVDecl)))
	{
		assert(0);
		return;
	}

	DWORD hr = m_d3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, PrimCount, vertices, Stride);
	if(FAILED(hr))
	{
		//		printf("Error!\n");
	}
}

void D3D9Renderer::setupScreenquadRenderStates()
{
	m_d3dDevice->GetRenderState(D3DRS_CULLMODE, &gCullMode);
	m_d3dDevice->GetRenderState(D3DRS_FILLMODE, &gFillMode);

	m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
#if defined(RENDERER_XBOX360)
	m_d3dDevice->SetRenderState(D3DRS_VIEWPORTENABLE, false);
#endif
}

void D3D9Renderer::resetScreenquadRenderStates()
{
	m_d3dDevice->SetRenderState(D3DRS_CULLMODE, gCullMode);
	m_d3dDevice->SetRenderState(D3DRS_FILLMODE, gFillMode);
	m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
#if defined(RENDERER_XBOX360)
	m_d3dDevice->SetRenderState(D3DRS_VIEWPORTENABLE, true);
#endif
}

bool D3D9Renderer::captureScreen(const char* filename)
{
#if defined(RENDERER_XBOX360)
	return false;
#endif
	if(!filename)
		return false;
	

	LPDIRECT3DSURFACE9 pSurface = NULL;

	HRESULT hr;
	bool bRet = false;
	do
	{
		hr = m_d3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurface );
		if(FAILED(hr))
			break;
				
		
		hr = m_d3dx.SaveSurfaceToFileA(filename, D3DXIFF_BMP, pSurface, NULL, NULL);
		if(FAILED(hr))
			break;
		
		bRet = true;
	}while(0);
	
	
	if(pSurface)
		pSurface->Release();
    
	return bRet;
}
#endif // #if defined(RENDERER_ENABLE_DIRECT3D9)
