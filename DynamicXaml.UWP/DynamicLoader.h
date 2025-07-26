#pragma once

#include "DynamicLoader.g.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::ApplicationModel::Resources::Core;

namespace winrt::DynamicXaml::UWP::implementation
{
    struct DynamicLoader : DynamicLoaderT<DynamicLoader>
    {
    private:
		static bool s_initialized;

		static bool EnsureInitialized();

    public:
        static HRESULT WINAPI LoadComponentWithResourceLocationHook(void* pThis, IInspectable* pComponent, void* resourceLocator, ComponentResourceLocation componentResourceLocation);

    public:
        DynamicLoader() = default;

        static void LoadPri(winrt::Windows::Storage::StorageFile const& priFile);
    };
}

namespace winrt::DynamicXaml::UWP::factory_implementation
{
    struct DynamicLoader : DynamicLoaderT<DynamicLoader, implementation::DynamicLoader>
    {
    };
}
