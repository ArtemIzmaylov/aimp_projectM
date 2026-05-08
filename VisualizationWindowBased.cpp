#include "VisualizationWindowBased.h"

static const wchar_t* CLASS_NAME = L"AIMPProjectMOpenGLWindow";

VisualizationWindowBased::VisualizationWindowBased(IAIMPCore* core, HINSTANCE inst)
	: VisualizationBase(core, inst)
{
}

BOOL VisualizationWindowBased::InitOpenGL()
{
	wndDC = GetDC(wnd);
	if (wndDC == 0)
	{
		OnError("Failed to obtain DC");
		return FALSE;
	}

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
		return FALSE;
	}

	if (!SetPixelFormat(wndDC, pixelFormat, &pfd))
	{
		OnError("Failed to set pixel format");
		return FALSE;
	}

	gl = wglCreateContext(wndDC);
	if (!gl)
	{
		OnError("Failed to create OpenGL context");
		return FALSE;
	}

	if (!wglMakeCurrent(wndDC, gl))
	{
		OnError("Failed to switch to OpenGL context");
		return FALSE;
	}

	if (glewInit() != 0)
	{
		OnError("Failed to initialize GLEW library");
		return FALSE;
	}

	return TRUE;
}

BOOL VisualizationWindowBased::InitWindow(int x, int y, int w, int h)
{
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
			return FALSE;
		}
	}

	HWND ownerWnd = 0;
	IAIMPServiceMessageDispatcher* dispatcher = nullptr;
	if (Succeeded(core->QueryInterface(IID_IAIMPServiceMessageDispatcher, reinterpret_cast<void**>(&dispatcher))))
		dispatcher->Send(AIMP_MSG_PROPERTY_HWND, AIMP_MPH_MAINFORM, &ownerWnd);

	wnd = CreateWindowEx(WS_EX_TOOLWINDOW, CLASS_NAME, nullptr, WS_OVERLAPPEDWINDOW, x, y, w, h, ownerWnd, nullptr, inst, nullptr);
	if (wnd == 0)
	{
		OnError("Failed to create Window");
		return FALSE;
	}

	SetProp(wnd, L"SELF", HANDLE(this));
	return TRUE;
}

void WINAPI VisualizationWindowBased::Draw(HCANVAS Canvas, PAIMPVisualData Data)
{
	VisualizationBase::Draw(Canvas, Data);

	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void WINAPI VisualizationWindowBased::Finalize()
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

void VisualizationWindowBased::OnClosed()
{
	IAIMPServiceMessageDispatcher* dispatcher = nullptr;
	if (Succeeded(core->QueryInterface(IID_IAIMPServiceMessageDispatcher, reinterpret_cast<void**>(&dispatcher))))
		dispatcher->Send(AIMP_MSG_CMD_VISUAL_STOP, 0, nullptr);
}

LRESULT CALLBACK VisualizationWindowBased::WndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
	VisualizationWindowBased* self;
	switch (msg)
	{
		case WM_SIZE:
			self = (VisualizationWindowBased*)GetProp(wnd, L"SELF");
			if (self != nullptr)
			{
				RECT rect = {};
				GetClientRect(wnd, &rect);
				self->pendingHeight = rect.bottom - rect.top;
				self->pendingWidth = rect.right - rect.left;
			}
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
			self = (VisualizationWindowBased*)GetProp(wnd, L"SELF");
			if (self != nullptr)
				self->Click(0, 0, AIMP_VISUAL_CLICK_BUTTON_LEFT);
			break;

		case WM_CLOSE:
			self = (VisualizationWindowBased*)GetProp(wnd, L"SELF");
			if (self != nullptr)
				self->OnClosed();
			break;
	}
	return DefWindowProc(wnd, msg, w, l);
}