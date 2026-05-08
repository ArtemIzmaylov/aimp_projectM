// dllmain.cpp : Defines the entry point for the DLL application.
#include "framework.h"
#include "aimp_sdk/apiPlugin.h"
#include "aimp_sdk/apiTypes.h"
#include "aimp_sdk/IUnknownImpl.h"
#include "Visualization.h"

static HINSTANCE mInstance;
static const TChar Author[] = TEXT("Artem Izmaylov");
static const TChar Description[] = TEXT("The most advanced open-source music visualizer");
static const TChar Name[] = TEXT("ProjectM Visualization v1.0b");

class Plugin : public IUnknownImpl<IAIMPPlugin>
{
    PChar WINAPI InfoGet(int Index)
    {
        switch (Index)
        {
        case AIMP_PLUGIN_INFO_NAME:
            return const_cast<PChar>(Name);
        case AIMP_PLUGIN_INFO_AUTHOR:
            return const_cast<PChar>(Author);
        case AIMP_PLUGIN_INFO_SHORT_DESCRIPTION:
            return const_cast<PChar>(Description);
        }
        return nullptr;
    }

    DWORD WINAPI InfoGetCategories()
    {
        return AIMP_PLUGIN_CATEGORY_VISUALS;
    }

    HRESULT WINAPI Initialize(IAIMPCore* Core)
    {
        Core->RegisterExtension(IID_IAIMPServiceVisualizations, new Visualization(Core, mInstance));
        return S_OK;
    }

    HRESULT WINAPI Finalize()
    {
        return S_OK;
    }

    void WINAPI SystemNotification(int NotifyID, IUnknown* Data) {}
};


HRESULT __declspec(dllexport) WINAPI AIMPPluginGetHeader(IAIMPPlugin** Header)
{
    (*Header) = new Plugin();
    (*Header)->AddRef();
    return S_OK;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    mInstance = hModule;
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

