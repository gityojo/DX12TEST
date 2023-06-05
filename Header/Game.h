#pragma once

#include "Audio.h"
#include "Maths.h"
#include "Struct.h"
#include "Timer.h"
#include "Upload.h"
#include "Util.h"
#include <d3d12.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>

#define SAMPLE_COUNT 1
#define BACK_BUFFER_COUNT 2

class Game
{
public:
	Game(HWND hWnd);
	virtual ~Game();
	void Init();
	int Run();
	float AspectRatio();
	void Resize();
	void SetDimension(const int width, const int height);
	void Toggle();

private:
	void CreateSwapChain();
	void FlushCommandQueue();
	void Fps();
	void Update();
	void Draw();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBox();
	void BuildPSO();
	ID3D12Resource* CurrBackBuffer();
	D3D12_CPU_DESCRIPTOR_HANDLE CurrRenderTargetDescriptor();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilDescriptor();
	D3D12_CPU_DESCRIPTOR_HANDLE ConstantBufferDescriptor();

	IDXGIFactory* mFactory;
	IDXGISwapChain* mSwapChain;

	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ID3D12Device* mDevice;
	ID3D12Fence* mFence;
	ID3D12CommandQueue* mCommandQueue;
	ID3D12CommandAllocator* mCommandAllocator;
	ID3D12GraphicsCommandList* mCommandList;
	ID3D12DescriptorHeap* mRtvHeap;
	ID3D12DescriptorHeap* mDsvHeap;
	ID3D12DescriptorHeap* mCbvHeap;
	ID3D12Resource* mBackBuffers[BACK_BUFFER_COUNT];
	ID3D12Resource* mDepthStencilBuffer;
	ID3D12RootSignature* mRootSignature;
	ID3D12PipelineState* mPipelineState = NULL;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputElementDescs;

	ID3DBlob* mVSByteCode;
	ID3DBlob* mPSByteCode;

	std::unique_ptr<Upload<ObjectConstant>> mObjectCB;
	std::unique_ptr<Mesh> mBox;
	std::unique_ptr<Mesh> mAmy;

	DirectX::XMFLOAT4X4 mWorld = Maths::Identity4x4();
	DirectX::XMFLOAT4X4 mView = Maths::Identity4x4();
	DirectX::XMFLOAT4X4 mProj = Maths::Identity4x4();

	HWND mhWnd;
	Audio mAudio;
	Timer mTimer;

	UINT64 mCurrFence = 0;

	UINT mWidth = 1920;
	UINT mHeight = 1080;
	UINT mNumQualityLevels = 0;
	UINT mRtvIncrementSize = 0;
	UINT mDsvIncrementSize = 0;
	UINT mCbvSrvUavIncrementSize = 0;
	UINT mCurrBackBufferIndex = 0;

	float mAngle = 0.0f;

	bool mGamePaused = false;
};
