#pragma once
#include "XamlMetadataProviderHelper.g.h"
#include "Common.h"

#include <corhdr.h>
#include <rometadata.h>
#include <rometadataapi.h>

#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>

namespace winrt::DYNAMIC_XAML_NAMESPACE::implementation
{
    using namespace ::winrt::Windows::Storage;
    using namespace ::winrt::Windows::Foundation;
    using namespace ::winrt::Windows::Foundation::Collections;

    struct XamlMetadataProviderHelper
    {
    private:
		static IVectorView<hstring> GetProviderTypeNames(IMetaDataImport* metaDataImport);
        inline static uint32_t GetParameterCount(PCCOR_SIGNATURE signature);

    public:
        XamlMetadataProviderHelper() = default;

        static IAsyncOperation<IVectorView<hstring>> GetProviderTypeNamesFromAssemblyAsync(hstring assemblyPath);
        static IAsyncOperation<IVectorView<hstring>> GetProviderTypeNamesFromAssemblyAsync(array_view<uint8_t const> assemblyBytes);
        static IAsyncOperation<IVectorView<hstring>> GetProviderTypeNamesFromAssemblyAsync(StorageFile const& assemblyFile);
    };
}

namespace winrt::DYNAMIC_XAML_NAMESPACE::factory_implementation
{
    struct XamlMetadataProviderHelper : XamlMetadataProviderHelperT<XamlMetadataProviderHelper, implementation::XamlMetadataProviderHelper>
    {
    };
}
