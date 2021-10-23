#include "stdafx.h"

void RootParameter::AddRange(D3D12_DESCRIPTOR_RANGE_TYPE type, uint count, uint baseRegister, uint registerSpace, D3D12_DESCRIPTOR_RANGE_FLAGS flag)
{
	D3D12_DESCRIPTOR_RANGE range = { type, count, baseRegister, registerSpace, flag };

	Ranges.push_back(range);
}

void RootParameter::AsDescriptorTable()
{
	Parameter = std::make_shared<D3D12_ROOT_PARAMETER>();

	Parameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	Parameter->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	Parameter->DescriptorTable.NumDescriptorRanges = Ranges.size();
	Parameter->DescriptorTable.pDescriptorRanges = Ranges.data();

}

void RootParameter::AsConstantBuffer(uint value32Count, uint shaderRegister)
{
	Parameter = std::make_shared<D3D12_ROOT_PARAMETER>();

	Parameter->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	Parameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	Parameter->Constants.Num32BitValues = value32Count;
	Parameter->Constants.ShaderRegister = shaderRegister;

	
}

void RootParameter::AsConstantBufferView(uint shaderRegister, uint registerSpace, D3D12_SHADER_VISIBILITY visibility)
{
	Parameter = std::make_shared<D3D12_ROOT_PARAMETER>();

	Parameter->ShaderVisibility = visibility;
	Parameter->Descriptor.ShaderRegister = shaderRegister;
	Parameter->Descriptor.RegisterSpace = registerSpace;

	Parameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

}

