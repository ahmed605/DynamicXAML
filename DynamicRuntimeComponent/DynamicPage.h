#pragma once

#include "DynamicPage.g.h"

namespace winrt::DynamicRuntimeComponent::implementation
{
    struct DynamicPage : DynamicPageT<DynamicPage>
    {
        DynamicPage() { }

        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::DynamicRuntimeComponent::factory_implementation
{
    struct DynamicPage : DynamicPageT<DynamicPage, implementation::DynamicPage>
    {
    };
}
