#include "Game.h"
#include <d3dx12.h>
#include <dxcapi.h>
#include <DirectXColors.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxcompiler")

using namespace std;
using namespace DirectX;

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
	mAudio.Play();

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
	MSG msg = {};

	mTimer.Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (!mGamePaused)
			{
				mTimer.Tick();
				Fps();
				Update();
				Draw();
			}
		}
	}

	return (int)msg.wParam;
}

void Game::Resize()
{
	FlushCommandQueue();

	mCommandList->Reset(mCommandAllocator, NULL);

	for (int i = 0; i < BACK_BUFFER_COUNT; i++)
	{
		if (mBackBuffers[i])
		{
			mBackBuffers[i]->Release();
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
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i]));
		mDevice->CreateRenderTargetView(mBackBuffers[i], NULL, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvIncrementSize);
	}

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

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

	D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	mCommandList->ResourceBarrier(1, &resourceBarrier);

	mCommandList->Close();
	mCommandQueue->ExecuteCommandLists(1, CommandListCast<ID3D12GraphicsCommandList>(&mCommandList));

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

	mFactory->MakeWindowAssociation(mhWnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
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

void Game::Fps()
{
	static double totalSeconds = 0.0;
	static int totalFrames = 0;

	totalFrames++;

	if ((mTimer.TotalTime() - totalSeconds) >= 1.0)
	{
		SetWindowText(mhWnd, to_wstring(totalFrames).c_str());

		totalSeconds += 1.0;
		totalFrames = 0;
	}
}

void Game::Update()
{
}

void Game::Draw()
{
	mCommandAllocator->Reset();
	mCommandList->Reset(mCommandAllocator, NULL);

	D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mCommandList->ResourceBarrier(1, &resourceBarrier);

	mCommandList->RSSetViewports(1, &mViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	D3D12_CPU_DESCRIPTOR_HANDLE currRenderTargetDescriptor = CurrRenderTargetDescriptor();
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilDescriptor = DepthStencilDescriptor();

	mCommandList->ClearRenderTargetView(currRenderTargetDescriptor, Colors::LightBlue, 0, NULL);
	mCommandList->ClearDepthStencilView(depthStencilDescriptor, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	mCommandList->OMSetRenderTargets(1, &currRenderTargetDescriptor, TRUE, &depthStencilDescriptor);

	resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCommandList->ResourceBarrier(1, &resourceBarrier);

	mCommandList->Close();
	mCommandQueue->ExecuteCommandLists(1, CommandListCast<ID3D12GraphicsCommandList>(&mCommandList));

	mSwapChain->Present(1, 0);
	mCurrBackBufferIndex = (mCurrBackBufferIndex + 1) % BACK_BUFFER_COUNT;

	FlushCommandQueue();
}

ID3D12Resource* Game::CurrBackBuffer()
{
	return mBackBuffers[mCurrBackBufferIndex];
}

D3D12_CPU_DESCRIPTOR_HANDLE Game::CurrRenderTargetDescriptor()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mCurrBackBufferIndex, mRtvIncrementSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE Game::DepthStencilDescriptor()
{
	return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
}
