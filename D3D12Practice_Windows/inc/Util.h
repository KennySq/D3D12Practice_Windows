#pragma once
#include"inc/stdafx.h"

using namespace Microsoft::WRL;

typedef long long fence64;
typedef UINT uint;
typedef long long address64;
struct FenceObject
{
	ComPtr<ID3D12Fence> Fence;
	HANDLE Handle;
	fence64 Value;
};

struct RootParameter
{

	std::unique_ptr<D3D12_ROOT_PARAMETER> Parameter;
	std::unique_ptr<D3D12_ROOT_DESCRIPTOR_TABLE> Table;
	std::unique_ptr<D3D12_DESCRIPTOR_RANGE> Range;

	uint DescriptorCount;
	D3D12_DESCRIPTOR_RANGE_TYPE Type;

	RootParameter(uint descriptorCount, D3D12_DESCRIPTOR_RANGE_TYPE type)
		: DescriptorCount(descriptorCount), Type(type),
		Parameter(std::make_unique<D3D12_ROOT_PARAMETER>()),
		Table(std::make_unique<D3D12_ROOT_DESCRIPTOR_TABLE>()),
		Range(std::make_unique<D3D12_DESCRIPTOR_RANGE>())
	{

	}

	RootParameter(const RootParameter& other) = delete;
	RootParameter(RootParameter&& other)
		:	Parameter(std::move(other.Parameter)), Table(std::move(other.Table)), Range(std::move(other.Range))
			, DescriptorCount(other.DescriptorCount), Type(other.Type)
	{

	}

	~RootParameter()
	{

	}

};

static HRESULT Throw(HRESULT result)
{
	if (FAILED(result))
	{
		throw std::runtime_error("");
	}

	return result;
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
	rtBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	rtBlendDesc.DestBlend = D3D12_BLEND_ZERO;
	rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	for (uint i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
	{
		blendDesc.RenderTarget[i] = rtBlendDesc;
	}

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	return blendDesc;
}

static D3D12_RESOURCE_BARRIER MakeTransition(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, D3D12_RESOURCE_BARRIER_FLAGS flag = D3D12_RESOURCE_BARRIER_FLAG_NONE)
{
	D3D12_RESOURCE_BARRIER barrier{};
	D3D12_RESOURCE_TRANSITION_BARRIER transition{};

	transition.pResource = resource;
	transition.StateBefore = before;
	transition.StateAfter = after;
	transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	
	barrier.Transition = transition;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	return barrier;
}

static D3D12_CPU_DESCRIPTOR_HANDLE MakeCPUDescriptorHandle(ID3D12DescriptorHeap* heap, address64 index, address64 stride)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = { heap->GetCPUDescriptorHandleForHeapStart().ptr + (index * stride) };

	return handle;
}


static const RootParameter&& MakeRootParameter(uint descriptorCount, D3D12_DESCRIPTOR_RANGE_TYPE type)
{
	RootParameter param(descriptorCount, type);

	param.Range->NumDescriptors = descriptorCount;
	param.Range->RangeType = type;
	param.Range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	param.Table->NumDescriptorRanges = 1;
	param.Table->pDescriptorRanges = param.Range.get();

	param.Parameter->DescriptorTable = *param.Table;

	return std::move(param);
	
}