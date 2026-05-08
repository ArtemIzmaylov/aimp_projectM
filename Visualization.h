#pragma once
#include "VisualizationWindowBased.h"

class Visualization : public VisualizationWindowBased
{
protected:
	virtual void ConfigLoad(IAIMPConfig* config);
	virtual void ConfigSave(IAIMPConfig* config);
	virtual void UpdateDisplayingText();

	// IAIMPExtensionEmbeddedVisualization
	virtual HRESULT WINAPI Initialize(INT32 Width, INT32 Height);
	virtual void WINAPI Click(INT32 X, INT32 Y, INT32 Button);
	virtual void WINAPI Draw(HCANVAS Canvas, PAIMPVisualData Data);
	virtual HRESULT WINAPI GetName(IAIMPString** S);
public:
	Visualization(IAIMPCore* core, HINSTANCE inst);
};