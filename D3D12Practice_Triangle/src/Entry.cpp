#include"inc/DXSample_Triangle.h"
#include"inc/Window.h"

int __stdcall WinMain(HINSTANCE handleInst, HINSTANCE prevInst, LPSTR, int nCmdShow)
{
	DXSample_Triangle sample(1280, 720, "DX12 Triangle");
	
	return Window::Run(&sample, handleInst, nCmdShow);
}