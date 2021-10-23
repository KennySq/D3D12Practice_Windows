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
	D3D12_RESOURCE_BARRIER rtvBarrier;
	mCmdAllocator[0]->Reset();
	mCmdAllocator[1]->Reset();

	static ID3D12CommandList* const cmdLists[] = { mCmdList.Get()};

	Throw(mCmdList->Reset(mCmdAllocator[0].Get(), mPSO.Get()));
	Throw(mComputeCmdList->Reset(mCmdAllocator[1].Get(), mComputePSO.Get()));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = MakeCPUDescriptorHandle(mRtvHeap.Get(), mFrameIndex, mRtvDescSize);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = MakeCPUDescriptorHandle(mDsvHeap.Get(), 0, 0);

	rtvBarrier = MakeTransition(mRenderTargets[mFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mCmdList->ResourceBarrier(1, &rtvBarrier);

	mCmdList->ClearRenderTargetView(rtvHandle, Colors::Green, 0, nullptr);
	mCmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	rtvBarrier = MakeTransition(mRenderTargets[mFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCmdList->ResourceBarrier(1, &rtvBarrier);


	mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	mCmdList->RSSetScissorRects(1, &mScissorRect);
	mCmdList->RSSetViewports(1, &mViewport);

	
	Throw(mCmdList->Close());
	Throw(mComputeCmdList->Close());

	mCmdQueue->ExecuteCommandLists(1, cmdLists);

	

}

void DXSample_ExecuteIndirect::Render(float delta)
{
	Throw(mSwapchain->Present(1, 0));
	waitGPU();
	waitFrame();

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

	std::wstring resourcePath = GetWorkingDirectoryW();
	std::wstring defaultPath = resourcePath;
	std::wstring computePath = resourcePath;
	defaultPath += L"..\\..\\D3D12Practice_ExecuteIndirect\\resources\\Default.hlsl";
	computePath += L"..\\..\\D3D12Practice_ExecuteIndirect\\resources\\DefaultCompute.hlsl";

	ComPtr<ID3DBlob> vertexBlob;
	ComPtr<ID3DBlob> pixelBlob;
	ComPtr<ID3DBlob> computeBlob;
	ComPtr<ID3DBlob> error;

#ifdef _DEBUG
	DWORD compileFlag = D3DCOMPILE_DEBUG;
#else
	DWORD compileFlag = 0;
#endif

	Throw(D3DCompileFromFile(defaultPath.c_str(), nullptr, nullptr, "vert", "vs_5_0", compileFlag, 0, vertexBlob.GetAddressOf(), error.GetAddressOf()));
	Throw(D3DCompileFromFile(defaultPath.c_str(), nullptr, nullptr, "frag", "ps_5_0", compileFlag, 0, pixelBlob.GetAddressOf(), error.GetAddressOf()));
	
	Throw(D3DCompileFromFile(computePath.c_str(), nullptr, nullptr, "comp", "cs_5_0", compileFlag, 0, computeBlob.GetAddressOf(), error.GetAddressOf()));

	D3D12_SHADER_BYTECODE vertexByte = { vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize() };
	D3D12_SHADER_BYTECODE pixelByte = { pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize() };
	D3D12_SHADER_BYTECODE computeByte = { computeBlob->GetBufferPointer(), computeBlob->GetBufferSize() };

	D3D12_INPUT_ELEMENT_DESC inputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	D3D12_ROOT_SIGNATURE_DESC defaultRootSignDesc{};
	D3D12_ROOT_SIGNATURE_DESC computeRootSignDesc{};

	RootParameter defaultRootParam;
	RootParameter computeRootParam[2];

	defaultRootParam.AsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	defaultRootSignDesc.pParameters = defaultRootParam.Parameter.get();
	defaultRootSignDesc.NumParameters = 1;
	defaultRootSignDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	computeRootParam[0].AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	computeRootParam[0].AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
	 
	computeRootParam[0].AsDescriptorTable();
	computeRootParam[1].AsConstantBuffer(4, 0);
	
	D3D12_ROOT_PARAMETER computeRootParamArray[] = { *computeRootParam[0].Parameter, *computeRootParam[1].Parameter };

	computeRootSignDesc.NumParameters = 2;
	computeRootSignDesc.pParameters = computeRootParamArray;

	ComPtr<ID3DBlob> computeRootSign;
	ComPtr<ID3DBlob> defaultRootSign;

	Throw(D3D12SerializeRootSignature(&computeRootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1, computeRootSign.GetAddressOf(), error.GetAddressOf()));
	Throw(D3D12SerializeRootSignature(&defaultRootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1, defaultRootSign.GetAddressOf(), error.GetAddressOf()));
	
	Throw(mDevice->CreateRootSignature(0, computeRootSign->GetBufferPointer(), computeRootSign->GetBufferSize(), IID_PPV_ARGS(&mComputeRootSign)));
	Throw(mDevice->CreateRootSignature(0, defaultRootSign->GetBufferPointer(), defaultRootSign->GetBufferSize(), IID_PPV_ARGS(&mDefaultRootSign)));

	D3D12_BLEND_DESC defaultBlend = MakeDefaultBlendState();
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc{};

	psoDesc.pRootSignature = mDefaultRootSign.Get();
	psoDesc.VS = vertexByte;
	psoDesc.PS = pixelByte;
	psoDesc.BlendState = defaultBlend;
	psoDesc.InputLayout = { inputElements, ARRAYSIZE(inputElements) };
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.RasterizerState = MakeDefaultRasterizerDescriptor();
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = 0xFFFFFFFF;
	psoDesc.DepthStencilState = MakeDefaultDepthStencilState();
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.NumRenderTargets = 1;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	
	computePsoDesc.CS = computeByte;
	computePsoDesc.pRootSignature = mComputeRootSign.Get();

	Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
	Throw(mDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&mComputePSO)));

	Throw(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCmdAllocator[0])));
	Throw(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&mCmdAllocator[1])));
	
	Throw(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAllocator[0].Get(), mPSO.Get(), IID_PPV_ARGS(&mCmdList)));
	Throw(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, mCmdAllocator[1].Get(), mComputePSO.Get(), IID_PPV_ARGS(&mComputeCmdList)));

	Throw(mCmdList->Close());
	Throw(mComputeCmdList->Close());

	mFence = MultipleFenceObject(mDevice.Get(), FRAME_COUNT);
}

void DXSample_ExecuteIndirect::loadAssets()
{
	mViewport = {};
	mScissorRect = {};

	mViewport.Width = mWidth;
	mViewport.Height = mHeight;
	mViewport.MaxDepth = 1.0f;

	mScissorRect.right = mWidth;
	mScissorRect.bottom = mHeight;

}

void DXSample_ExecuteIndirect::waitGPU()
{

	Throw(mCmdQueue->Signal(mFence.Fence.Get(), mFence.Values[mFrameIndex]));

	mFence.Fence->SetEventOnCompletion(mFence.Values[mFrameIndex], mFence.Handle);
	WaitForSingleObject(mFence.Handle, INFINITE);

	mFence.Values[mFrameIndex]++;
}

void DXSample_ExecuteIndirect::waitFrame()
{
	const fence64 currentValue = mFence.Values[mFrameIndex];
	static ID3D12Fence* fence = mFence.Fence.Get();

	Throw(mCmdQueue->Signal(fence, currentValue));
	
	mFrameIndex = mSwapchain->GetCurrentBackBufferIndex();

	if (fence->GetCompletedValue() < mFence.Values[mFrameIndex])
	{
		fence->SetEventOnCompletion(mFence.Values[mFrameIndex], mFence.Handle);

		WaitForSingleObject(mFence.Handle, INFINITE);
	}

	mFence.Values[mFrameIndex] = (fence64)currentValue + 1;

}
