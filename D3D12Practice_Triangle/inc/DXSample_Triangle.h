#pragma once
#include"inc/DXSample.h"

#pragma comment(lib, "D3D12Practice_DXSample.lib")

class DXSample_Triangle : public DXSample
{
public:
	DXSample_Triangle(uint width, uint height, const char* appName);
	
	virtual void Awake() override;
	virtual void Update(float delta) override;
	virtual void Render(float delta) override;
	virtual void Release() override;

private:
	void loadAssets();

	ComPtr<ID3D12CommandList> mCmdList;

	ComPtr<ID3D12Resource> mVertexBuffer;
	ComPtr<ID3D12RootSignature> mRootSign;

	ComPtr<ID3D12Fence> mFence;
	fence64 mFenceValue;
	HANDLE mFenceHandle;

	ComPtr<ID3D12Resource> mRenderTargets[BUFFER_COUNT];
	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
};
