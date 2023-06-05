#pragma once

#include "Struct.h"
#include "Upload.h"

struct FrameResource
{
public:
	FrameResource(ID3D12Device* device, UINT objectCount, UINT passCount, UINT materialCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	ID3D12CommandAllocator* CmdListAlloc;

	std::unique_ptr<Upload<ObjectConstant>> ObjectCB;
	std::unique_ptr<Upload<PassConstant>> PassCB;
	std::unique_ptr<Upload<MaterialConstant>> MaterialCB;

	UINT64 Fence = 0;
};
