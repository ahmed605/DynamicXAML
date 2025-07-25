#pragma once

#include "DynamicPage.g.h"

namespace winrt::DynamicRuntimeComponent::WinUI::implementation
{
    struct DynamicPage : DynamicPageT<DynamicPage>
    {
        DynamicPage() { }

        void ClickHandler(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::DynamicRuntimeComponent::WinUI::factory_implementation
{
    struct DynamicPage : DynamicPageT<DynamicPage, implementation::DynamicPage>
    {
    };
}
