#pragma once

#ifdef DYNAMIC_XAML_UWP

#define XAML_NAMESPACE Windows::UI::Xaml
#define XAML_NAMESPACE_MIDL Windows.UI.Xaml

#define DYNAMIC_XAML_NAMESPACE DynamicXaml::UWP
#define DYNAMIC_XAML_NAMESPACE_MIDL DynamicXaml.UWP
#define XAML_UI_MARKUP_H <winrt/Windows.UI.Xaml.Markup.h>
#define XAML_UI_WSTR L"Windows.UI.Xaml"

#else

#define XAML_NAMESPACE Microsoft::UI::Xaml
#define XAML_NAMESPACE_MIDL Microsoft.UI.Xaml

#define DYNAMIC_XAML_NAMESPACE DynamicXaml::WinUI
#define DYNAMIC_XAML_NAMESPACE_MIDL DynamicXaml.WinUI
#define XAML_UI_MARKUP_H <winrt/Microsoft.UI.Xaml.Markup.h>
#define XAML_UI_WSTR L"Microsoft.UI.Xaml"

#endif