#include "inc/stdafx.h"
#include"inc/Window.h"
#include "inc/DXSample.h"

DXSample::DXSample(uint width, uint height, const char* name)
	: mWidth(width), mHeight(height), mAppName(name), mAspectRatio(static_cast<float>(width) / static_cast<float>(height))
{
}

DXSample::~DXSample()
{
}

void DXSample::AcquireHardware()
{
	

#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController;

	Throw(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));

	debugController->EnableDebugLayer();

	DWORD flag = DXGI_CREATE_FACTORY_DEBUG;
#else
	DWORD flag = 0;
#endif

	Throw(CreateDXGIFactory2(flag, IID_PPV_ARGS(&mFactory)));
	Throw(mFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&mAdapter)));
	
	Throw(D3D12CreateDevice(mAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice)));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};

	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	Throw(mDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&mCmdQueue)));

	HWND winHandle = Window::GetHwnd();

	DXGI_SWAP_CHAIN_DESC1 scDesc{};
	ComPtr<IDXGISwapChain1> swapchain;

	scDesc.BufferCount = BUFFER_COUNT;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Width = mWidth;
	scDesc.Height = mHeight;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.Scaling = DXGI_SCALING_STRETCH;
	scDesc.SampleDesc.Count = 1;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	Throw(mFactory->CreateSwapChainForHwnd(mCmdQueue.Get(), winHandle, &scDesc, nullptr, nullptr, swapchain.GetAddressOf()));
	Throw(swapchain.As<IDXGISwapChain4>(&mSwapchain));

	Throw(mFactory->MakeWindowAssociation(winHandle, DXGI_MWA_NO_ALT_ENTER));
}