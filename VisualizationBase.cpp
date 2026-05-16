#include "VisualizationBase.h"

VisualizationBase::VisualizationBase(IAIMPCore* core)
{
	this->core = core;
	std::string currentPath = std::filesystem::current_path().string();
	this->pathPresets  = (std::filesystem::path(currentPath) / "Presets").string();
	this->pathTextures = (std::filesystem::path(currentPath) / "Textures").string();

	// The Log Service was introduced in v6.0 Beta 4
	if (Failed(core->QueryInterface(IID_IAIMPServiceLog, reinterpret_cast<void**>(&logger))))
		logger = nullptr;
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

	width = height = 0;
}

HRESULT WINAPI VisualizationBase::Initialize(INT32 Width, INT32 Height)
{
	pm = projectm_create();
	if (pm == nullptr)
	{
		LogEntry("Failed to initialize ProjectM");
		Finalize();
		return E_HANDLE;
	}

	width = height = 0;

	const char* path = pathTextures.data();
	projectm_set_texture_search_paths(pm, &path, 1);

	projectm_set_fps(pm, 30);
	projectm_set_mesh_size(pm, 48, 32);
	projectm_set_beat_sensitivity(pm, 1.0);
	projectm_set_aspect_correction(pm, true);
	projectm_set_soft_cut_duration(pm, 3.0);
	projectm_set_hard_cut_enabled(pm, false);
	projectm_set_hard_cut_duration(pm, 20.0);
	projectm_set_hard_cut_sensitivity(pm, 1.0);
	

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
	return AIMP_VISUAL_FLAGS_RQD_DATA_WAVEFORM;
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

void VisualizationBase::DrawCore(PAIMPVisualData Data)
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
		waveform[j++] = max(min(Data->WaveForm[0][i], 1.0f), -1.0f);
		waveform[j++] = max(min(Data->WaveForm[1][i], 1.0f), -1.0f);
	}
	projectm_pcm_add_float(pm, &waveform[0], AIMP_VISUAL_WAVEFORM_MAX, PROJECTM_STEREO);

	if (presets != nullptr)
	{
		int index = projectm_playlist_get_position(presets);
		if (index != activePreset)
		{
			activePreset = index;
			char* active = projectm_playlist_item(presets, activePreset);
			if (active != nullptr)
			{
				std::string path(active);
				size_t pos = path.find_last_of("\\/");
				if (pos != std::string::npos)
				{
					path = "preset: " + path.substr(pos + 1);
					LogEntry(path.data());
				}
				projectm_playlist_free_string(active);
			}
			else
				LogEntry("preset: idle");
		}
	}
}

void VisualizationBase::LogEntry(const char* text)
{
#ifdef PROJECTM_DEBUG_OUTPUT
	OutputDebugStringA(text);
#endif 
	if (logger != nullptr)
		logger->Add1((char*)text, 0);
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