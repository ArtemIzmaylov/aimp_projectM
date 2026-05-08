#include "Visualization.h"

static const TChar KeyFullscreen[] = TEXT("ProjectM\\Fullscreen");
static const TChar KeyFormLeft[]   = TEXT("ProjectM\\WindowX");
static const TChar KeyFormTop[]    = TEXT("ProjectM\\WindowY");
static const TChar KeyFormHeight[] = TEXT("ProjectM\\WindowHeight");
static const TChar KeyFormWidth[]  = TEXT("ProjectM\\WindowWidth");
static const TChar VisualizationName[] = TEXT("ProjectM");

Visualization::Visualization(IAIMPCore* core, HINSTANCE inst)
	: VisualizationWindowBased(core, inst)
{}

HRESULT WINAPI Visualization::Initialize(INT32 Width, INT32 Height)
{
	if (!InitWindow(0, 0, 800, 600))
	{
		Finalize();
		return E_FAIL;
	}

	VisualizationBase::ConfigLoad();
	UpdateWindow(wnd);

	if (!InitOpenGL())
	{
		Finalize();
		return E_FAIL;
	}

	if (Failed(VisualizationWindowBased::Initialize(Width, Height)))
	{
		Finalize();
		return E_FAIL;
	}

	UpdateDisplayingText();
	return S_OK;
}

void WINAPI Visualization::Click(INT32 X, INT32 Y, INT32 Button)
{
	if (GetActiveWindow() == wnd)
		VisualizationBase::Click(X, Y, Button);
	else
		BringWindowToTop(wnd);
}

void WINAPI Visualization::Draw(HCANVAS Canvas, PAIMPVisualData Data)
{
	// TODO: draw stub
	if (wndDC == 0)
		return;
	if (wglMakeCurrent(wndDC, gl))
	{
		VisualizationWindowBased::Draw(Canvas, Data);
		if (pm != nullptr)
			projectm_opengl_render_frame(pm);
		SwapBuffers(wndDC);
	}
	else
		if (error.empty()) 
		{
			error = std::to_string(GetLastError());
			UpdateDisplayingText();
		}
}

void Visualization::ConfigLoad(IAIMPConfig* config)
{
	int showCmd = SW_SHOW;
	int x, y, w, h = 0; 
	config->GetValueAsInt32(MakeString(KeyFormLeft),   &x);
	config->GetValueAsInt32(MakeString(KeyFormTop),    &y);
	config->GetValueAsInt32(MakeString(KeyFormWidth),  &w);
	config->GetValueAsInt32(MakeString(KeyFormHeight), &h);
	if (w > 0 && h > 0)
		SetWindowPos(wnd, 0, x, y, w, h, SWP_NOZORDER);

	int v = 0;
	config->GetValueAsInt32(MakeString(KeyFullscreen), &v);
	if (v != 0)
		showCmd = SW_MAXIMIZE;
	ShowWindow(wnd, showCmd);
}

void Visualization::ConfigSave(IAIMPConfig* config)
{
	if (wnd == 0) return;
	WINDOWPLACEMENT pos = {};
	pos.length = sizeof(pos);
	if (GetWindowPlacement(wnd, &pos))
	{
		config->SetValueAsInt32(MakeString(KeyFormLeft),   pos.rcNormalPosition.left);
		config->SetValueAsInt32(MakeString(KeyFormTop),    pos.rcNormalPosition.top);
		config->SetValueAsInt32(MakeString(KeyFormWidth),  pos.rcNormalPosition.right - pos.rcNormalPosition.left);
		config->SetValueAsInt32(MakeString(KeyFormHeight), pos.rcNormalPosition.bottom - pos.rcNormalPosition.top);
	}
	config->SetValueAsInt32(MakeString(KeyFullscreen), IsZoomed(wnd));
}

HRESULT WINAPI Visualization::GetName(IAIMPString** S)
{
	(*S) = MakeString(VisualizationName);
	return S_OK;
}

void Visualization::UpdateDisplayingText()
{
	if (!error.empty())
	{
		SetWindowTextA(wnd, error.data());
		return;
	}
	
	std::string text("AIMP - ProjectM Visualization");
	if (presets != nullptr && activePreset >= 0) 
	{
		char* current = projectm_playlist_item(presets, activePreset);
		if (current != nullptr)
		{
			std::string path(current);
			size_t pos = path.find_last_of("\\/");
			if (pos != std::string::npos)
				text += " - [" + path.substr(pos + 1) + "]";
		}
	}
	SetWindowTextA(wnd, text.data());
}