#include "Struct.h"
#include "Upload.h"
#include "Util.h"
#include <d3dx12.h>

template<typename T>
Upload<T>::Upload(ID3D12Device* device, UINT elementCount, bool isConstantBuffer)
{
	mElementByteSize = sizeof(T);
	if (isConstantBuffer)
	{
		mElementByteSize = Util::CalcConstantBufferByteSize(sizeof(T));
	}

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(elementCount * mElementByteSize);

	device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL,
		IID_PPV_ARGS(&mUploadBuffer));

	mUploadBuffer->Map(0, NULL, (void**)&mMappedData);
}

template<typename T>
Upload<T>::~Upload()
{
	if (mUploadBuffer != NULL)
	{
		mUploadBuffer->Unmap(0, NULL);
	}

	mMappedData = NULL;
}

template<typename T>
ID3D12Resource* Upload<T>::Resource() const
{
	return mUploadBuffer;
}

template<typename T>
void Upload<T>::CopyData(int elementIndex, const T& data)
{
	memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
}

// 明示的なインスタンス化
template class Upload<Constant>;
