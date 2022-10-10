#pragma once

#include <d3d12.h>
#include <dxgi.h>

#include "Audio.h"
#include "Timer.h"

#define SAMPLE_COUNT 1
#define BACK_BUFFER_COUNT 2

class Game {
public:
	Game(HWND hWnd);
	virtual ~Game();
	void Init();
	int Run();
	void SetDimension(const int width, const int height);
	void Resize();
	void Toggle();

private:
	void CreateSwapChain();
	void FlushCommandQueue();
	void Fps();
	void Update(const double dt);
	void Draw(const double dt);

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
	ID3D12Resource* mBackBuffer[BACK_BUFFER_COUNT];
	ID3D12Resource* mDepthStencilBuffer;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

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

	bool mGamePaused = false;
};
