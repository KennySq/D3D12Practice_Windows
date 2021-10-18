#pragma once
#include"inc/DXSample.h"

using namespace DirectX;

#pragma comment(lib, "D3D12Practice_DXSample.lib")

class DXSample_Triangle : public DXSample
{
	struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};

public:
	DXSample_Triangle(uint width, uint height, const char* appName);
	
	virtual void Awake() override;
	virtual void Update(float delta) override;
	virtual void Render(float delta) override;
	virtual void Release() override;

private:

	void waitGPU();

	void loadAssets();

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	ComPtr<ID3D12CommandAllocator> mCmdAllocator;
	ComPtr<ID3D12GraphicsCommandList> mCmdList;

	ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;

	ComPtr<ID3D12RootSignature> mRootSign;

	ComPtr<ID3D12Fence> mFence;
	fence64 mFenceValue;
	HANDLE mFenceHandle;

	ComPtr<ID3D12Resource> mRenderTargets[BUFFER_COUNT];
	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	uint mRtvDescSize;

	ComPtr<ID3D12PipelineState> mPso;

	uint mFrameIndex = 0;

};
