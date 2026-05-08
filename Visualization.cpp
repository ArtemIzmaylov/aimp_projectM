#include "Visualization.h"

const wchar_t* CLASS_NAME = L"OpenGLProjectMWindow";
const TChar KeyFullscreen[] = TEXT("ProjectM\\Fullscreen");
const TChar KeyFormLeft[]   = TEXT("ProjectM\\WindowX");
const TChar KeyFormTop[]    = TEXT("ProjectM\\WindowY");
const TChar KeyFormHeight[] = TEXT("ProjectM\\WindowHeight");
const TChar KeyFormWidth[]  = TEXT("ProjectM\\WindowWidth");

IAIMPString* makeString(IAIMPCore* core, const TChar* text)
{
	IAIMPString* string;
	if (SUCCEEDED(core->CreateObject(IID_IAIMPString, reinterpret_cast<void**>(&string))))
	{
		string->SetData((PChar)text, _clen(text));
		return string;
	}
	return nullptr;
}

Visualization::Visualization(IAIMPCore* core, HINSTANCE inst)
{
	this->core = core;
	this->inst = inst;

	std::string currentPath(MAX_PATH, '\0');
	DWORD length = GetCurrentDirectoryA(MAX_PATH, currentPath.data());
	currentPath.resize(length);
	pathPresets  = currentPath + "\\Presets\\";
	pathTextures = currentPath + "\\Textures\\";
	error.clear();
}

BOOL Visualization::isOurRIID(REFIID riid)
{
	return EqualGUID(riid, IID_IAIMPExtensionEmbeddedVisualization);
}

DWORD __stdcall Visualization::GetFlags()
{
	return 
		AIMP_VISUAL_FLAGS_NOT_SUSPEND |
		AIMP_VISUAL_FLAGS_RQD_DATA_WAVEFORM;
}

HRESULT __stdcall Visualization::GetMaxDisplaySize(INT32* Width, INT32* Height)
{
	return E_NOTIMPL;
}

HRESULT __stdcall Visualization::GetName(IAIMPString** S)
{
	IAIMPString* result = makeString(core, VisualizationName);
	if (result == nullptr)
		return E_FAIL;
	(*S) = result;
	return S_OK;
}

HRESULT __stdcall Visualization::Initialize(INT32 Width, INT32 Height)
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

	wnd = CreateWindowEx(WS_EX_TOOLWINDOW, CLASS_NAME, nullptr, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, GetOwnerWnd(), nullptr, inst, nullptr);
	if (wnd == 0)
	{
		OnError("Failed to create Window");
		return E_FAIL;
	}
	SetProp(wnd, L"SELF", HANDLE(this));
	ConfigLoad();
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

	pm = projectm_create();
	if (pm == nullptr)
	{
		OnError("Failed to initialize ProjectM");
		Finalize();
		return E_HANDLE;
	}

	error.clear();
	const char* path = pathTextures.data();
	projectm_set_texture_search_paths(pm, &path, 1);

	projectm_set_fps(pm, 30);
	projectm_set_beat_sensitivity(pm, 1.0);
	projectm_set_aspect_correction(pm, true);

	presets = projectm_playlist_create(pm);
	if (presets != nullptr)
	{
		projectm_playlist_insert_path(presets, pathPresets.data(), 0, true, true);
		int presetCount = projectm_playlist_size(presets);
		if (presetCount > 0)
		{
			projectm_playlist_set_position(presets, rand() % presetCount, true);
			projectm_playlist_set_shuffle(presets, true);
			projectm_set_preset_duration(pm, 33);
		}
	}

	UpdateWindowCaption();
#pragma endregion

	glHeight = 0;
	glWidth = 0;
	return S_OK;
}

void __stdcall Visualization::Finalize()
{
	ConfigSave();

	if (presets)
	{
		projectm_playlist_destroy(presets);
		presets = nullptr;
	}

	if (pm) 
	{
		projectm_destroy(pm);
		pm = nullptr;
	}

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

	error.clear();
	glHeight = 0;
	glWidth = 0;
}

void __stdcall Visualization::Click(INT32 X, INT32 Y, INT32 Button)
{
	BringWindowToTop(wnd);
}

void __stdcall Visualization::Draw(HCANVAS Canvas, PAIMPVisualData Data)
{
	// TODO: draw stub
	if (wndDC == 0)
		return;
	if (wglMakeCurrent(wndDC, gl))
	{
		GetClientRect(wnd, &wndRect);
		if (wndRect.right - wndRect.left != glWidth ||
			wndRect.bottom - wndRect.top != glHeight)
		{
			glHeight = wndRect.bottom - wndRect.top;
			glWidth = wndRect.right - wndRect.left;
			if (pm != nullptr)
				projectm_set_window_size(pm, glWidth, glHeight);
		}

		glViewport(0, 0, glWidth, glHeight);
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

		if (pm != nullptr)
		{
			int j = 0;
			for (int i = 0; i < AIMP_VISUAL_WAVEFORM_MAX; i++)
			{
				waveform[j++] = Data->WaveForm[0][i];
				waveform[j++] = Data->WaveForm[1][i];
			}
			projectm_pcm_add_float(pm, &waveform[0], AIMP_VISUAL_WAVEFORM_MAX, PROJECTM_STEREO);
			projectm_opengl_render_frame(pm);
		}

		SwapBuffers(wndDC);
	}
	else
		if (error.empty()) 
		{
			error = std::to_string(GetLastError());
			UpdateWindowCaption();
		}
}

void Visualization::ConfigLoad()
{
	int showCmd = SW_SHOW;
	IAIMPServiceConfig* config;
	if (Succeeded(core->QueryInterface(IID_IAIMPConfig, reinterpret_cast<void**>(&config))))
	{
		int x, y, w, h = 0; 
		config->GetValueAsInt32(makeString(core, KeyFormLeft),   &x);
		config->GetValueAsInt32(makeString(core, KeyFormTop),    &y);
		config->GetValueAsInt32(makeString(core, KeyFormWidth),  &w);
		config->GetValueAsInt32(makeString(core, KeyFormHeight), &h);
		if (w > 0 && h > 0)
			SetWindowPos(wnd, 0, x, y, w, h, SWP_NOZORDER);

		int v = 0;
		config->GetValueAsInt32(makeString(core, KeyFullscreen), &v);
		if (v != 0)
			showCmd = SW_MAXIMIZE;
	}
	ShowWindow(wnd, showCmd);
}

void Visualization::ConfigSave()
{
	if (wnd == 0) return;

	IAIMPServiceConfig* config;
	if (Succeeded(core->QueryInterface(IID_IAIMPConfig, reinterpret_cast<void**>(&config))))
	{
		WINDOWPLACEMENT pos = {};
		pos.length = sizeof(pos);
		if (GetWindowPlacement(wnd, &pos))
		{
			config->SetValueAsInt32(makeString(core, KeyFormLeft),   pos.rcNormalPosition.left);
			config->SetValueAsInt32(makeString(core, KeyFormTop),	 pos.rcNormalPosition.top);
			config->SetValueAsInt32(makeString(core, KeyFormWidth),  pos.rcNormalPosition.right - pos.rcNormalPosition.left);
			config->SetValueAsInt32(makeString(core, KeyFormHeight), pos.rcNormalPosition.bottom - pos.rcNormalPosition.top);
		}
		config->SetValueAsInt32(makeString(core, KeyFullscreen), IsZoomed(wnd));
	}
}

HWND Visualization::GetOwnerWnd()
{
	HWND ownerWnd = 0;
	IAIMPServiceMessageDispatcher* dispatcher = nullptr;
	if (Succeeded(core->QueryInterface(IID_IAIMPServiceMessageDispatcher, reinterpret_cast<void**>(&dispatcher))))
		dispatcher->Send(AIMP_MSG_PROPERTY_HWND, AIMP_MPH_MAINFORM, &ownerWnd);
	return ownerWnd;
}

void Visualization::OnClick(BOOL right)
{
	if (right)
	{
		//LONG style = GetWindowLong(wnd, GWL_STYLE);
		//if (style & WS_OVERLAPPEDWINDOW)
		//	style = WS_POPUP;
		//else
		//	style = WS_OVERLAPPEDWINDOW;

		//SetWindowLong(wnd, GWL_STYLE, style);
	}
	else
		if (presets != nullptr)
			projectm_playlist_play_next(presets, true);
}

void Visualization::OnClosed()
{
	IAIMPServiceMessageDispatcher* dispatcher = nullptr;
	if (Succeeded(core->QueryInterface(IID_IAIMPServiceMessageDispatcher, reinterpret_cast<void**>(&dispatcher))))
		dispatcher->Send(AIMP_MSG_CMD_VISUAL_STOP, 0, nullptr);
}

void Visualization::OnError(const char* text)
{
	error = text;
#ifdef VIS_DEBUG_LOG
	OutputDebugStringA(text);
#endif 
	UpdateWindowCaption();
}

void __stdcall Visualization::Resize(INT32 NewWidth, INT32 NewHeight)
{
	// do nothing
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
		case WM_GETMINMAXINFO:
			if (l != 0)
			{
				((MINMAXINFO*)l)->ptMinTrackSize.x = 200;
				((MINMAXINFO*)l)->ptMinTrackSize.y = 200;
				return 0;
			}
			break;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			self = (Visualization*)GetProp(wnd, L"SELF");
			if (self != nullptr)
				self->OnClick(msg == WM_RBUTTONUP);
			break;

		case WM_CLOSE:
			self = (Visualization*)GetProp(wnd, L"SELF");
			if (self != nullptr)
				self->OnClosed();
			break;
	}
	return DefWindowProc(wnd, msg, w, l);
}