#include "pch.h"
#include "XamlMetadataProviderHelper.h"
#include "XamlMetadataProviderHelper.g.cpp"

namespace winrt::DYNAMIC_XAML_NAMESPACE::implementation
{
    inline uint32_t XamlMetadataProviderHelper::GetParameterCount(PCCOR_SIGNATURE signature)
    {
        CorSigUncompressData(signature); // calling convention
        return CorSigUncompressData(signature);
    }

    IVectorView<hstring> XamlMetadataProviderHelper::GetProviderTypeNames(IMetaDataImport* metaDataImport)
    {
        std::vector<hstring> typeNames;

        ULONG count = 0;
        HCORENUM hEnum = nullptr;
        mdTypeDef mdType = { };

        WCHAR* typeName = new WCHAR[MAX_CLASS_NAME + 1]();
        WCHAR* interfaceTypeName = new WCHAR[MAX_CLASS_NAME + 1]();

        while (SUCCEEDED(metaDataImport->EnumTypeDefs(&hEnum, &mdType, 1, &count)) && count == 1)
        {
            DWORD typeDefFlags = 0;
            ULONG typeNameLength = 0;
            mdToken baseTypeToken = { };

            if (SUCCEEDED(metaDataImport->GetTypeDefProps(mdType, typeName, MAX_CLASS_NAME, &typeNameLength, &typeDefFlags, &baseTypeToken)) &&
                IsTdClass(typeDefFlags) && TypeFromToken(baseTypeToken) == mdtTypeRef)
            {
                ULONG implCount = 0;
                HCORENUM hEnumImpl = NULL;
                mdInterfaceImpl impl = { };

				bool implementsIXamlMetadataProvider = false;
                while (SUCCEEDED(metaDataImport->EnumInterfaceImpls(&hEnumImpl, mdType, &impl, 1, &implCount)) && implCount == 1)
                {
                    mdToken interfaceToken = { };
                    if (SUCCEEDED(metaDataImport->GetInterfaceImplProps(impl, nullptr, &interfaceToken)) &&
                        TypeFromToken(interfaceToken) == mdtTypeRef)
                    {
                        ULONG interfaceNameLength = 0;
                        if (SUCCEEDED(metaDataImport->GetTypeRefProps(interfaceToken, nullptr, interfaceTypeName, MAX_CLASS_NAME, &interfaceNameLength)) &&
                            wcsncmp(interfaceTypeName, XAML_UI_WSTR L".Markup.IXamlMetadataProvider", interfaceNameLength) == 0)
                        {
                            implementsIXamlMetadataProvider = true;
                            break;
                        }
                    }
				}

				if (hEnumImpl) metaDataImport->CloseEnum(hEnumImpl);
                if (!implementsIXamlMetadataProvider) continue;

                ULONG ctorCount = 0;
                mdMethodDef mdCtor = { };
                HCORENUM hCtorEnum = nullptr;

                bool hasPublicDefaultCtor = false;
                while (SUCCEEDED(metaDataImport->EnumMethodsWithName(&hCtorEnum, mdType, L".ctor", &mdCtor, 1, &ctorCount)) && ctorCount == 1)
                {
                    ULONG cbSig = 0;
                    DWORD methodAttributes = 0;
                    PCCOR_SIGNATURE pvSig = { };

                    if (SUCCEEDED(metaDataImport->GetMethodProps(mdCtor, nullptr, nullptr, 0, nullptr, &methodAttributes, &pvSig, &cbSig, nullptr, nullptr)) &&
                        IsMdPublic(methodAttributes) && GetParameterCount(pvSig) == 0)
                    {
                        hasPublicDefaultCtor = true;
                        break;
                    }
                }

                if (hCtorEnum) metaDataImport->CloseEnum(hCtorEnum);
                if (!hasPublicDefaultCtor) continue;
                
                typeNames.push_back(hstring(typeName, typeNameLength - 1));
            }
        }

        delete[] typeName;
        delete[] interfaceTypeName;
        if (hEnum) metaDataImport->CloseEnum(hEnum);

        return single_threaded_vector(std::move(typeNames)).GetView();
    }

    IAsyncOperation<IVectorView<hstring>> XamlMetadataProviderHelper::GetProviderTypeNamesFromAssemblyAsync(hstring assemblyPath)
    {
        co_await winrt::resume_background();

		com_ptr<IMetaDataDispenser> metaDataDispenser;
		check_hresult(MetaDataGetDispenser(CLSID_CorMetaDataDispenser, IID_IMetaDataDispenser, metaDataDispenser.put_void()));

		com_ptr<IMetaDataImport> metaDataImport;
		check_hresult(metaDataDispenser->OpenScope(assemblyPath.c_str(), ofRead, IID_IMetaDataImport, reinterpret_cast<::IUnknown**>(metaDataImport.put())));

		co_return GetProviderTypeNames(metaDataImport.get());
    }

    IAsyncOperation<IVectorView<hstring>> XamlMetadataProviderHelper::GetProviderTypeNamesFromAssemblyAsync(array_view<uint8_t const> assemblyBytes)
    {
        co_await winrt::resume_background();

        com_ptr<IMetaDataDispenser> metaDataDispenser;
        check_hresult(MetaDataGetDispenser(CLSID_CorMetaDataDispenser, IID_IMetaDataDispenser, metaDataDispenser.put_void()));

        com_ptr<IMetaDataImport> metaDataImport;
        check_hresult(metaDataDispenser->OpenScopeOnMemory(
            const_cast<uint8_t*>(assemblyBytes.data()),
            static_cast<ULONG>(assemblyBytes.size()),
            ofRead,
            IID_IMetaDataImport,
            reinterpret_cast<::IUnknown**>(metaDataImport.put())));

		co_return GetProviderTypeNames(metaDataImport.get());
    }

    IAsyncOperation<IVectorView<hstring>> XamlMetadataProviderHelper::GetProviderTypeNamesFromAssemblyAsync(StorageFile const& assemblyFile)
    {
        const auto& buffer = co_await winrt::Windows::Storage::FileIO::ReadBufferAsync(assemblyFile);
        co_return co_await GetProviderTypeNamesFromAssemblyAsync({ buffer.data(), buffer.Length() });
    }
}
