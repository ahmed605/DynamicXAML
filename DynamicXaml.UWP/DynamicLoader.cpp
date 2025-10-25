#include "pch.h"
#include "DynamicLoader.h"
#if __has_include("DynamicLoader.g.cpp")
#include "DynamicLoader.g.cpp"
#endif

#include <detours/detours.h>
#include <DynamicHelpers.h>

#include <DynamicXamlMetadataProvider.h>

namespace winrt::DynamicXaml::UWP::implementation
{
    bool DynamicLoader::s_initialized = false;
    bool DynamicLoader::s_enableUnsafeHooks = false;
    std::unordered_map<awarc::IMrtResourceMap*, awarc::IMrtResourceMap*> DynamicLoader::s_resourceMaps;

    decltype(&DynamicLoader::LoadComponentWithResourceLocationHook) s_LoadComponentWithResourceLocation = nullptr;
    decltype(&DynamicLoader::GetNamedResourceHook) s_GetNamedResource = nullptr;
    decltype(&DynamicLoader::GetSubtreeHook) s_GetSubtree = nullptr;

    bool DynamicLoader::EnableUnsafeHooks()
    {
        return s_enableUnsafeHooks;
    }

    void DynamicLoader::EnableUnsafeHooks(bool value)
    {
        if (s_initialized)
            throw winrt::hresult_illegal_method_call(L"EnableUnsafeHooks can only be set before loading any PRIs.");

        s_enableUnsafeHooks = value;
    }

    void DynamicLoader::LoadPri(winrt::Windows::Storage::StorageFile const& priFile)
    {
        if (!priFile)
            throw winrt::hresult_invalid_argument(L"priFile cannot be null.");

        if (!EnsureInitialized())
            throw winrt::hresult_error(E_FAIL, L"Failed to initialize DynamicLoader.");

        ResourceManager::Current().LoadPriFiles({ priFile });

        if (s_enableUnsafeHooks)
        {
            com_ptr<awarc::IMrtResourceManager> resourceManager;
            winrt::check_hresult(CoCreateInstance(__uuidof(awarc::MrtResourceManager), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(resourceManager.put())));
            winrt::check_hresult(resourceManager->InitializeForFile(priFile.Path().c_str()));

            awarc::IMrtResourceMap* resourceMap;
            awarc::IMrtResourceMap* filesResourceMap;
            winrt::check_hresult(resourceManager->GetMainResourceMap(IID_PPV_ARGS(&resourceMap)));
            winrt::check_hresult(s_GetSubtree(resourceMap, L"Files", &filesResourceMap));
            s_resourceMaps.emplace(resourceMap, filesResourceMap);
        }
    }

    #include <IDynamicLoaderStatics2.Impl.cpp.inl>

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

    HRESULT WINAPI DynamicLoader::GetNamedResourceHook(void* pThis, LPCWSTR unk1, const GUID& iid, void** ppv)
    {
        HRESULT hr = S_OK;
        if (FAILED(hr = s_GetNamedResource(pThis, unk1, iid, ppv)))
        {
            for (auto& pair : s_resourceMaps)
            {
                if (SUCCEEDED(hr = s_GetNamedResource(pair.second, unk1, iid, ppv)))
					break;
			}
        }

		return hr;
    }

    HRESULT WINAPI DynamicLoader::GetSubtreeHook(void* pThis, LPCWSTR name, awarc::IMrtResourceMap** ppSubtree)
    {
        HRESULT hr = S_OK;

        if (FAILED(hr = s_GetSubtree(pThis, name, ppSubtree)) && IsWindowsXamlCallee() && std::wstring_view(name) != L"Files")
        {
            for (const auto& pair : s_resourceMaps)
            {
                if (SUCCEEDED(hr = s_GetSubtree(pair.first, name, ppSubtree)))
                    break;
            }
        }

        return hr;
    }

    bool DynamicLoader::EnsureInitialized()
    {
        if (!s_initialized)
        {
            if (s_enableUnsafeHooks)
            {
				com_ptr<awarc::IMrtResourceManager> resourceManager;
                if (SUCCEEDED(CoCreateInstance(__uuidof(awarc::MrtResourceManager), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(resourceManager.put()))))
                {
                    if (SUCCEEDED(resourceManager->InitializeForCurrentApplication()))
                    {
						com_ptr<awarc::IMrtResourceMap> resourceMap;
                        if (SUCCEEDED(resourceManager->GetMainResourceMap(IID_PPV_ARGS(resourceMap.put()))))
                        {
							auto vftbl = *(void***)resourceMap.get();
                            s_GetNamedResource = (decltype(s_GetNamedResource))vftbl[11];
                            s_GetSubtree = (decltype(s_GetSubtree))vftbl[4];

                            DetourTransactionBegin();
                            DetourUpdateThread(GetCurrentThread());
                            DetourAttach(&(PVOID&)s_GetNamedResource, DynamicLoader::GetNamedResourceHook);
                            DetourAttach(&(PVOID&)s_GetSubtree, DynamicLoader::GetSubtreeHook);
                            s_initialized = DetourTransactionCommit() == NO_ERROR;
						}
                    }
                }
            }
            else
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
        }

		return s_initialized;
    }
}
