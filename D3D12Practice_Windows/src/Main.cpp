#include"inc/Window.h"
#include"inc/DXSample.h"

int WinMain(HINSTANCE handleInst, HINSTANCE prevInstance, LPSTR, int nCmdShow)
{
	DXSample sample = DXSample(1280, 720, "DX12 Sample");

	return Window::Run(&sample, handleInst, nCmdShow);
}