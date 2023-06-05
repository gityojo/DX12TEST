#pragma once

#include <d3d12.h>
#include <dxgi.h>
#include <string>

inline void d3dSetDebugName(IDXGIObject* obj, const char* name)
{
	if(obj)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}
inline void d3dSetDebugName(ID3D12Device* obj, const char* name)
{
	if(obj)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}
inline void d3dSetDebugName(ID3D12DeviceChild* obj, const char* name)
{
	if(obj)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

class Util
{
public:
	static bool IsKeyDown(int vkeyCode);

	static UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}

	// 1. Pass a pointer(type*) to the function and copy the data unsing e.g.memcpy.You should also pass the size of the destination array and make sure you don't copy too much data.
	// 2. Pass a reference to the pointer(type*&) and assign to the pointer in the function.
	// 3. Pass the address of the pointer(type**) and assign the address to the dereferenced pointer.
	static void CreateDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		ID3D12Resource*& uploadBuffer,
		ID3D12Resource*& defaultBuffer);

	static ID3DBlob* LoadBinary(const std::wstring& filename);

	static ID3DBlob* CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);
};
