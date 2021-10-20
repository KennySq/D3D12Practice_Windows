#include"inc/DXSample_ConstantBuffer.h"
#include"inc/Window.h"

int __stdcall WinMain(HINSTANCE handleInst, HINSTANCE, LPSTR, int nCmdShow)
{
	DXSample_ConstantBuffer sample(1280, 720, "DX12 Sample Constant Buffer");

	return Window::Run(&sample, handleInst, nCmdShow);
}