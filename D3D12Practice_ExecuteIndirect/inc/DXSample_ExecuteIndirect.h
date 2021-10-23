#pragma once
#include"inc/stdafx.h"
#include"inc/DXSample.h"

#pragma comment(lib, "D3D12Practice_DXSample.lib")

using namespace DirectX;

class DXSample_ExecuteIndirect : public DXSample
{
	struct Constants
	{
		XMFLOAT4 Velocity;
		XMFLOAT4 Offset;
		XMFLOAT4 Color;
		XMFLOAT4X4 Projection;

		float pad[36];
	};

public:
	DXSample_ExecuteIndirect(uint width, uint height, const char* appName);

	virtual void Awake() override;
	virtual void Update(float delta) override;
	virtual void Render(float delta) override;
	virtual void Release() override;
private:
	static const uint FRAME_COUNT = 2;
	static const uint RESOURCES_PER_FRAME = 3;
	static const uint TRIANGLE_COUNT = 1024;

	void startPipeline();
	void loadAssets();
	void waitGPU();
	void waitFrame();

	void clearScreen();

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	ComPtr<ID3D12Resource> mRenderTargets[FRAME_COUNT];
	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	uint mRtvDescSize;
	uint mFrameIndex = 0;

	ComPtr<ID3D12Resource> mDepthStencil;
	ComPtr<ID3D12DescriptorHeap> mDsvHeap;
	uint mDsvDescSize;

	ComPtr<ID3D12Resource> mCbuffer;
	ComPtr<ID3D12DescriptorHeap> mResourceHeap;
	uint mResourceDescSize;
	std::vector<Constants> mInstanceData;

	DescriptorRangeMaker mDRM;

	ComPtr<ID3D12GraphicsCommandList> mCmdList;
	ComPtr<ID3D12GraphicsCommandList> mComputeCmdList;


	ComPtr<ID3D12RootSignature> mDefaultRootSign;
	ComPtr<ID3D12RootSignature> mComputeRootSign;
	MultipleFenceObject mFence;

	ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;

	ComPtr<ID3D12PipelineState> mPSO;
	ComPtr<ID3D12PipelineState> mComputePSO;

	ComPtr<ID3D12CommandAllocator> mCmdAllocator[FRAME_COUNT];
	ComPtr<ID3D12CommandAllocator> mComputeCmdAllocator;

};