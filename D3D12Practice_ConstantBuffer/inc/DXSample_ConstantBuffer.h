#pragma once
#include"inc/stdafx.h"
#include"inc/DXSample.h"

#pragma comment(lib, "D3D12Practice_DXSample.lib")

class DXSample_ConstantBuffer : public DXSample
{
public:
	DXSample_ConstantBuffer(uint width, uint height, const char* appName);


	virtual void Awake() override;
	virtual void Update(float delta) override;
	virtual void Render(float delta) override;
	virtual void Release() override;

private:

	ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;

	ComPtr<ID3D12Resource> mConstantBuffer;

	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	
	ComPtr<ID3D12PipelineState> mPSO;

	ComPtr<ID3D12CommandList> mCmdList;
	ComPtr<ID3D12CommandAllocator> mCmdAllocator;

	ComPtr<ID3D12RootSignature> mRootSign;


	FenceObject mFence;

	uint mFrameIndex = 0;

	void waitGPU();

	void startPipeline();
	void loadAssets();



};