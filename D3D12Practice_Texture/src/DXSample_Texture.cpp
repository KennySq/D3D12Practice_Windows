#include "..\inc\DXSample_Texture.h"

DXSample_Texture::DXSample_Texture(uint width, uint height, const char* appName)
	: DXSample(width, height, appName)
{
}

void DXSample_Texture::Awake()
{
	startPipeline();
	loadAssets();

}

void DXSample_Texture::Update(float delta)
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	D3D12_RESOURCE_BARRIER rtvBarrier;

	Throw(mCmdAllocator->Reset());
	Throw(mCmdList->Reset(mCmdAllocator.Get(), mPSO.Get()));

	rtvHandle = MakeCPUDescriptorHandle(mRtvHeap.Get(), static_cast<address64>(mFrameIndex), static_cast<address64>(mRtvDescSize));
	rtvBarrier = MakeTransition(mRenderTargets[mFrameIndex].Get(), 
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	mCmdList->ResourceBarrier(1, &rtvBarrier);

	mCmdList->ClearRenderTargetView(rtvHandle, DirectX::Colors::Green, 0, nullptr);
	mCmdList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	rtvBarrier = MakeTransition(mRenderTargets[mFrameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	mCmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mCmdList->RSSetScissorRects(1, &mScissorRect);
	mCmdList->RSSetViewports(1, &mViewport);


	mCmdList->SetGraphicsRootSignature(mRootSign.Get());
	mCmdList->SetPipelineState(mPSO.Get());

	static ID3D12DescriptorHeap* srvHeaps[] = { mSrvHeap.Get() };

	mCmdList->SetDescriptorHeaps(1, srvHeaps);
	mCmdList->SetGraphicsRootDescriptorTable(0, mSrvHeap->GetGPUDescriptorHandleForHeapStart());



	mCmdList->DrawInstanced(3, 1, 0, 0);


	mCmdList->ResourceBarrier(1, &rtvBarrier);


	Throw(mCmdList->Close());

}

void DXSample_Texture::Render(float delta)
{
	static ID3D12CommandList* cmdLists[] = { mCmdList.Get() };

	mCmdQueue->ExecuteCommandLists(1, cmdLists);
	
	
	Throw(mSwapchain->Present(1, 0));
	
	waitGPU();

}

void DXSample_Texture::Release()
{
}

//void DXSample_Texture::waitGPU()
//{
//	static ID3D12Fence* fence = mFence.Fence.Get();
//	const address64 fenceValue = mFence.Value;
//
//	Throw(mCmdQueue->Signal(fence, fenceValue));
//
//	mFence.Value++;
//
//
//	if (fence->GetCompletedValue() < fenceValue)
//	{
//		Throw(fence->SetEventOnCompletion(fenceValue, mFence.Handle));
//
//		WaitForSingleObject(mFence.Handle, INFINITE);
//	}
//
//	mFrameIndex = mSwapchain->GetCurrentBackBufferIndex();
//}

void DXSample_Texture::waitGPU()
{
	static ID3D12Fence* fence = mFence.Fence.Get();
	const address64 fenceValue = fence->GetCompletedValue() + 1;

	Throw(mCmdQueue->Signal(fence, fenceValue));

	mFence.Value++;


	if (fence->GetCompletedValue() < fenceValue)
	{
		Throw(fence->SetEventOnCompletion(fenceValue, mFence.Handle));

		WaitForSingleObject(mFence.Handle, INFINITE);
	}

	mFrameIndex = mSwapchain->GetCurrentBackBufferIndex();
}

void DXSample_Texture::startPipeline()
{
	AcquireHardware();

	mFence = MakeFence(mDevice.Get());

	D3D12_ROOT_SIGNATURE_DESC rootSignDesc = {};
	D3D12_ROOT_PARAMETER parameters[1] = { {} };
	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable = {};
	D3D12_ROOT_DESCRIPTOR descriptor = {};
	D3D12_DESCRIPTOR_RANGE range[1] = { {} };

	range[0].NumDescriptors = 1;
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	descriptorTable.pDescriptorRanges = range;
	descriptorTable.NumDescriptorRanges = 1;

	parameters[0].DescriptorTable = descriptorTable;
	parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.Filter = D3D12_FILTER::D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	

	rootSignDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignDesc.NumParameters = 1;
	rootSignDesc.NumStaticSamplers = 1;
	rootSignDesc.pParameters = parameters;
	rootSignDesc.pStaticSamplers = &samplerDesc;
	
	ComPtr<ID3DBlob> rootSign;
	ComPtr<ID3DBlob> error;

	mFrameIndex = mSwapchain->GetCurrentBackBufferIndex();

	Throw(D3D12SerializeRootSignature(&rootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSign, &error));
	Throw(mDevice->CreateRootSignature(0, rootSign->GetBufferPointer(), rootSign->GetBufferSize(), IID_PPV_ARGS(&mRootSign)));


	D3D12_INPUT_ELEMENT_DESC inputElements[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	D3D12_INPUT_LAYOUT_DESC ilDesc = { inputElements, ARRAYSIZE(inputElements) };

	D3D12_BLEND_DESC blendDesc = MakeBlendState();

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

	D3D12_SHADER_BYTECODE vs;
	D3D12_SHADER_BYTECODE ps;

	std::wstring path = GetWorkingDirectoryW();
	path += L"..\\..\\D3D12Practice_Texture\\resources\\Default.hlsl";

#ifdef _DEBUG
	DWORD compileFlag = D3DCOMPILE_DEBUG;

#else
	DWORD compileFlag = 0;
#endif

	Throw(D3DCompileFromFile(path.c_str(), nullptr, nullptr, "vert", "vs_5_0", compileFlag, 0, vertexShader.GetAddressOf(), error.GetAddressOf()));
	Throw(D3DCompileFromFile(path.c_str(), nullptr, nullptr, "frag", "ps_5_0", compileFlag, 0, pixelShader.GetAddressOf(), error.GetAddressOf()));

	vs = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	ps = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
	
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ForcedSampleCount = 0;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

	psoDesc.pRootSignature = mRootSign.Get();
	psoDesc.VS = vs;
	psoDesc.PS = ps;
	psoDesc.BlendState = blendDesc;
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.InputLayout = ilDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.RasterizerState = rasterizerDesc;
	
	Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));

	Throw(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCmdAllocator)));
	Throw(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAllocator.Get(), mPSO.Get(), IID_PPV_ARGS(&mCmdList)));

	Throw(mCmdList->Close());

	waitGPU();
}

void DXSample_Texture::loadAssets()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	mSrvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	Throw(mDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvHeap)));

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};

	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = FRAME_COUNT;

	mRtvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	Throw(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));

	// resources being generated after all descriptor heaps are initialized.

	generateTexture();

	mViewport.TopLeftX = 0;
	mViewport.TopLeftY = 0;

	mViewport.Width = (float)mWidth;
	mViewport.Height = (float)mHeight;
	mViewport.MaxDepth = 1.0f;


	mScissorRect.left = 0;
	mScissorRect.top = 0;
	mScissorRect.right = mWidth;
	mScissorRect.bottom = mHeight;


	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle{};
	srvHandle.ptr = mSrvHeap->GetCPUDescriptorHandleForHeapStart().ptr;

	mDevice->CreateShaderResourceView(mTexture.Get(), nullptr, srvHandle);

	// RTV Descriptor Heap
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
	for (uint i = 0; i < FRAME_COUNT; i++)
	{
		rtvHandle.ptr = mRtvHeap->GetCPUDescriptorHandleForHeapStart().ptr;
		rtvHandle.ptr = rtvHandle.ptr + ((address64)i * (address64)mRtvDescSize);

		Throw(mSwapchain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])));

		mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);

	}

	D3D12_RESOURCE_DESC vertexBufferDesc{};

	vertexBufferDesc.Width = sizeof(Vertex) * 3;
	vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	vertexBufferDesc.MipLevels = 1;
	vertexBufferDesc.Height = 1;
	vertexBufferDesc.DepthOrArraySize = 1;
	vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vertexBufferDesc.SampleDesc.Count = 1;
	vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	D3D12_HEAP_PROPERTIES heapProps{};

	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	Throw(mDevice->CreateCommittedResource1(&heapProps, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, nullptr, IID_PPV_ARGS(&mVertexBuffer)));

	Vertex vertices[] =
	{
		{ {0.0f, 0.25f * mAspectRatio , 0.0f},{ 0.5f, 0.0f} },
		{ {0.25f, -0.25f * mAspectRatio , 0.0f},{ 1.0f, 1.0f} },
		{ {-0.25f, -0.25f * mAspectRatio , 0.0f},{ 0.0f, 1.0f} },
	};

	D3D12_RANGE readRange{};

	byte* vertexPtr;

	Throw(mVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&vertexPtr)));

	memcpy(vertexPtr, vertices, sizeof(vertices));

	mVertexBuffer->Unmap(0, nullptr);

	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = sizeof(vertices);
	mVertexBufferView.StrideInBytes = sizeof(Vertex);

	waitGPU();
}

void DXSample_Texture::generateTexture()
{
	Throw(mCmdAllocator->Reset());
	Throw(mCmdList->Reset(mCmdAllocator.Get(), mPSO.Get()));

	const uint width = 512;
	const uint height = 512;

	const uint pixelSize = 16;
	const uint component = 4;
	const uint rowPitch = width * component;
	const uint cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
	const uint cellHeight = height >> 3;    // The height of a cell in the checkerboard texture.
	const uint textureSize = rowPitch * height;

	D3D12_RESOURCE_DESC rsDesc{};

	rsDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rsDesc.Width = width;
	rsDesc.Height = height;
	rsDesc.MipLevels = 1;
	rsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rsDesc.SampleDesc.Count = 1;
	rsDesc.DepthOrArraySize = 1;

	D3D12_HEAP_PROPERTIES heapProps{};

	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	Throw(mDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &rsDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&mTexture)));

	ComPtr<ID3D12Resource> textureBuffer;
	D3D12_RESOURCE_DESC bufferDesc{};

	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = width * height * pixelSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	D3D12_HEAP_PROPERTIES bufferHeapProps{};
	bufferHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	bufferHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	
	Throw(mDevice->CreateCommittedResource(&bufferHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureBuffer)));
	
	D3D12_RANGE readRange = {0, 0};

	std::vector<float> data(textureSize);
	float* pData = &data[0];

	for (uint n = 0; n < textureSize; n += component)
	{
		uint x = n % rowPitch;
		uint y = n / rowPitch;
		uint i = x / cellPitch;
		uint j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0;        // R
			pData[n + 1] = 0;    // G
			pData[n + 2] = 0;    // B
			pData[n + 3] = 0;    // A
		}
		else
		{
			pData[n] = 1;        // R
			pData[n + 1] = 1;    // G
			pData[n + 2] = 1;    // B
			pData[n + 3] = 1;    // A
		}
	}

	D3D12_TEXTURE_COPY_LOCATION regionDest{};
	D3D12_TEXTURE_COPY_LOCATION regionSrc{};
	
	address64* rawPtr;

	Throw(textureBuffer->Map(0, &readRange, reinterpret_cast<void**>(&rawPtr)));
	
	memcpy(rawPtr, pData, width * height * pixelSize);

	textureBuffer->Unmap(0, nullptr);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprintLayout;

	uint rows;
	unsigned long long rowSize;
	unsigned long long totalSize;
	mDevice->GetCopyableFootprints(&rsDesc, 0, 1, 0, &footprintLayout, &rows, &rowSize, &totalSize);
	
	D3D12_TEXTURE_COPY_LOCATION srcRegion;
	D3D12_TEXTURE_COPY_LOCATION destRegion;

	srcRegion.PlacedFootprint = footprintLayout;
	srcRegion.pResource = textureBuffer.Get();
	srcRegion.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

	destRegion.pResource = mTexture.Get();
	destRegion.SubresourceIndex = 0;
	destRegion.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

	mCmdList->CopyTextureRegion(&destRegion, 0, 0, 0, &srcRegion, nullptr);

	Throw(mCmdList->Close());

	ID3D12CommandList* cmdLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(1, cmdLists);

	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle{};

	mSrvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHandle = MakeCPUDescriptorHandle(mSrvHeap.Get(), 0, mSrvDescSize);

	D3D12_RESOURCE_BARRIER textureTransition = MakeTransition(mTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	mCmdList->ResourceBarrier(1, &textureTransition);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	mDevice->CreateShaderResourceView(mTexture.Get(), &srvDesc, srvHandle);


	waitGPU();
}

