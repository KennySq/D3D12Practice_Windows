#include"inc/DXSample_Triangle.h"
#include"inc/Window.h"

DXSample_Triangle::DXSample_Triangle(uint width, uint height, const char* appName)
	: DXSample(width, height, appName)
{

}

void DXSample_Triangle::Awake()
{
	AcquireHardware();
	loadAssets();

	mbInitialized = true;

	waitGPU();
}

void DXSample_Triangle::Update(float delta)
{
	if (mbInitialized != true)
	{
		return;
	}

	generateCommands();
}

void DXSample_Triangle::Render(float delta)
{
	if (mbInitialized != true)
	{
		return;
	}
	static ID3D12CommandList* cmdLists[] = { mCmdList.Get() };

	mCmdQueue->ExecuteCommandLists(1, cmdLists);

	mSwapchain->Present(1, 0);

	waitGPU();
}

void DXSample_Triangle::Release()
{
	
}

void DXSample_Triangle::waitGPU()
{
	const fence64 fenceValue = mFenceValue;

	Throw(mCmdQueue->Signal(mFence.Get(), fenceValue));

	mFenceValue++;

	if (mFence->GetCompletedValue() < fenceValue)
	{
		Throw(mFence->SetEventOnCompletion(fenceValue, mFenceHandle));

		WaitForSingleObject(mFenceHandle, INFINITE);
	}

	mFrameIndex = mSwapchain->GetCurrentBackBufferIndex();

}

void DXSample_Triangle::loadAssets()
{
	mViewport.Width = mWidth;
	mViewport.Height = mHeight;
	mViewport.MaxDepth = 1.0f;

	mScissorRect.right = mWidth;
	mScissorRect.bottom = mHeight;

	Throw(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

	mFenceHandle = CreateEvent(nullptr, false, false, nullptr);

	if (mFenceHandle == nullptr)
	{
		Throw(HRESULT_FROM_WIN32(GetLastError()));
	}

	waitGPU();

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};

	heapDesc.NumDescriptors = BUFFER_COUNT;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	Throw(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mRtvHeap)));

	mRtvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;

	rtvHandle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();

	for (uint i = 0; i < BUFFER_COUNT; i++)
	{
		auto rawPtr = rtvHandle.ptr;
		Throw(mSwapchain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])));

		mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);
		mRenderTargets[i]->SetName((std::wstring(L"Render Target") + std::to_wstring(i)).c_str());
		rawPtr += mRtvDescSize;
		rtvHandle.ptr = rawPtr;
	}
	
	D3D12_ROOT_SIGNATURE_DESC rootSignDesc{};

	rootSignDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignDesc.NumParameters = 0;
	rootSignDesc.NumStaticSamplers = 0;
	rootSignDesc.pParameters = nullptr;
	rootSignDesc.pStaticSamplers = nullptr;

	ComPtr<ID3DBlob> signBlob;
	ComPtr<ID3DBlob> errorBlob;

	Throw(D3D12SerializeRootSignature(&rootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1, signBlob.GetAddressOf(), errorBlob.GetAddressOf()));
	Throw(mDevice->CreateRootSignature(0, signBlob->GetBufferPointer(), signBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSign)));

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

#ifdef _DEBUG
	DWORD compileFlag = D3DCOMPILE_DEBUG;
#else
	DWORD compileFlag = 0;
#endif
	std::wstring path = GetWorkingDirectoryW();
	path += L"..\\..\\D3D12Practice_Triangle\\resources\\Default.hlsl";

	Throw(D3DCompileFromFile(path.c_str(), nullptr, nullptr, "vert", "vs_5_0", compileFlag, 0, vertexShader.GetAddressOf(), errorBlob.GetAddressOf()));
	Throw(D3DCompileFromFile(path.c_str(), nullptr, nullptr, "frag", "ps_5_0", compileFlag, 0, pixelShader.GetAddressOf(), errorBlob.GetAddressOf()));
	
	D3D12_SHADER_BYTECODE vertexBytes = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	D3D12_SHADER_BYTECODE pixelBytes = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };

	D3D12_INPUT_ELEMENT_DESC inputElements[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineState{};

	D3D12_RASTERIZER_DESC rasterDesc{};

	rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_BLEND_DESC blendDesc{};
	D3D12_RENDER_TARGET_BLEND_DESC rtvBlendDesc{};

	rtvBlendDesc.BlendEnable = false;
	rtvBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	rtvBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	rtvBlendDesc.DestBlend = D3D12_BLEND_ZERO;
	rtvBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rtvBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	rtvBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	rtvBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	rtvBlendDesc.LogicOpEnable = false;
	rtvBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	for (uint i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
	{
		blendDesc.RenderTarget[i] = rtvBlendDesc;
	}

	pipelineState.InputLayout = { inputElements, ARRAYSIZE(inputElements) };
	pipelineState.BlendState = blendDesc;
	pipelineState.VS = vertexBytes;
	pipelineState.PS = pixelBytes;
	pipelineState.NumRenderTargets = 1;
	pipelineState.SampleDesc.Count = 1;
	pipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineState.pRootSignature = mRootSign.Get();
	pipelineState.DepthStencilState.DepthEnable = false;
	pipelineState.DepthStencilState.StencilEnable = false;
	pipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineState.SampleMask = UINT_MAX;
	pipelineState.RasterizerState = rasterDesc;

	Throw(mDevice->CreateGraphicsPipelineState(&pipelineState, IID_PPV_ARGS(&mPso)));
	
	Throw(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCmdAllocator)));

	Throw(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAllocator.Get(), mPso.Get(), IID_PPV_ARGS(&mCmdList)));

	Throw(mCmdList->Close());

	{
		Vertex vertices[] =
		{
			{ {0.0f, 0.25f * mAspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f} },
			{ {0.25f, -0.25f * mAspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f} },
			{ {-0.25f, -0.25f * mAspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f} },
		};

		const uint vertexCount = ARRAYSIZE(vertices);
		const uint verticesSize = sizeof(vertices);

		D3D12_RANGE readRange = { 0,0 };
		address64* mapAddress;

		D3D12_HEAP_PROPERTIES heapProps{};

		heapProps.CreationNodeMask = 1;
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC vbDesc{};
		vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		vbDesc.Format = DXGI_FORMAT_UNKNOWN;
		vbDesc.Width = verticesSize;
		vbDesc.Height = 1;
		vbDesc.MipLevels = 1;
		vbDesc.DepthOrArraySize = 1;
		//vbDesc.Alignment = sizeof(Vertex);
		vbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		vbDesc.SampleDesc.Count = 1;
		vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		Throw(mDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &vbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mVertexBuffer)));
		mVertexBuffer->SetName(L"Vertex Buffer");

		Throw(mVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mapAddress)));
		
		memcpy(mapAddress, vertices, verticesSize);

		mVertexBuffer->Unmap(0, nullptr);

		mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
		mVertexBufferView.SizeInBytes = verticesSize;
		mVertexBufferView.StrideInBytes = sizeof(Vertex);
	}

	

}

void DXSample_Triangle::generateCommands()
{
	Throw(mCmdAllocator->Reset());
	Throw(mCmdList->Reset(mCmdAllocator.Get(), mPso.Get()));

	mCmdList->SetGraphicsRootSignature(mRootSign.Get());
	mCmdList->RSSetScissorRects(1, &mScissorRect);
	mCmdList->RSSetViewports(1, &mViewport);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;

	D3D12_RESOURCE_BARRIER rtvBarrier{};
	D3D12_RESOURCE_TRANSITION_BARRIER rtvTransition{};

	rtvTransition.pResource = mRenderTargets[mFrameIndex].Get();
	rtvTransition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	rtvTransition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	rtvTransition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	rtvBarrier.Transition = rtvTransition;
	rtvBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rtvBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	

	mCmdList->ResourceBarrier(1, &rtvBarrier);

	auto virtualAddr = mRtvHeap->GetCPUDescriptorHandleForHeapStart();

	rtvHandle.ptr = virtualAddr.ptr + (static_cast<address64>(mFrameIndex) * mRtvDescSize);

	mCmdList->ClearRenderTargetView(rtvHandle, Colors::Green, 0, nullptr);
	mCmdList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	mCmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);

	rtvTransition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	rtvTransition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	rtvBarrier.Transition = rtvTransition;
	
	mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	mCmdList->DrawInstanced(3, 1, 0, 0);


	mCmdList->ResourceBarrier(1, &rtvBarrier);


	Throw(mCmdList->Close());
}
