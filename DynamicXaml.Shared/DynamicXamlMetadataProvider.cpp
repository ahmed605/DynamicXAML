#include "pch.h"
#include "DynamicXamlMetadataProvider.h"
#include "DynamicXaml_MetadataProvider.DynamicXamlMetadataProvider.g.cpp"
#include <wil/resource.h>

namespace winrt::DYNAMIC_XAML_NAMESPACE::DynamicXaml_MetadataProvider::implementation
{
    std::unordered_map<uint16_t, IXamlMetadataProvider> DynamicXamlMetadataProvider::s_providers { };
    uint16_t DynamicXamlMetadataProvider::s_nextToken { 0 };
    std::mutex DynamicXamlMetadataProvider::s_mutex { };
    thread_local bool DynamicXamlMetadataProvider::s_isInsideCall { false };

    IXamlType DynamicXamlMetadataProvider::GetXamlType(TypeName const& type)
    {
        if (s_isInsideCall)
            return nullptr;

        std::lock_guard<std::mutex> lock(s_mutex);

        s_isInsideCall = true;
        auto r = wil::scope_exit([] { s_isInsideCall = false; });

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
        if (s_isInsideCall)
            return nullptr;

        std::lock_guard<std::mutex> lock(s_mutex);

        s_isInsideCall = true;
        auto r = wil::scope_exit([] { s_isInsideCall = false; });

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
        if (s_isInsideCall)
            return { };

        std::lock_guard<std::mutex> lock(s_mutex);

        s_isInsideCall = true;
        auto r = wil::scope_exit([] { s_isInsideCall = false; });
        std::vector<XmlnsDefinition> allDefinitions;

        for (auto const& provider : s_providers)
        {
            auto definitions = provider.second.GetXmlnsDefinitions();
            allDefinitions.insert(allDefinitions.end(), definitions.begin(), definitions.end());
        }

        return com_array<XmlnsDefinition>(std::move(allDefinitions));
    }
}
