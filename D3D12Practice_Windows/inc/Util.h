#pragma once
#include"stdafx.h"

static HRESULT Throw(HRESULT result)
{
	if (FAILED(result))
	{
		throw std::runtime_error("");
	}
}