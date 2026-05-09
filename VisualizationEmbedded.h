#pragma once
#include "VisualizationBase.h"

class VisualizationEmbeddedDirectOutput;

class VisualizationEmbedded : public VisualizationBase
{
private:
	GLuint fbo = 0;
	GLuint texture = 0;
	GLFWwindow* window = nullptr;

	void* buffer = nullptr;
	BITMAPINFOHEADER bmi = {};
	IAIMPVisualizationDirectOutput* directOutput = nullptr;

	BOOL CreateFrameObject(int w, int h);
	void FreeFrameObject();
protected:
	virtual void DrawCore(PAIMPVisualData Data);
	virtual void ResizeSurface(int w, int h);

	// IAIMPExtensionEmbeddedVisualization
	virtual void WINAPI Finalize();
	virtual HRESULT WINAPI Initialize(INT32 Width, INT32 Height);
	virtual void WINAPI Draw(HCANVAS Canvas, PAIMPVisualData Data);
	virtual HRESULT WINAPI GetName(IAIMPString** S);

	friend class VisualizationEmbeddedDirectOutput;
public:
	 VisualizationEmbedded(IAIMPCore* core) : VisualizationBase(core) {}
	~VisualizationEmbedded();
	// IUnknown
	virtual HRESULT _unknwncall QueryInterface(REFIID riid, LPVOID* ppvObj);
};

class VisualizationEmbeddedDirectOutput : public ISubInterfaceImpl<VisualizationEmbedded, IAIMPVisualizationDirectOutput>
{
public:
	VisualizationEmbeddedDirectOutput(VisualizationEmbedded* owner) : ISubInterfaceImpl(owner) {}
	// IAIMPVisualizationDirectOutput
	virtual void WINAPI Draw(RGBQUAD* Buffer, PAIMPVisualData Data);
};