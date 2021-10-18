#pragma once
#include"stdafx.h"

class DXSample;
class Window
{
public:
	Window(uint width, uint height, HWND hWnd, HINSTANCE handleInst, DXSample* sample, int nCmdShow);
	~Window();
	static int Run(DXSample* sample, HINSTANCE handleInst, int nCmdShow);
	static HWND GetHwnd() { return mHandle; }
private:

	void registerClass(HINSTANCE handleInst);
	static LRESULT __stdcall WndProc(HWND hWnd, uint message, WPARAM wParam, LPARAM lParam);


	static HWND mHandle;
	HINSTANCE mHandleInst;

	uint mWidth;
	uint mHeight;
};