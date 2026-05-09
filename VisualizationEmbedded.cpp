#include "VisualizationEmbedded.h"
//#include "stb_image_write.h"

static const TChar VisualizationName[] = TEXT("ProjectM");

VisualizationEmbedded::VisualizationEmbedded(IAIMPCore* core): VisualizationBase(core)
{}

void WINAPI VisualizationEmbedded::Finalize()
{
	VisualizationBase::Finalize();
	FreeFrameObject();
	if (window)
	{
		glfwDestroyWindow(window);
		window = nullptr;
	}
}

HRESULT WINAPI VisualizationEmbedded::Initialize(INT32 Width, INT32 Height)
{
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);      
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(Width, Height, "ProjectM-Headless-OpenGL", nullptr, nullptr);
	if (window == nullptr) 
	{
		OnError("GLFW failed to create a window");
		Finalize();
		return E_FAIL;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != 0)
	{
		OnError("GLEW failed to initialize");
		Finalize();
		return E_FAIL;
	}

	if (Failed(VisualizationBase::Initialize(Width, Height)))
	{
		Finalize();
		return E_FAIL;
	}

	UpdateDisplayingText();
	return S_OK;
}

void WINAPI VisualizationEmbedded::Draw(HCANVAS Canvas, PAIMPVisualData Data)
{
	DrawCore(Data);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	SetDIBitsToDevice(Canvas, 0, 0, width, height, 0, 0, 0, height, buffer, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
}

void WINAPI VisualizationEmbedded::DrawCore(PAIMPVisualData Data)
{
	// Ensure the objects are ready
	if (window == nullptr || pm == nullptr)
		return;

	// First, it will resize the surface if necessary
	VisualizationBase::DrawCore(Data);

	// Draw to the FBO
	if (fbo != 0) // frame object may be failed to initialize due ResizeSurface
	{
		glfwMakeContextCurrent(window);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, width, height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		projectm_opengl_render_frame(pm);
	}
}

BOOL VisualizationEmbedded::CreateFrameObject(int w, int h)
{
	buffer = new unsigned char[w * h * 4];

	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void VisualizationEmbedded::FreeFrameObject()
{
	if (buffer) 
	{
		delete buffer;
		buffer = nullptr;
	}

	if (fbo)
	{
		glDeleteFramebuffers(1, &fbo);
		fbo = 0;
	}

	if (texture)
	{
		glDeleteTextures(1, &texture);
		texture = 0;
	}
}

HRESULT WINAPI VisualizationEmbedded::GetName(IAIMPString** S)
{
	(*S) = MakeString(VisualizationName);
	return S_OK;
}

void VisualizationEmbedded::ResizeSurface(int w, int h)
{
	FreeFrameObject();
	if (!CreateFrameObject(w, h))
	{
		OnError("Failed to create FBO");
		FreeFrameObject();
	}

	bmi.biSize = sizeof(bmi);
	bmi.biWidth = width;
	bmi.biHeight = -height;
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biCompression = BI_RGB;

	VisualizationBase::ResizeSurface(w, h);
}

//void VisualizationEmbedded::UpdateDisplayingText()
//{
//	if (!error.empty())
//	{
//		SetWindowTextA(wnd, error.data());
//		return;
//	}
//	
//	std::string text("AIMP - ProjectM VisualizationEmbedded");
//	if (presets != nullptr && activePreset >= 0) 
//	{
//		char* current = projectm_playlist_item(presets, activePreset);
//		if (current != nullptr)
//		{
//			std::string path(current);
//			size_t pos = path.find_last_of("\\/");
//			if (pos != std::string::npos)
//				text += " - [" + path.substr(pos + 1) + "]";
//		}
//	}
//	SetWindowTextA(wnd, text.data());
//}