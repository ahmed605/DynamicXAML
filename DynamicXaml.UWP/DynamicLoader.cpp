#include "pch.h"
#include "DynamicLoader.h"
#if __has_include("DynamicLoader.g.cpp")
#include "DynamicLoader.g.cpp"
#endif

#include <Windows.h>
#include <detours/detours.h>

namespace winrt::DynamicXaml::UWP::implementation
{
	bool DynamicLoader::s_initialized = false;
    decltype(&DynamicLoader::LoadComponentWithResourceLocationHook) s_LoadComponentWithResourceLocation = nullptr;

    void DynamicLoader::LoadPri(winrt::Windows::Storage::StorageFile const& priFile)
    {
        if (!priFile)
            throw winrt::hresult_invalid_argument(L"priFile cannot be null.");

		if (!EnsureInitialized())
            throw winrt::hresult_error(E_FAIL, L"Failed to initialize DynamicLoader.");

		ResourceManager::Current().LoadPriFiles({ priFile });
    }

    HRESULT WINAPI DynamicLoader::LoadComponentWithResourceLocationHook(void* pThis, IInspectable* pComponent, void* resourceLocator, ComponentResourceLocation componentResourceLocation)
    {
		HRESULT hr = S_OK;
        if (FAILED(hr = s_LoadComponentWithResourceLocation(pThis, pComponent, resourceLocator, componentResourceLocation)))
        {
			Uri resourceUri = nullptr;
			winrt::attach_abi(resourceUri, resourceLocator);

            auto uri = resourceUri.AbsoluteUri();
            if (uri.starts_with(L"ms-appx:///"))
            {
                auto maps = ResourceManager::Current().AllResourceMaps();
                for (auto const& map : maps)
                {
                    auto mapUriString = std::wstring(uri);
					mapUriString.replace(0, 11, L"ms-appx://" + map.Key() + L"/");
					auto mapUri = Uri(mapUriString);

                    if (SUCCEEDED(hr = s_LoadComponentWithResourceLocation(pThis, pComponent, winrt::get_abi(mapUri), componentResourceLocation)))
                        break;
			    }
            }
		}

        return hr;
    }

    bool DynamicLoader::EnsureInitialized()
    {
        if (!s_initialized)
        {
            try
            {
				auto app = winrt::get_activation_factory<Application, IApplicationStatics>();
                auto vtftbl = *(void***)winrt::get_abi(app);
                s_LoadComponentWithResourceLocation = (decltype(s_LoadComponentWithResourceLocation))vtftbl[9];

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourAttach(&(PVOID&)s_LoadComponentWithResourceLocation, DynamicLoader::LoadComponentWithResourceLocationHook);
                s_initialized = DetourTransactionCommit() == NO_ERROR;
			} catch (...) { }
        }

		return s_initialized;
    }
}
