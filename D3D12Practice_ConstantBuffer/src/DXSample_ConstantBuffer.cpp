#include"inc/DXSample_ConstantBuffer.h"

DXSample_ConstantBuffer::DXSample_ConstantBuffer(uint width, uint height, const char* appName)
	: DXSample(width, height, appName)
{
	
}

void DXSample_ConstantBuffer::Awake()
{
	AcquireHardware();

	D3D12_ROOT_SIGNATURE_DESC rootDesc{};

	const auto&& rootParam = MakeRootParameter(1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);

	ComPtr<ID3DBlob> rootSign;
	ComPtr<ID3DBlob> error;
	Throw(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSign, &error));

	Throw(mDevice->CreateRootSignature(0, rootSign->GetBufferPointer(), rootSign->GetBufferSize(), IID_PPV_ARGS(&mRootSign)));



	waitGPU();
}

void DXSample_ConstantBuffer::Update(float delta)
{
}

void DXSample_ConstantBuffer::Render(float delta)
{
}

void DXSample_ConstantBuffer::Release()
{
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
