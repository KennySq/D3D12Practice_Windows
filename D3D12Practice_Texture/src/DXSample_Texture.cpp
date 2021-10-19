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
}

void DXSample_Texture::Render(float delta)
{
}

void DXSample_Texture::Release()
{
}

void DXSample_Texture::startPipeline()
{
	AcquireHardware();

	mFence = MakeFence(mDevice.Get());

	D3D12_ROOT_SIGNATURE_DESC rootSignDesc{};

	rootSignDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignDesc.pParameters = nullptr;
	rootSignDesc.pStaticSamplers = nullptr;
	
	ComPtr<ID3DBlob> rootSign;
	ComPtr<ID3DBlob> error;

	Throw(D3D12SerializeRootSignature(&rootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSign, &error));
	Throw(mDevice->CreateRootSignature(0, rootSign->GetBufferPointer(), rootSign->GetBufferSize(), IID_PPV_ARGS(&mRootSign)));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

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
	rasterizerDesc.ForcedSampleCount = 1;


	psoDesc.pRootSignature = mRootSign.Get();
	psoDesc.VS = vs;
	psoDesc.PS = ps;
	psoDesc.InputLayout = ilDesc;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.RasterizerState = rasterizerDesc;
	
	Throw(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));

	Throw(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCmdAllocator)));
	Throw(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAllocator.Get(), mPSO.Get(), IID_PPV_ARGS(&mCmdList)));
}

void DXSample_Texture::loadAssets()
{
	generateTexture();

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.NumDescriptors = 1;

	mSrvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	Throw(mDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvHeap)));

	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle{};
	srvHandle.ptr = mSrvHeap->GetCPUDescriptorHandleForHeapStart().ptr;

	mDevice->CreateShaderResourceView(mTexture.Get(), nullptr, srvHandle);

	// RTV Descriptor Heap
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};

	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = 1;

	mRtvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	Throw(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
	rtvHandle.ptr = mRtvHeap->GetCPUDescriptorHandleForHeapStart().ptr;
	rtvHandle.ptr = rtvHandle.ptr + ((address64)mFrameIndex * (address64)mRtvDescSize);
	
	mDevice->CreateRenderTargetView(mRenderTargets[mFrameIndex].Get(), nullptr, rtvHandle);

}

void DXSample_Texture::generateTexture()
{
	D3D12_RESOURCE_DESC1 resourceDesc{};

	const uint width = 512;
	const uint height = 512;

	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resourceDesc.SampleDesc.Count = 1;

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
	bufferDesc.Width = width * height * sizeof(int);
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

	const uint pixelSize = 4;
	const uint rowPitch =  width * pixelSize;
	const uint cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
	const uint cellHeight = width >> 3;    // The height of a cell in the checkerboard texture.
	const uint textureSize = rowPitch * height;

	std::vector<byte> data(textureSize);
	byte* pData = &data[0];

	for (uint n = 0; n < textureSize; n += pixelSize)
	{
		uint x = n % rowPitch;
		uint y = n / rowPitch;
		uint i = x / cellPitch;
		uint j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00;        // R
			pData[n + 1] = 0x00;    // G
			pData[n + 2] = 0x00;    // B
			pData[n + 3] = 0xff;    // A
		}
		else
		{
			pData[n] = 0xff;        // R
			pData[n + 1] = 0xff;    // G
			pData[n + 2] = 0xff;    // B
			pData[n + 3] = 0xff;    // A
		}
	}

	D3D12_TEXTURE_COPY_LOCATION regionDest{};
	D3D12_TEXTURE_COPY_LOCATION regionSrc{};
	
	//D3D12_RESOURCE_BARRIER barrier{};
	//D3D12_RESOURCE_TRANSITION_BARRIER transition{};
	//barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//transition.pResource = mTexture.Get();
	//transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	//barrier.Transition = transition;

	//mCmdList->ResourceBarrier(1, &barrier);

	//transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	//transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//barrier.Transition = transition;

	//mCmdList->ResourceBarrier(1, &barrier);


	address64* rawPtr;

	Throw(textureBuffer->Map(0, &readRange, reinterpret_cast<void**>(&rawPtr)));
	
	memcpy(rawPtr, pData, pixelSize * width * height);

	textureBuffer->Unmap(0, nullptr);

	//Throw(mTexture->WriteToSubresource(0, nullptr, pData, 0, 0));

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprintLayout;
	//D3D12_SUBRESOURCE_FOOTPRINT footprint;
	//footprint.Width = width;
	//footprint.Height = height;
	//footprint.Depth = 1;
	//footprint.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	//footprint.RowPitch = 

	mDevice->GetCopyableFootprints(&bufferDesc,0, 1, 0,  )


	mCmdList->CopyBufferRegion(mTexture.Get(), 0, textureBuffer.Get(), 0, bufferDesc.Width);
	
	//Throw(mTexture->Map(0, &readRange, reinterpret_cast<void**>(&rawPtr)));
	//memcpy(rawPtr, pData, pixelSize * width * height);
	//mTexture->Unmap(0, nullptr);
}

