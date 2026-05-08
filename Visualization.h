#pragma once

#define PROJECTM_STATIC_DEFINE 1
#define VIS_DEBUG_LOG 1

#include "aimp_sdk/apiMessages.h"
#include "aimp_sdk/apiTypes.h"
#include "aimp_sdk/apiVisuals.h"
#include "aimp_sdk/IUnknownImpl.h"
#include <projectM-4/projectM.h>
#include <projectM-4/playlist.h>
#include <GL/glew.h>
#include <string>
#include <iostream>

static const TChar VisualizationName[] = TEXT("ProjectM Visualization v1.0b");

class Visualization : public IUnknownImpl<IAIMPExtensionEmbeddedVisualization>
{
private:
	IAIMPCore* core;
	HINSTANCE  inst;

	int activePreset = -1;
	projectm_handle pm = nullptr;
	projectm_playlist_handle presets = nullptr;
	std::string pathPresets;
	std::string pathTextures;
	std::string error;

    HGLRC	gl = 0;
	int		glHeight = 0;
	int		glWidth = 0;
	HWND	wnd = 0;
	HDC		wndDC = 0;
	BOOL	wndRegistered = false;
	RECT	wndRect = {};
	SINGLE  waveform[2 * AIMP_VISUAL_WAVEFORM_MAX] = {};

	void ConfigLoad();
	void ConfigSave();
	HWND GetOwnerWnd();
	void OnClick(BOOL right);
	void OnClosed();
	void OnError(const char* text);
	void UpdateWindowCaption();
private:
	static LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l);
protected:
	virtual BOOL isOurRIID(REFIID riid);

	// Common Information
	virtual DWORD WINAPI GetFlags();
	virtual HRESULT WINAPI GetMaxDisplaySize(INT32* Width, INT32* Height);
	virtual HRESULT WINAPI GetName(IAIMPString** S);

	// Initialization / Finalization
	virtual HRESULT WINAPI Initialize(INT32 Width, INT32 Height);
	virtual void WINAPI Finalize();

	// Basic functionality
	virtual void WINAPI Click(INT32 X, INT32 Y, INT32 Button);
	virtual void WINAPI Draw(HCANVAS Canvas, PAIMPVisualData Data);
	virtual void WINAPI Resize(INT32 NewWidth, INT32 NewHeight);
public:
	Visualization(IAIMPCore* core, HINSTANCE inst);
};