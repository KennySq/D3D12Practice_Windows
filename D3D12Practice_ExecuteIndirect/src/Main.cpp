#include"inc/stdafx.h"
#include"inc/Window.h"
#include"inc/DXSample_ExecuteIndirect.h"

int __stdcall WinMain(HINSTANCE handleInst, HINSTANCE prevInst, LPSTR message, int nCmdShow)
{
	DXSample_ExecuteIndirect sample(1280, 720, "DX12 Sample Execute Indirect");

	return Window::Run(&sample, handleInst, nCmdShow);
}