#pragma once
#include "VisualizationBase.h"

class Visualization : public VisualizationBase
{
private:
	HGLRC	gl = 0;
	HWND	wnd = 0;
	HDC		wndDC = 0;
	BOOL	wndRegistered = false;
	BOOL	wndRectDirty = true;
	RECT	wndRect = {};

	HWND GetOwnerWnd();
	void OnClosed();
	void UpdateWindowCaption();
private:
	static LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l);
protected:
	virtual void ConfigLoad(IAIMPConfig* config);
	virtual void ConfigSave(IAIMPConfig* config);
	virtual void OnError(const char* text);

	// IAIMPExtensionEmbeddedVisualization
	virtual void WINAPI Finalize();
	virtual HRESULT WINAPI Initialize(INT32 Width, INT32 Height);
	virtual void WINAPI Click(INT32 X, INT32 Y, INT32 Button);
	virtual void WINAPI Draw(HCANVAS Canvas, PAIMPVisualData Data);
	virtual HRESULT WINAPI GetName(IAIMPString** S);
	virtual void WINAPI Resize(INT32 NewWidth, INT32 NewHeight);
public:
	Visualization(IAIMPCore* core, HINSTANCE inst);
};