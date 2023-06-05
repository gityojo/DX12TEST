#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT objectCount, UINT passCount, UINT materialCount)
{
	device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&CmdListAlloc));

	ObjectCB = std::make_unique<Upload<ObjectConstant>>(device, objectCount, true);
	PassCB = std::make_unique<Upload<PassConstant>>(device, passCount, true);
	MaterialCB = std::make_unique<Upload<MaterialConstant>>(device, materialCount, true);
}

FrameResource::~FrameResource()
{

}
