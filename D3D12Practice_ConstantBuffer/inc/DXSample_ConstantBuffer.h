#pragma once
#include"inc/stdafx.h"
#include"inc/DXSample.h"

#pragma comment(lib, "D3D12Practice_DXSample.lib")

class DXSample_ConstantBuffer : public DXSample
{
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT4 Color;
	};

	struct Constants
	{
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 Unused0;
		DirectX::XMFLOAT4X4 Unused1;
		DirectX::XMFLOAT4X4 Unused2;
	};

public:
	DXSample_ConstantBuffer(uint width, uint height, const char* appName);


	virtual void Awake() override;
	virtual void Update(float delta) override;
	virtual void Render(float delta) override;
	virtual void Release() override;

private:
	static const int FRAME_COUNT = 2;

	ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;

	ComPtr<ID3D12DescriptorHeap> mCbvHeap;
	ComPtr<ID3D12Resource> mConstantBuffer;
	uint mCbvDescSize;
	Constants mRawConstant;

	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	ComPtr<ID3D12Resource> mRenderTargets[FRAME_COUNT];
	
	ComPtr<ID3D12PipelineState> mPSO;

	ComPtr<ID3D12GraphicsCommandList> mCmdList;
	ComPtr<ID3D12CommandAllocator> mCmdAllocator;

	ComPtr<ID3D12RootSignature> mRootSign;
	RootParameter mRootParameter;
	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	FenceObject mFence;

	uint mFrameIndex = 0;
	uint mRtvDescSize;

	void waitGPU();

	void clearScreen();
	void drawTriangle();
	void generateTriangle();

	void startPipeline();
	void loadAssets();



};