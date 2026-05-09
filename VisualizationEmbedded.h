#pragma once
#include "VisualizationBase.h"

class VisualizationEmbedded : public VisualizationBase
{
private:
	GLuint fbo = 0;
	GLuint texture = 0;
	GLFWwindow* window = nullptr;

	void* buffer = nullptr;
	BITMAPINFOHEADER bmi = {};

	BOOL CreateFrameObject(int w, int h);
	void FreeFrameObject();
protected:
	virtual void ResizeSurface(int w, int h);

	// IAIMPExtensionEmbeddedVisualization
	virtual void WINAPI Finalize();
	virtual HRESULT WINAPI Initialize(INT32 Width, INT32 Height);
	virtual void WINAPI Draw(HCANVAS Canvas, PAIMPVisualData Data);
	virtual void WINAPI DrawCore(PAIMPVisualData Data);
	virtual HRESULT WINAPI GetName(IAIMPString** S);
public:
	VisualizationEmbedded(IAIMPCore* core);
};