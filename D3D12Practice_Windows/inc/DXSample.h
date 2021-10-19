#pragma once
#include"stdafx.h"

using namespace Microsoft::WRL;

class DXSample
{
public:
	DXSample(uint width, uint height, const char* name);
	virtual ~DXSample();

	virtual void Awake() = 0;
	virtual void Update(float delta) = 0;
	virtual void Render(float delta) = 0;
	virtual void Release() = 0;

	uint GetWidth() const { return mWidth; }
	uint GetHeight() const { return mHeight; }

	const char* GetName() const { return mAppName; }

protected:
	static const uint BUFFER_COUNT = 2;

	void AcquireHardware();
	
	uint mWidth;
	uint mHeight;

	ComPtr<ID3D12Device4> mDevice;
	ComPtr<IDXGIFactory6> mFactory;
	ComPtr<IDXGIAdapter4> mAdapter;

	ComPtr<ID3D12CommandQueue> mCmdQueue;

	ComPtr<IDXGISwapChain4> mSwapchain;

	float mAspectRatio;
private:

	const char* mAppName;

};