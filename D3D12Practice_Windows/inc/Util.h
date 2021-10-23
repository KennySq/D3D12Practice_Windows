#pragma once
#include"inc/stdafx.h"

using namespace Microsoft::WRL;

typedef long long fence64;
typedef UINT uint;
typedef long long address64;

static HRESULT Throw(HRESULT result)
{
	if (FAILED(result))
	{
		throw std::runtime_error("");
	}

	return result;
}


struct FenceObject
{
	ComPtr<ID3D12Fence> Fence;
	HANDLE Handle;
	fence64 Value;
};

struct MultipleFenceObject
{
	ComPtr<ID3D12Fence> Fence;
	std::vector<fence64> Values;
	HANDLE Handle;
	MultipleFenceObject()
	{

	}

	MultipleFenceObject(const MultipleFenceObject& other)
		: Fence(other.Fence), Values(other.Values), Handle(other.Handle)
	{

	}

	MultipleFenceObject(ID3D12Device* device, uint count)
		: Values(count), Handle(CreateEvent(nullptr, false, false, nullptr))
	{
		Throw(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
	}

};

struct DescriptorRangeMaker
{
	const D3D12_DESCRIPTOR_RANGE& operator()(uint descriptorCount, D3D12_DESCRIPTOR_RANGE_TYPE type)
	{
		D3D12_DESCRIPTOR_RANGE range{};
		range.NumDescriptors = descriptorCount;
		range.RangeType = type;
		range.OffsetInDescriptorsFromTableStart = 0xffffffff;

		return range;
	}

	const D3D12_DESCRIPTOR_RANGE& operator()()
	{
		D3D12_DESCRIPTOR_RANGE range{};

		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		
		return range;
	}
};

struct RootParameter
{
	std::shared_ptr<D3D12_ROOT_PARAMETER> Parameter;

	uint DescriptorCount;
	D3D12_DESCRIPTOR_RANGE_TYPE Type;

	std::vector<D3D12_DESCRIPTOR_RANGE> Ranges;

	void AddRange(D3D12_DESCRIPTOR_RANGE_TYPE type, uint count, uint baseRegister, uint registerSpace = 0, D3D12_DESCRIPTOR_RANGE_FLAGS flag = D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

	void AsDescriptorTable();
	void AsConstantBuffer(uint value32Count, uint shaderRegister);
	void AsConstantBufferView(uint shaderRegister, uint registerSpace, D3D12_SHADER_VISIBILITY visibility);

	RootParameter()
		: DescriptorCount(0), Type(D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
	{

	}

	~RootParameter()
	{

	}


private:

	void generate();
};


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

static D3D12_BLEND_DESC MakeDefaultBlendState()
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


//static RootParameter&& MakeRootParameter(uint descriptorCount, D3D12_DESCRIPTOR_RANGE_TYPE type)
//{
//	RootParameter param(descriptorCount, type, );
//
//	param.Range->NumDescriptors = descriptorCount;
//	param.Range->RangeType = type;
//	param.Range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
//
//	param.Table->NumDescriptorRanges = 1;
//	param.Table->pDescriptorRanges = param.Range.get();
//
//	param.Parameter->DescriptorTable = *param.Table;
//
//	return std::move(param);
//	
//}

static D3D12_RASTERIZER_DESC MakeDefaultRasterizerDescriptor()
{
	D3D12_RASTERIZER_DESC rsDesc{};

	rsDesc.CullMode = D3D12_CULL_MODE_NONE;
	rsDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rsDesc.DepthClipEnable = true;
	rsDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rsDesc;
}

static D3D12_RESOURCE_DESC MakeTexture2DDescriptor(uint width, uint height, DXGI_FORMAT format)
{
	D3D12_RESOURCE_DESC texDesc{};
	
	texDesc.Format = format;
	texDesc.DepthOrArraySize = 1;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.SampleDesc.Count = 1;
	texDesc.MipLevels = 1;

	return texDesc;

}

static D3D12_DEPTH_STENCIL_DESC MakeDefaultDepthStencilState()
{
	D3D12_DEPTH_STENCILOP_DESC opDesc{};
	D3D12_DEPTH_STENCIL_DESC desc;

	desc.StencilEnable = false;
	opDesc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	opDesc.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	opDesc.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	opDesc.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	desc.BackFace = opDesc;
	desc.FrontFace = opDesc;

	desc.DepthEnable = true;
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0xFF;

	desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	
	return desc;
}