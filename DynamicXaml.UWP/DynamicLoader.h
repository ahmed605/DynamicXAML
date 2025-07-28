#pragma once

#include "DynamicLoader.g.h"

#include <mrm_private.h>

namespace awarc = ABI::Windows::ApplicationModel::Resources::Core;

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::ApplicationModel::Resources::Core;

namespace winrt::DynamicXaml::UWP::implementation
{
    struct DynamicLoader
    {
    private:
		static bool s_initialized;
		static bool s_enableUnsafeHooks;
		static std::unordered_map<awarc::IMrtResourceMap*, awarc::IMrtResourceMap*> s_resourceMaps;

		static bool EnsureInitialized();

    public:
        static HRESULT WINAPI LoadComponentWithResourceLocationHook(void* pThis, IInspectable* pComponent, void* resourceLocator, ComponentResourceLocation componentResourceLocation);
        static HRESULT WINAPI GetNamedResourceHook(void* pThis, LPCWSTR unk1, const GUID& iid, void** ppv);

    public:
        DynamicLoader() = default;

        static bool EnableUnsafeHooks();
        static void EnableUnsafeHooks(bool value);
        static void LoadPri(winrt::Windows::Storage::StorageFile const& priFile);
    };
}

namespace winrt::DynamicXaml::UWP::factory_implementation
{
    struct DynamicLoader : DynamicLoaderT<DynamicLoader, implementation::DynamicLoader>
    {
    };
}
