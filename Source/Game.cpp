#include "Game.h"

#include <d3dx12.h>
#include <dxcapi.h>

#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxcompiler")

Game::Game(HWND hWnd)
{
	mhWnd = hWnd;
}

Game::~Game()
{
	if (mDevice)
	{
		FlushCommandQueue();
	}
}

void Game::Init()
{
	//mAudio.Play();

#ifdef _DEBUG
	ID3D12Debug* debug;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
	debug->EnableDebugLayer();
#endif

	D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS fd;
	fd.Format = mBackBufferFormat;
	fd.SampleCount = SAMPLE_COUNT;
	fd.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

	mDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &fd, sizeof(fd));
	mNumQualityLevels = fd.NumQualityLevels;

	mRtvIncrementSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvIncrementSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavIncrementSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));

	D3D12_COMMAND_QUEUE_DESC cqd = {};
	cqd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	cqd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cqd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqd.NodeMask = 0;

	mDevice->CreateCommandQueue(&cqd, IID_PPV_ARGS(&mCommandQueue));

	mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator));
	
	mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator, NULL, IID_PPV_ARGS(&mCommandList));
	mCommandList->Close();

	CreateSwapChain();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = BACK_BUFFER_COUNT;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	
	mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;

	mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDsvHeap));

	Resize();
}

int Game::Run()
{
	MSG msg;
	return 0;
}

void Game::Resize()
{
	FlushCommandQueue();

	mCommandList->Reset(mCommandAllocator, NULL);

	for (int i = 0; i < BACK_BUFFER_COUNT; i++)
	{
		if (mBackBuffer[i])
		{
			mBackBuffer[i]->Release();
		}
	}
	if (mDepthStencilBuffer)
	{
		mDepthStencilBuffer->Release();
	}

	mSwapChain->ResizeBuffers(0, mWidth, mHeight, mBackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	mCurrBackBufferIndex = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < BACK_BUFFER_COUNT; i++)
	{
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffer[i]));
		mDevice->CreateRenderTargetView(mBackBuffer[i], NULL, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvIncrementSize);
	}

	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mWidth;
	depthStencilDesc.Height = mHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = mDepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = SAMPLE_COUNT;
	depthStencilDesc.SampleDesc.Quality = mNumQualityLevels - 1;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = mDepthStencilFormat;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	mDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&mDepthStencilBuffer));

	mDevice->CreateDepthStencilView(mDepthStencilBuffer, NULL, mDsvHeap->GetCPUDescriptorHandleForHeapStart());

	CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	mCommandList->ResourceBarrier(1, &resourceBarrier);

	mCommandList->Close();
	mCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&mCommandList);

	FlushCommandQueue();

	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width = static_cast<FLOAT>(mWidth);
	mViewport.Height = static_cast<FLOAT>(mHeight);
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	mScissorRect.left = 0;
	mScissorRect.top = 0;
	mScissorRect.right = static_cast<LONG>(mWidth);
	mScissorRect.bottom = static_cast<LONG>(mHeight);
}

void Game::CreateSwapChain()
{
	CreateDXGIFactory(IID_PPV_ARGS(&mFactory));

	DXGI_SWAP_CHAIN_DESC scd = {};
	scd.BufferDesc.Width = mWidth;
	scd.BufferDesc.Height = mHeight;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = mBackBufferFormat;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.SampleDesc.Count = SAMPLE_COUNT;
	scd.SampleDesc.Quality = mNumQualityLevels - 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = BACK_BUFFER_COUNT;
	scd.OutputWindow = mhWnd;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// DXGI_SWAP_EFFECT_FLIP_* swapchains do not support MSAA directly. You must always create a single-sample swapchain, then create your own MSAA render target which you explicitly resolve to the swapchain buffer.
	mFactory->CreateSwapChain(mCommandQueue, &scd, &mSwapChain);

	mFactory->MakeWindowAssociation(mhWnd, DXGI_MWA_NO_ALT_ENTER);
}

void Game::FlushCommandQueue()
{
	mCurrFence++;

	mCommandQueue->Signal(mFence, mCurrFence);

	if (mFence->GetCompletedValue() < mCurrFence)
	{
		HANDLE hEvent = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);

		mFence->SetEventOnCompletion(mCurrFence, hEvent);

		WaitForSingleObject(hEvent, INFINITE);

		CloseHandle(hEvent);
	}
}
