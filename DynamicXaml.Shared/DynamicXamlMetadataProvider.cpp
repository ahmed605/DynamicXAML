#include "pch.h"
#include "DynamicXamlMetadataProvider.h"
#include "DynamicXaml_MetadataProvider.DynamicXamlMetadataProvider.g.cpp"

namespace winrt::DYNAMIC_XAML_NAMESPACE::DynamicXaml_MetadataProvider::implementation
{
    std::unordered_map<uint16_t, IXamlMetadataProvider> DynamicXamlMetadataProvider::s_providers { };
    uint16_t DynamicXamlMetadataProvider::s_nextToken { 0 };
    std::mutex DynamicXamlMetadataProvider::s_mutex { };

    IXamlType DynamicXamlMetadataProvider::GetXamlType(TypeName const& type)
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        for (auto const& provider : s_providers)
        {
            if (auto xamlType = provider.second.GetXamlType(type))
            {
                return xamlType;
            }
        }

		return nullptr;
    }

    IXamlType DynamicXamlMetadataProvider::GetXamlType(hstring const& fullName)
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        for (auto const& provider : s_providers)
        {
            if (auto xamlType = provider.second.GetXamlType(fullName))
            {
                return xamlType;
            }
        }

        return nullptr;
    }

    com_array<XmlnsDefinition> DynamicXamlMetadataProvider::GetXmlnsDefinitions()
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        std::vector<XmlnsDefinition> allDefinitions;

        for (auto const& provider : s_providers)
        {
            auto definitions = provider.second.GetXmlnsDefinitions();
            allDefinitions.insert(allDefinitions.end(), definitions.begin(), definitions.end());
        }

		return com_array<XmlnsDefinition>(std::move(allDefinitions));
    }
}
