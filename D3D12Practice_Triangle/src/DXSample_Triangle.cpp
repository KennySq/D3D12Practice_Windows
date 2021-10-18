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
}

void DXSample_Triangle::Update(float delta)
{


}

void DXSample_Triangle::Render(float delta)
{
}

void DXSample_Triangle::Release()
{
	
}

void DXSample_Triangle::loadAssets()
{
	Throw(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};

	heapDesc.NumDescriptors = BUFFER_COUNT;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	Throw(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mRtvHeap)));

	uint rtvStride = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;

	rtvHandle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();

	for (uint i = 0; i < BUFFER_COUNT; i++)
	{
		auto rawPtr = rtvHandle.ptr;
		Throw(mSwapchain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])));

		mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);
	
		rawPtr += rtvStride;
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


	//Throw(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAllocator.Get(), ))
}
