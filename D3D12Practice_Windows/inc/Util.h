#pragma once
#include"inc/stdafx.h"

using namespace Microsoft::WRL;

typedef long long fence64;
typedef UINT uint;
struct FenceObject
{
	ComPtr<ID3D12Fence> Fence;
	HANDLE Handle;
	fence64 Value;
};

static HRESULT Throw(HRESULT result)
{
	if (FAILED(result))
	{
		throw std::runtime_error("");
	}
}

static std::wstring GetWorkingDirectoryW()
{
	static constexpr unsigned int MAX_STR = 512;
	wchar_t buffer[MAX_STR];

	GetModuleFileNameW(nullptr, buffer, MAX_STR);

	std::wstring path(buffer);

	path = path.substr(0, path.find_last_of('\\') + 1);

	return path;
}

static FenceObject MakeFence(ID3D12Device4* device, D3D12_FENCE_FLAGS flags = D3D12_FENCE_FLAG_NONE)
{
	FenceObject fence;

	Throw(device->CreateFence(0, flags, IID_PPV_ARGS(&fence.Fence)));

	fence.Handle = CreateEvent(nullptr, false, false, nullptr);
	fence.Value = 0;

	return fence;
}

static D3D12_BLEND_DESC MakeBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc{};

	rtBlendDesc.BlendEnable = false;
	rtBlendDesc.LogicOpEnable = false;
	rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	rtBlendDesc.SrcBlend = D3D12_BLEND_ZERO;
	rtBlendDesc.DestBlend = D3D12_BLEND_ONE;
	rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ZERO;
	rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
	rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	for (uint i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
	{
		blendDesc.RenderTarget[i] = rtBlendDesc;
	}

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	return blendDesc;
}