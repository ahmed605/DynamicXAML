#include "pch.h"
#include "DynamicPage.h"
#if __has_include("DynamicPage.g.cpp")
#include "DynamicPage.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::DynamicRuntimeComponent::UWP::implementation
{
    void DynamicPage::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Native world says hi!"));
    }
}
