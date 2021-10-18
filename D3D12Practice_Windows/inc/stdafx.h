#pragma once

#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>

#include<wrl.h>

#include<exception>
#include<stdexcept>

#include"Util.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

typedef UINT uint;
typedef unsigned char byte;
typedef long long address64;
typedef long long fence64;