#include"inc/DXSample_ExecuteIndirect.h"

DXSample_ExecuteIndirect::DXSample_ExecuteIndirect(uint width, uint height, const char* appName)
	: DXSample(width, height, appName)
{

}

void DXSample_ExecuteIndirect::Awake()
{
	AcquireHardware();
	startPipeline();
	loadAssets();
}

void DXSample_ExecuteIndirect::Update(float delta)
{
}

void DXSample_ExecuteIndirect::Render(float delta)
{
}

void DXSample_ExecuteIndirect::Release()
{
}

void DXSample_ExecuteIndirect::startPipeline()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};

	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = FRAME_COUNT;

	Throw(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));

	mRtvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (uint i = 0; i < FRAME_COUNT; i++)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = MakeCPUDescriptorHandle(mRtvHeap.Get(), i, mRtvDescSize);

		Throw(mSwapchain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])));

		mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);
	}

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	D3D12_RESOURCE_DESC depthStencilDesc{};

	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.NumDescriptors = 1;

	mDsvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	Throw(mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDsvHeap)));
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.Width = mWidth;
	depthStencilDesc.Height = mHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES dsvHeapProps{};
	dsvHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	dsvHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.DepthStencil.Stencil = 0;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	Throw(mDevice->CreateCommittedResource(&dsvHeapProps, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&mDepthStencil)));

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = MakeCPUDescriptorHandle(mDsvHeap.Get(), 0, mDsvDescSize);

	mDevice->CreateDepthStencilView(mDepthStencil.Get(), nullptr, dsvHandle);

	Throw(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence.Fence)));

	D3D12_RESOURCE_DESC cbufferDesc{};
	D3D12_DESCRIPTOR_HEAP_DESC resourceHeapDesc{};

	resourceHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	resourceHeapDesc.NumDescriptors = FRAME_COUNT * RESOURCES_PER_FRAME;
	resourceHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	D3D12_HEAP_PROPERTIES resourceHeapProps{};

	resourceHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	resourceHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	resourceHeapProps.VisibleNodeMask = D3D12_SHADER_VISIBILITY_VERTEX;
	
	Throw(mDevice->CreateDescriptorHeap(&resourceHeapDesc, IID_PPV_ARGS(&mResourceHeap)));

	cbufferDesc.MipLevels = 1;
	cbufferDesc.DepthOrArraySize = 1;
	cbufferDesc.Width = sizeof(Constants);
	cbufferDesc.Height = 1;
	cbufferDesc.MipLevels = 1;
	cbufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	cbufferDesc.SampleDesc.Count = 1;
	cbufferDesc.Format = DXGI_FORMAT_UNKNOWN;

	Throw(mDevice->CreateCommittedResource(&resourceHeapProps, D3D12_HEAP_FLAG_NONE, &cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mCbuffer)));



}

void DXSample_ExecuteIndirect::loadAssets()
{
}
