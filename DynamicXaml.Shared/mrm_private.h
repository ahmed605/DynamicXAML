// Sources:
// - https://github.com/GPUOpen-Tools/common_src_amdtoswrappers/blob/master/src/win32/osApplication.cpp

#pragma once

#include <Windows.h>

namespace ABI::Windows::ApplicationModel::Resources::Core
{
    class DECLSPEC_UUID("DBCE7E40-7345-439D-B12C-114A11819A09") MrtResourceManager;

    MIDL_INTERFACE("130A2F65-2BE7-4309-9A58-A9052FF2B61C")
    IMrtResourceManager : public ::IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize() = 0;
        virtual HRESULT STDMETHODCALLTYPE InitializeForCurrentApplication() = 0;
        virtual HRESULT STDMETHODCALLTYPE InitializeForPackage(LPCWSTR) = 0;
        virtual HRESULT STDMETHODCALLTYPE InitializeForFile(LPCWSTR) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetMainResourceMap(const GUID&, void**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetResourceMap(LPCWSTR, const GUID&, void**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetDefaultContext(const GUID&, void**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetReference(const GUID&, void**) = 0;
        virtual HRESULT STDMETHODCALLTYPE IsResourceReference(LPCWSTR, BOOL*) = 0;
    };

    MIDL_INTERFACE("6E21E72B-B9B0-42AE-A686-983CF784EDCD")
    IMrtResourceMap : public ::IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetUri(LPWSTR*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetSubtree(LPCWSTR, IMrtResourceMap**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetString(LPCWSTR, LPWSTR*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetStringForContext(struct IMrtResourceContext*, LPCWSTR, LPWSTR*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetFilePath(LPCWSTR, LPWSTR*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetFilePathForContext(struct IMrtResourceContext*, LPCWSTR, LPWSTR*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetNamedResourceCount(unsigned int*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetNamedResourceUri(unsigned int, LPWSTR*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetNamedResource(LPCWSTR, const GUID&, void**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetFullyQualifiedReference(LPCWSTR, LPCWSTR, LPWSTR*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetFilePathByUri(IUri*, LPWSTR*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetFilePathForContextByUri(struct IMrtResourceContext*, IUri*, LPWSTR*) = 0;
    };
}