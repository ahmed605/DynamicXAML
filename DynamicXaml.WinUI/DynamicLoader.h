#pragma once

#include "DynamicLoader.g.h"
#include <Common.h>

using namespace winrt;
using namespace Microsoft::Windows::ApplicationModel::Resources;

namespace winrt::DynamicXaml::WinUI::implementation
{
    struct DynamicLoader
    {
    private:
		static bool s_initialized;
		static std::vector<ResourceMap> s_resourceMaps;

		static bool EnsureInitialized();

    public:
        static HRESULT WINAPI TryGetValueWithContextHook(void* pThis, HSTRING resource, void* pContext, void** ppCandidate);
        static HRESULT WINAPI TryGetSubtreeHook(void* pThis, HSTRING reference, void** ppResMap);

    public:
        DynamicLoader() = default;

        static void LoadPri(hstring const& priFilePath);
        static void LoadPri(winrt::Windows::Storage::StorageFile const& priFile);
        static bool TryLoadPri(hstring const& priFilePath);
        static bool TryLoadPri(winrt::Windows::Storage::StorageFile const& priFile);

        #include <IDynamicLoaderStatics2.impl.h.inl>
    };
}

namespace winrt::DynamicXaml::WinUI::factory_implementation
{
    struct DynamicLoader : DynamicLoaderT<DynamicLoader, implementation::DynamicLoader>
    {
    };
}
