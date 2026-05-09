#pragma once

#define PROJECTM_STATIC_DEFINE 1
#define PROJECTM_VERBOSE_OUTPUT 1

#include "aimp_sdk/apiMessages.h"
#include "aimp_sdk/apiTypes.h"
#include "aimp_sdk/apiVisuals.h"
#include "aimp_sdk/IUnknownImpl.h"
#include <projectM-4/projectM.h>
#include <projectM-4/playlist.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <filesystem>

class VisualizationBase : public IUnknownImpl<IAIMPExtensionEmbeddedVisualization>
{
protected:
	IAIMPCore* core;

	int activePreset = -1;
	int pendingHeight = 0;
	int pendingWidth = 0;
	int height = 0;
	int width = 0;
	projectm_handle pm = nullptr;
	projectm_playlist_handle presets = nullptr;

	std::string error;
	std::string pathPresets;
	std::string pathTextures;

	SINGLE  waveform[2 * AIMP_VISUAL_WAVEFORM_MAX] = {};

	IAIMPString* MakeString(const TChar* text);
	void OnError(const char* text);
	virtual void ResizeSurface(int w, int h);
protected:
	virtual void DrawCore(PAIMPVisualData Data);

	// IUnknown
	virtual BOOL isOurRIID(REFIID riid);

	// IAIMPExtensionEmbeddedVisualization
	virtual void WINAPI Finalize();
	virtual HRESULT WINAPI Initialize(INT32 Width, INT32 Height);
	virtual DWORD WINAPI GetFlags();
	virtual HRESULT WINAPI GetMaxDisplaySize(INT32* Width, INT32* Height);
	virtual HRESULT WINAPI GetName(IAIMPString** S) = 0;
	virtual void WINAPI Click(INT32 X, INT32 Y, INT32 Button);
	virtual void WINAPI Draw(HCANVAS Canvas, PAIMPVisualData Data) = 0;
	virtual void WINAPI Resize(INT32 NewWidth, INT32 NewHeight);
public:
	VisualizationBase(IAIMPCore* core);
};