#pragma once
#include"inc/stdafx.h"
#include"inc/DXSample.h"
#include"inc/Window.h"

#pragma comment(lib, "D3D12Practice_DXSample.lib")

class DXSample_Texture : public DXSample
{
public:
	DXSample_Texture(uint width, uint height, const char* appName);
	
	virtual void Awake() override;
	virtual void Update(float delta) override;
	virtual void Render(float delta) override;
	virtual void Release() override;
private:

	static const uint FRAME_COUNT = 2;

	void startPipeline();
	void loadAssets();

	void generateTexture();

	FenceObject mFence;
	uint mFrameIndex = 0;

	ComPtr<ID3D12GraphicsCommandList> mCmdList;
	ComPtr<ID3D12CommandAllocator> mCmdAllocator;

	ComPtr<ID3D12RootSignature> mRootSign;
	
	ComPtr<ID3D12Resource> mRenderTargets[FRAME_COUNT];
	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	uint mRtvDescSize;

	ComPtr<ID3D12Resource> mTexture;
	ComPtr<ID3D12DescriptorHeap> mSrvHeap;
	uint mSrvDescSize;

	ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;

	ComPtr<ID3D12PipelineState> mPSO;
};