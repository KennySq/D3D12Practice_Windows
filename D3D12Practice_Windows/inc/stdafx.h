#pragma once

#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<d3d12shader.h>
#include<d3dcompiler.h>
#include<DirectXMath.h>
#include<DirectXColors.h>

#include<wrl.h>

#include<exception>
#include<stdexcept>

#include"Util.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

typedef UINT uint;
typedef unsigned char byte;
typedef long long address64;
typedef long long fence64;