#include "VisualizationBase.h"

VisualizationBase::VisualizationBase(IAIMPCore* core)
{
	this->core = core;
	std::string currentPath = std::filesystem::current_path().string();
	this->pathPresets  = (std::filesystem::path(currentPath) / "Presets").string();
	this->pathTextures = (std::filesystem::path(currentPath) / "Textures").string();
	error.clear();
}

void VisualizationBase::ConfigLoad()
{
	IAIMPServiceConfig* config = nullptr;
	if (Succeeded(core->QueryInterface(IID_IAIMPConfig, reinterpret_cast<void**>(&config))))
		ConfigLoad(config);
}

void VisualizationBase::ConfigLoad(IAIMPConfig* config)
{
	// do nothing
}

void VisualizationBase::ConfigSave(IAIMPConfig* config)
{
	// do nothing
}

IAIMPString* VisualizationBase::MakeString(const TChar* text)
{
	IAIMPString* string;
	if (SUCCEEDED(core->CreateObject(IID_IAIMPString, reinterpret_cast<void**>(&string))))
	{
		string->SetData((PChar)text, _clen(text));
		return string;
	}
	return nullptr;
}

BOOL VisualizationBase::isOurRIID(REFIID riid)
{
	return EqualGUID(riid, IID_IAIMPExtensionEmbeddedVisualization);
}

void WINAPI VisualizationBase::Finalize()
{
	IAIMPServiceConfig* config = nullptr;
	if (Succeeded(core->QueryInterface(IID_IAIMPConfig, reinterpret_cast<void**>(&config))))
		ConfigSave(config);

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

	error.clear();
	width = height = 0;
}

HRESULT WINAPI VisualizationBase::Initialize(INT32 Width, INT32 Height)
{
	pm = projectm_create();
	if (pm == nullptr)
	{
		OnError("Failed to initialize ProjectM");
		Finalize();
		return E_HANDLE;
	}

	error.clear();
	width = height = 0;

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

	Resize(Width, Height);
	return S_OK;
}

DWORD WINAPI VisualizationBase::GetFlags()
{
	return
		AIMP_VISUAL_FLAGS_NOT_SUSPEND |
		AIMP_VISUAL_FLAGS_RQD_DATA_WAVEFORM;
}

HRESULT WINAPI VisualizationBase::GetMaxDisplaySize(INT32* Width, INT32* Height)
{
	return E_NOTIMPL;
}

void WINAPI VisualizationBase::Click(INT32 X, INT32 Y, INT32 Button)
{
	if (presets != nullptr)
		projectm_playlist_play_next(presets, true);
}

void WINAPI VisualizationBase::DrawCore(PAIMPVisualData Data)
{
	if (pm == nullptr)
		return;
	if (pendingHeight != height || pendingWidth != width)
	{
		width = pendingWidth;
		height = pendingHeight;
		ResizeSurface(width, height);
	}

	int j = 0;
	for (int i = 0; i < AIMP_VISUAL_WAVEFORM_MAX; i++)
	{
		waveform[j++] = Data->WaveForm[0][i];
		waveform[j++] = Data->WaveForm[1][i];
	}
	projectm_pcm_add_float(pm, &waveform[0], AIMP_VISUAL_WAVEFORM_MAX, PROJECTM_STEREO);

	if (presets != nullptr)
	{
		int index = projectm_playlist_get_position(presets);
		if (index != activePreset)
		{
			activePreset = index;
			UpdateDisplayingText();
		}
	}
}

void VisualizationBase::OnError(const char* text)
{
	error = text;
#ifdef PROJECTM_VERBOSE_OUTPUT
	OutputDebugStringA(text);
#endif // PROJECTM_VERBOSE_OUTPUT
	UpdateDisplayingText();
}

void WINAPI VisualizationBase::Resize(INT32 NewWidth, INT32 NewHeight)
{
	pendingWidth = NewWidth;
	pendingHeight = NewHeight;
}

void VisualizationBase::ResizeSurface(int w, int h)
{
	projectm_set_window_size(pm, width, height);
}

void VisualizationBase::UpdateDisplayingText()
{
	// do nothing
}
