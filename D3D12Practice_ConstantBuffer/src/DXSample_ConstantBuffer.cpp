#include"inc/DXSample_ConstantBuffer.h"

DXSample_ConstantBuffer::DXSample_ConstantBuffer(uint width, uint height, const char* appName)
	: DXSample(width, height, appName)
{
	
}

void DXSample_ConstantBuffer::Awake()
{
	AcquireHardware();
	startPipeline();
	loadAssets();
	generateTriangle();

	waitGPU();

	XMStoreFloat4x4(&mRawConstant.World, DirectX::XMMatrixIdentity());
}

void DXSample_ConstantBuffer::Update(float delta)
{
	if (mRawConstant.World._41 > 2.0f)
	{
		mRawConstant.World._41 = -2.0f;
	}

}

void DXSample_ConstantBuffer::Render(float delta)
{
	static ID3D12CommandList* cmdLists[] = { mCmdList.Get() };
	Throw(mCmdAllocator->Reset());
	Throw(mCmdList->Reset(mCmdAllocator.Get(), mPSO.Get()));

	clearScreen();
	drawTriangle();

	Throw(mCmdList->Close());

	mCmdQueue->ExecuteCommandLists(1, cmdLists);

	mSwapchain->Present(1, 0);
	waitGPU();
}

void DXSample_ConstantBuffer::Release()
{

	waitGPU();

	mConstantBuffer->Unmap(0, nullptr);
}

void DXSample_ConstantBuffer::waitGPU()
{
	static ID3D12Fence* fence = mFence.Fence.Get();
	const address64 fenceValue = mFence.Value;

	mCmdQueue->Signal(fence, fenceValue);
	
	mFence.Value++;

	if (fence->GetCompletedValue() < fenceValue)
	{
		fence->SetEventOnCompletion(fenceValue, mFence.Handle);

		WaitForSingleObject(mFence.Handle, INFINITE);
	}

	mFrameIndex = mSwapchain->GetCurrentBackBufferIndex();
}

void DXSample_ConstantBuffer::clearScreen()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = MakeCPUDescriptorHandle(mRtvHeap.Get(), mFrameIndex, mRtvDescSize);

	D3D12_RESOURCE_BARRIER rtvBarrier = MakeTransition(mRenderTargets[mFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	mCmdList->ResourceBarrier(1, &rtvBarrier);

	mCmdList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
	mCmdList->ClearRenderTargetView(rtvHandle, DirectX::Colors::Green, 0, nullptr);
}

void DXSample_ConstantBuffer::drawTriangle()
{
	static ID3D12DescriptorHeap* cbvHeaps[] = { mCbvHeap.Get() };
	mCmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);

	mCmdList->SetGraphicsRootSignature(mRootSign.Get());

	mCmdList->SetPipelineState(mPSO.Get());

	mCmdList->SetDescriptorHeaps(1, cbvHeaps);
	
	mCmdList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());
	//mCmdList->SetGraphicsRootConstantBufferView(0, mConstantBuffer->GetGPUVirtualAddress());
	
	DirectX::XMMATRIX reader;

	reader = DirectX::XMLoadFloat4x4(&mRawConstant.World);

	reader *= DirectX::XMMatrixTranslation(0.01f, 0, 0);

	DirectX::XMStoreFloat4x4(&mRawConstant.World, reader);

	D3D12_RANGE range{};
	address64* ptr;
	mConstantBuffer->Map(0, &range, reinterpret_cast<void**>(&ptr));
	
	memcpy(ptr, &mRawConstant, sizeof(Constants));

	mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCmdList->RSSetViewports(1, &mViewport);
	mCmdList->RSSetScissorRects(1, &mScissorRect);

	mCmdList->DrawInstanced(3, 1, 0, 0);

	D3D12_RESOURCE_BARRIER rtvBarrier = MakeTransition(mRenderTargets[mFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCmdList->ResourceBarrier(1, &rtvBarrier);

}

void DXSample_ConstantBuffer::generateTriangle()
{
	Vertex vertices[] =
	{
		{ {-0.25f, 0.25f * mAspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f}},
		{ {-0.25f, -0.25f * mAspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f}},
		{ {0.25f,  -0.25f * mAspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f}},
	};

	D3D12_RESOURCE_DESC vbDesc{};

	vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vbDesc.Format = DXGI_FORMAT_UNKNOWN;
	vbDesc.Width = sizeof(vertices);
	vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vbDesc.MipLevels = 1;
	vbDesc.Height = 1;
	vbDesc.SampleDesc.Count = 1;
	vbDesc.DepthOrArraySize = 1;
	
	D3D12_HEAP_PROPERTIES heapProp{};

	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	


	Throw(mDevice->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &vbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mVertexBuffer)));

	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = sizeof(vertices);
	mVertexBufferView.StrideInBytes = sizeof(Vertex);

	D3D12_RANGE readRange{};

	address64* ptr;

	Throw(mVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&ptr)));
	memcpy(ptr, vertices, sizeof(vertices));
	mVertexBuffer->Unmap(0, nullptr);
	
	return;
}

void DXSample_ConstantBuffer::startPipeline()
{
	Throw(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence.Fence)));

	D3D12_ROOT_SIGNATURE_DESC rootDesc{};

	mRootParameter = RootParameter(1, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);

	ComPtr<ID3DBlob> rootSign;
	ComPtr<ID3DBlob> error;

	rootDesc.NumParameters = 1;
	rootDesc.pParameters = mRootParameter.Parameter.get();
	rootDesc.NumStaticSamplers = 0;
	rootDesc.pStaticSamplers = nullptr;
	rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	Throw(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSign, &error));
	Throw(mDevice->CreateRootSignature(0, rootSign->GetBufferPointer(), rootSign->GetBufferSize(), IID_PPV_ARGS(&mRootSign)));

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	D3D12_RESOURCE_DESC rtDesc = MakeTexture2DDescriptor(mWidth, mHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

	rtvHeapDesc.NumDescriptors = FRAME_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	
	Throw(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));
	
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
	mRtvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (uint i = 0; i < FRAME_COUNT; i++)
	{
		rtvHandle.ptr = mRtvHeap->GetCPUDescriptorHandleForHeapStart().ptr;

		Throw(mSwapchain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])));

		rtvHandle.ptr += ((address64)i * (address64)mRtvDescSize);
		
		mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);
	}

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc{};

	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	Throw(mDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap)));

	D3D12_HEAP_PROPERTIES cbvHeapProps{};
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle{};
	D3D12_RESOURCE_DESC cbDesc{};

	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();

	cbvHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	cbvHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	
	cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbDesc.Width = sizeof(Constants);
	cbDesc.Format = DXGI_FORMAT_UNKNOWN;
	cbDesc.MipLevels = 1;
	cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	cbDesc.SampleDesc.Count = 1;
	cbDesc.Height = 1;
	cbDesc.DepthOrArraySize = 1;

	mCbvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	Throw(mDevice->CreateCommittedResource(&cbvHeapProps, D3D12_HEAP_FLAG_NONE, &cbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mConstantBuffer)));

	cbvDesc.BufferLocation = mConstantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = sizeof(Constants);
	cbvHandle = MakeCPUDescriptorHandle(mCbvHeap.Get(), 0, mCbvDescSize);

	mDevice->CreateConstantBufferView(&cbvDesc, cbvHandle);

}

void DXSample_ConstantBuffer::loadAssets()
{
	mViewport.Width = mWidth;
	mViewport.Height = mHeight;
	mViewport.MaxDepth = 1.0f;

	mScissorRect.right = mWidth;
	mScissorRect.bottom = mHeight;

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

	std::wstring path = GetWorkingDirectoryW();

	path += L"..\\..\\D3D12Practice_ConstantBuffer\\resources\\Default.hlsl";

#ifdef _DEBUG
	DWORD compileFlag = D3DCOMPILE_DEBUG;
#else
	DWORD compileFlag = 0;
#endif

	Throw(D3DCompileFromFile(path.c_str(), nullptr, nullptr, "vert", "vs_5_0", compileFlag, 0, &vertexShader, nullptr));
	Throw(D3DCompileFromFile(path.c_str(), nullptr, nullptr, "frag", "ps_5_0", compileFlag, 0, &pixelShader, nullptr));

	D3D12_SHADER_BYTECODE vertexBytecode{ vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	D3D12_SHADER_BYTECODE pixelBytecode{ pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };

	D3D12_INPUT_ELEMENT_DESC inputElements[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};	

	D3D12_INPUT_LAYOUT_DESC ilDesc{ inputElements, ARRAYSIZE(inputElements) };

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

	psoDesc.BlendState = MakeDefaultBlendState();
	psoDesc.RasterizerState = MakeDefaultRasterizerDescriptor();

	psoDesc.VS = vertexBytecode;
	psoDesc.PS = pixelBytecode;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.InputLayout = ilDesc;
	psoDesc.pRootSignature = mRootSign.Get();
	psoDesc.SampleDesc.Count = 1;
	psoDesc.NumRenderTargets = FRAME_COUNT;
	//psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleMask = 0xFFFFFFFF;
	for (uint i = 0; i < FRAME_COUNT; i++)
	{
		psoDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
	}


	Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));

	
	Throw(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCmdAllocator)));
	Throw(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAllocator.Get(), mPSO.Get(), IID_PPV_ARGS(&mCmdList)));

	mCmdList->Close();

}
