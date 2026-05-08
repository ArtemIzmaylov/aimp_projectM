#pragma once

#include "VisualizationBase.h"

class VisualizationWindowBased : public VisualizationBase
{
private:
	BOOL wndRegistered = false;
	void OnClosed();
private:
	static LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l);
protected:
	HGLRC gl = 0;
	HWND wnd = 0;
	HDC	 wndDC = 0;

	BOOL InitOpenGL();
	BOOL InitWindow(int x, int y, int w, int h);
	virtual void WINAPI Finalize();
	virtual void WINAPI Draw(HCANVAS Canvas, PAIMPVisualData Data);
public:
	VisualizationWindowBased(IAIMPCore* core, HINSTANCE inst);
};