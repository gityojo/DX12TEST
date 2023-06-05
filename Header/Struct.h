#pragma once

#include "Maths.h"
#include "Util.h"
#include <d3d12.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <string>
#include <unordered_map>

#define MaxLights 16

using namespace std;
using namespace DirectX;

extern const int gNumFrameResources;

struct Light
{
	XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
	float FalloffStart = 1.0f;                          // point/spot light only
	XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
	float FalloffEnd = 10.0f;                           // point/spot light only
	XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
	float SpotPower = 64.0f;                            // spot light only
};

struct Material
{
	// Unique material name for lookup.
	string Name;

	// Index into constant buffer corresponding to this material.
	int MatCBIndex = -1;

	// Index into SRV heap for diffuse texture.
	int DiffuseSrvHeapIndex = -1;

	// Index into SRV heap for normal texture.
	int NormalSrvHeapIndex = -1;

	// Dirty flag indicating the material has changed and we need to update the constant buffer.
	// Because we have a material constant buffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify a material we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Material constant buffer data used for shading.
	XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = .25f;
	XMFLOAT4X4 MatTransform = Maths::Identity4x4();
};

struct MaterialConstant
{
	XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;

	// Used in texture mapping.
	XMFLOAT4X4 MatTransform = Maths::Identity4x4();
};

struct Submesh
{
	UINT IndexCountPerInstance = 0;
	UINT InstanceCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;
	UINT StartInstanceLocation = 0;
};

struct Mesh
{
	string Name;

	ID3D12Resource* VertexBufferDefault;
	ID3D12Resource* IndexBufferDefault;

	ID3D12Resource* VertexBufferUpload;
	ID3D12Resource* IndexBufferUpload;

	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	unordered_map<string, Submesh> Submeshes;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferDefault->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferDefault->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	void DisposeUploaders()
	{
		VertexBufferUpload = NULL;
		IndexBufferUpload = NULL;
	}
};

struct ObjectConstant
{
	XMFLOAT4X4 WorldViewProj = Maths::Identity4x4();
};

struct PassConstant
{
	XMFLOAT4X4 View = Maths::Identity4x4();
	XMFLOAT4X4 InvView = Maths::Identity4x4();
	XMFLOAT4X4 Proj = Maths::Identity4x4();
	XMFLOAT4X4 InvProj = Maths::Identity4x4();
	XMFLOAT4X4 ViewProj = Maths::Identity4x4();
	XMFLOAT4X4 InvViewProj = Maths::Identity4x4();
	XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
	XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;

	XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light Lights[MaxLights];
};

struct RenderItem
{
	RenderItem() = default;

	XMFLOAT4X4 World = Maths::Identity4x4();

	XMFLOAT4X4 TexTransform = Maths::Identity4x4();

	int NumFramesDirty = gNumFrameResources;

	int ObjCBIndex = -1;

	Material* Mat = NULL;
	Mesh* Geo = NULL;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;
};

struct Texture
{
	string Name;
	wstring Filename;

	ID3D12Resource* Resource;
	ID3D12Resource* UploadHeap;
};

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};
