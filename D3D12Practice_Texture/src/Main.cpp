#include"inc/stdafx.h"
#include"inc/DXSample_Texture.h"
#include"inc/Window.h"


int __stdcall WinMain(HINSTANCE handleInst, HINSTANCE prevInst, LPSTR msg, int nCmdShow)
{
	DXSample_Texture sample(1280, 720, "DX12 Sample - Texture");
	return Window::Run(&sample, handleInst, nCmdShow);
}