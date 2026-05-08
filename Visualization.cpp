#include "Visualization.h"

static const wchar_t* CLASS_NAME = L"OpenGLProjectMWindow";
static const TChar KeyFullscreen[] = TEXT("ProjectM\\Fullscreen");
static const TChar KeyFormLeft[]   = TEXT("ProjectM\\WindowX");
static const TChar KeyFormTop[]    = TEXT("ProjectM\\WindowY");
static const TChar KeyFormHeight[] = TEXT("ProjectM\\WindowHeight");
static const TChar KeyFormWidth[]  = TEXT("ProjectM\\WindowWidth");
static const TChar VisualizationName[] = TEXT("ProjectM");

Visualization::Visualization(IAIMPCore* core, HINSTANCE inst): VisualizationBase(core, inst)
{
	std::string currentPath(MAX_PATH, '\0');
	DWORD length = GetCurrentDirectoryA(MAX_PATH, currentPath.data());
	currentPath.resize(length);
	pathPresets  = currentPath + "\\Presets\\";
	pathTextures = currentPath + "\\Textures\\";
}

HRESULT WINAPI Visualization::Initialize(INT32 Width, INT32 Height)
{
#pragma region "Create the Window"

	if (!wndRegistered)
	{
		WNDCLASS wc = {};
		wc.hInstance = inst;
		wc.lpszClassName = CLASS_NAME;
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpfnWndProc = WndProc;
		wc.style = CS_OWNDC; // Критично для OpenGL
		if (RegisterClass(&wc))
			wndRegistered = true;
		else
		{
			OnError("Failed to register WindowClass");
			return E_FAIL;
		}
	}

	wndRectDirty = true;
	wnd = CreateWindowEx(WS_EX_TOOLWINDOW, CLASS_NAME, nullptr, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, GetOwnerWnd(), nullptr, inst, nullptr);
	if (wnd == 0)
	{
		OnError("Failed to create Window");
		return E_FAIL;
	}
	SetProp(wnd, L"SELF", HANDLE(this));
	VisualizationBase::ConfigLoad();
	UpdateWindow(wnd);

	wndDC = GetDC(wnd);
	if (wndDC == 0)
	{
		OnError("Failed to obtain Window's DC");
		Finalize();
		return E_FAIL;
	}

#pragma endregion

#pragma region "Create the OpenGL context"

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24, // Цвет: 24 бита
		0,0,0,0,0,0,
		0,0,0,0,0,0,0,
		24, // Глубина буфера
		0,0, PFD_MAIN_PLANE, 0,0,0,0
	};

	int pixelFormat = ChoosePixelFormat(wndDC, &pfd);
	if (pixelFormat == 0)
	{
		OnError("Failed to obtain pixel format");
		Finalize();
		return E_FAIL;
	}

	if (!SetPixelFormat(wndDC, pixelFormat, &pfd))
	{
		OnError("Failed to set pixel format");
		Finalize();
		return E_FAIL;
	}

	gl = wglCreateContext(wndDC);
	if (!gl)
	{
		OnError("Failed to create OpenGL context");
		Finalize();
		return E_FAIL;
	}

	if (!wglMakeCurrent(wndDC, gl))
	{
		OnError("Failed to switch to OpenGL context");
		Finalize();
		return E_FAIL;
	}

#pragma endregion

#pragma region "Create ProjectM handle"

	if (glewInit() != 0)
	{
		OnError("Failed to initialize GLEW library");
		Finalize();
		return E_FAIL;
	}

#pragma endregion

	if (Succeeded(VisualizationBase::Initialize(Width, Height)))
	{
		UpdateWindowCaption();
		return S_OK;
	}
	return E_FAIL;
}

void WINAPI Visualization::Finalize()
{
	VisualizationBase::Finalize();

	if (gl) 
	{
		wglMakeCurrent(0, 0);
		wglDeleteContext(gl);
		gl = 0;
	}

	if (wndDC) 
	{
		ReleaseDC(wnd, wndDC);
		wndDC = 0;
	}

	if (wnd) 
	{
		DestroyWindow(wnd);
		wnd = 0;
	}
}

void WINAPI Visualization::Click(INT32 X, INT32 Y, INT32 Button)
{
	BringWindowToTop(wnd);
}

void WINAPI Visualization::Draw(HCANVAS Canvas, PAIMPVisualData Data)
{
	// TODO: draw stub
	if (wndDC == 0)
		return;
	if (wglMakeCurrent(wndDC, gl))
	{
		if (wndRectDirty || width == 0 || height == 0)
		{
			GetClientRect(wnd, &wndRect);
			VisualizationBase::Resize(
				wndRect.right - wndRect.left,
				wndRect.bottom - wndRect.top);
		}

		glViewport(0, 0, width, height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (presets != nullptr)
		{
			int index = projectm_playlist_get_position(presets);
			if (index != activePreset)
			{
				activePreset = index;
				UpdateWindowCaption();
			}
		}

		PushAudioData(Data);

		if (pm != nullptr)
			projectm_opengl_render_frame(pm);

		SwapBuffers(wndDC);
	}
	else
		if (error.empty()) 
		{
			error = std::to_string(GetLastError());
			UpdateWindowCaption();
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

HWND Visualization::GetOwnerWnd()
{
	HWND ownerWnd = 0;
	IAIMPServiceMessageDispatcher* dispatcher = nullptr;
	if (Succeeded(core->QueryInterface(IID_IAIMPServiceMessageDispatcher, reinterpret_cast<void**>(&dispatcher))))
		dispatcher->Send(AIMP_MSG_PROPERTY_HWND, AIMP_MPH_MAINFORM, &ownerWnd);
	return ownerWnd;
}

HRESULT WINAPI Visualization::GetName(IAIMPString** S)
{
	(*S) = MakeString(VisualizationName);
	return S_OK;
}

void WINAPI Visualization::Resize(INT32 NewWidth, INT32 NewHeight)
{
	// do nothing here
}

void Visualization::OnClosed()
{
	IAIMPServiceMessageDispatcher* dispatcher = nullptr;
	if (Succeeded(core->QueryInterface(IID_IAIMPServiceMessageDispatcher, reinterpret_cast<void**>(&dispatcher))))
		dispatcher->Send(AIMP_MSG_CMD_VISUAL_STOP, 0, nullptr);
}

void Visualization::OnError(const char* text)
{
	VisualizationBase::OnError(text);
	UpdateWindowCaption();
}

void Visualization::UpdateWindowCaption()
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

LRESULT CALLBACK Visualization::WndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
	Visualization* self;
	switch (msg) 
	{
		case WM_SIZE:
			self = (Visualization*)GetProp(wnd, L"SELF");
			if (self != nullptr)
				self->wndRectDirty = true;
			break;

		case WM_GETMINMAXINFO:
			if (l != 0)
			{
				((MINMAXINFO*)l)->ptMinTrackSize.x = 200;
				((MINMAXINFO*)l)->ptMinTrackSize.y = 200;
				return 0;
			}
			break;

		case WM_LBUTTONUP:
			self = (Visualization*)GetProp(wnd, L"SELF");
			if (self != nullptr)
				self->Click(0, 0, AIMP_VISUAL_CLICK_BUTTON_LEFT);
			break;

		case WM_CLOSE:
			self = (Visualization*)GetProp(wnd, L"SELF");
			if (self != nullptr)
				self->OnClosed();
			break;
	}
	return DefWindowProc(wnd, msg, w, l);
}