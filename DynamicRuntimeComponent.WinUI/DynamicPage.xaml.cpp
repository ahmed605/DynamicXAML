#include "pch.h"
#include "DynamicPage.xaml.h"
#if __has_include("DynamicPage.g.cpp")
#include "DynamicPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::DynamicRuntimeComponent::WinUI::implementation
{
    void DynamicPage::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"WinUI's native world says hi!"));
    }
}
