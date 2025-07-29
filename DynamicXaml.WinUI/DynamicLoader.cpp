#include "pch.h"
#include "DynamicLoader.h"
#if __has_include("DynamicLoader.g.cpp")
#include "DynamicLoader.g.cpp"
#endif

#include <Windows.h>
#include <detours/detours.h>

#include <DynamicHelpers.h>

namespace warc = winrt::Windows::ApplicationModel::Resources::Core;

namespace winrt::DynamicXaml::WinUI::implementation
{
	bool DynamicLoader::s_initialized = false;
	std::vector<ResourceMap> DynamicLoader::s_resourceMaps;
    decltype(&DynamicLoader::TryGetValueWithContextHook) s_TryGetValueWithContext = nullptr;
    decltype(&DynamicLoader::TryGetSubtreeHook) s_TryGetSubtree = nullptr;

    bool implementation::DynamicLoader::TryLoadPri(hstring const& priFilePath)
    {
        if (EnsureInitialized())
        {
            try
            {
                ResourceManager resourceManager{ priFilePath };
                s_resourceMaps.push_back(resourceManager.MainResourceMap());

                if (auto task = winrt::Windows::Storage::StorageFile::GetFileFromPathAsync(priFilePath))
                {
                    task.Completed([](auto const& asyncInfo, auto const&)
                    {
                        try
                        {
                            auto file = asyncInfo.GetResults();
                            warc::ResourceManager::Current().LoadPriFiles({ file });
                        } catch (...) { }
                    });
                }

                return true;
            } catch (...) { }
        }

        return false;
    }

    bool implementation::DynamicLoader::TryLoadPri(winrt::Windows::Storage::StorageFile const& priFile)
    {
        if (!priFile)
            return false;

        return TryLoadPri(priFile.Path());;
    }

    void DynamicLoader::LoadPri(hstring const& priFilePath)
    {
		if (!TryLoadPri(priFilePath))
            throw winrt::hresult_error(E_FAIL, L"Failed to load the specified PRI file.");
    }

    void DynamicLoader::LoadPri(winrt::Windows::Storage::StorageFile const& priFile)
    {
        if (!priFile)
            throw winrt::hresult_invalid_argument(L"priFile cannot be null.");

		LoadPri(priFile.Path());
    }

    HRESULT WINAPI DynamicLoader::TryGetValueWithContextHook(void* pThis, HSTRING resource, void* pContext, void** ppCandidate)
    {
        HRESULT hr = S_OK;

		void* pCandidate = nullptr;
        if (SUCCEEDED(hr = s_TryGetValueWithContext(pThis, resource, pContext, &pCandidate)) && pCandidate == nullptr)
        {
            for (const auto& map : s_resourceMaps)
            {
                if (SUCCEEDED(hr = s_TryGetValueWithContext(winrt::get_abi(map), resource, pContext, &pCandidate)) && pCandidate)
                    break;
            }
        }

        if (ppCandidate)
        {
            *ppCandidate = pCandidate;
        }

		return hr;
    }

    HRESULT WINAPI DynamicLoader::TryGetSubtreeHook(void* pThis, HSTRING reference, void** ppResMap)
    {
		HRESULT hr = S_OK;

        winrt::hstring refStr;
        winrt::copy_from_abi(refStr, reference);

        if (SUCCEEDED(hr = s_TryGetSubtree(pThis, reference, ppResMap)) && ppResMap && !*ppResMap && IsWinUICallee() && refStr != L"Files")
        {
            for (const auto& map : s_resourceMaps)
            {
                if (SUCCEEDED(hr = s_TryGetSubtree(winrt::get_abi(map), reference, ppResMap)) && *ppResMap)
                    break;
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
                ResourceManager manager;
                auto map = manager.MainResourceMap();
                auto vtftbl = *(void***)winrt::get_abi(map);
                s_TryGetValueWithContext = (decltype(s_TryGetValueWithContext))vtftbl[14];
                s_TryGetSubtree = (decltype(s_TryGetSubtree))vtftbl[8];

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourAttach(&(PVOID&)s_TryGetValueWithContext, DynamicLoader::TryGetValueWithContextHook);
                DetourAttach(&(PVOID&)s_TryGetSubtree, DynamicLoader::TryGetSubtreeHook);
                s_initialized = DetourTransactionCommit() == NO_ERROR;
			} catch (...) { }
        }

		return s_initialized;
    }
}
