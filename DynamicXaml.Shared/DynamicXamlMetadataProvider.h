#pragma once

#include <mutex>
#include "DynamicXaml_MetadataProvider.DynamicXamlMetadataProvider.g.h"
#include "Common.h"

#include XAML_UI_MARKUP_H
#include <winrt/Windows.UI.Xaml.Interop.h>

namespace winrt::DYNAMIC_XAML_NAMESPACE::DynamicXaml_MetadataProvider::implementation
{
	using namespace ::winrt::XAML_NAMESPACE::Markup;
	using namespace ::winrt::Windows::UI::Xaml::Interop;

    struct DynamicXamlMetadataProvider : DynamicXamlMetadataProviderT<DynamicXamlMetadataProvider>
    {
    public:
		static std::unordered_map<uint16_t, IXamlMetadataProvider> s_providers;
		static uint16_t s_nextToken;
		static std::mutex s_mutex;

    public:
        DynamicXamlMetadataProvider() = default;

        IXamlType GetXamlType(TypeName const& type);
        IXamlType GetXamlType(hstring const& fullName);
        com_array<XmlnsDefinition> GetXmlnsDefinitions();
    };
}

namespace winrt::DYNAMIC_XAML_NAMESPACE::DynamicXaml_MetadataProvider::factory_implementation
{
    struct DynamicXamlMetadataProvider : DynamicXamlMetadataProviderT<DynamicXamlMetadataProvider, implementation::DynamicXamlMetadataProvider>
    {
    };
}
