#pragma once

#include <Windows.h>

namespace DynamicXaml
{
    inline bool IsWinUIAddress(PVOID Address)
    {
		static HMODULE winui = GetModuleHandleW(L"Microsoft.UI.Xaml.dll");

        HMODULE Module;
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)Address, &Module))
            return false;

        return Module == winui;
    }

    inline bool IsWindowsXamlAddress(PVOID Address)
    {
        static HMODULE wux = GetModuleHandleW(L"Windows.UI.Xaml.dll");

        HMODULE Module;
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)Address, &Module))
            return false;

        return Module == wux;
    }

    #define IsWinUICallee() ::DynamicXaml::IsWinUIAddress(_ReturnAddress())
    #define IsWindowsXamlCallee() ::DynamicXaml::IsWindowsXamlAddress(_ReturnAddress())
}