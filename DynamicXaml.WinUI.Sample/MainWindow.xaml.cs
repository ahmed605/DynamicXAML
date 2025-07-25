using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.Storage.Pickers;
using Windows.UI.Popups;
using WinRT;
using WinRT.Interop;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace DynamicXaml.WinUI.Sample
{
    /// <summary>
    /// An empty window that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private async void Grid_Loaded(object sender, RoutedEventArgs e)
        {
            FileOpenPicker picker = new()
            {
                FileTypeFilter = { ".dll", ".pri" },
                CommitButtonText = "Load"
            };

            InitializeWithWindow.Initialize(picker, WindowNative.GetWindowHandle(this));

        Pick:
            var result = await picker.PickMultipleFilesAsync();
            if (result is { Count: 2 } files)
            {
                var pri = files.FirstOrDefault(f => f.FileType == ".pri");
                var dll = files.FirstOrDefault(f => f.FileType == ".dll");

                if (pri is not null && dll is not null)
                {
                    var name = $"{dll.Name[0..^4]}.DynamicPage";

                    var box = new TextBox
                    {
                        Text = name,
                        PlaceholderText = "UIElement fully class qualified name (e.g. MyLibrary.Controls.MyPage)",
                        Margin = new(15)
                    };

                    var dialog = new ContentDialog
                    {
                        Title = "Enter UIElement fully class qualified name to load",
                        Content = box,
                        PrimaryButtonText = "Load",
                        IsPrimaryButtonEnabled = true,
                        IsSecondaryButtonEnabled = false,
                        DefaultButton = ContentDialogButton.Primary,
                        XamlRoot = Content.XamlRoot
                    };

                    dialog.PrimaryButtonClick += (s, e) =>
                    {
                        if (string.IsNullOrWhiteSpace(box.Text))
                        {
                            e.Cancel = true;
                        }
                    };

                    if (await dialog.ShowAsync() == ContentDialogResult.Primary)
                    {
                        name = box.Text;
                        var appdata = ApplicationData.Current.LocalFolder;
                        pri = await pri.CopyAsync(appdata, pri.Name, NameCollisionOption.ReplaceExisting);
                        dll = await dll.CopyAsync(appdata, dll.Name, NameCollisionOption.ReplaceExisting);

                        DynamicLoader.LoadPri(pri);

                        UIElement element = null;
                        if (NativeLibrary.TryLoad(dll.Path, out nint handle) && NativeLibrary.TryGetExport(handle, "DllGetActivationFactory", out nint pDllGetActivationFactory))
                        {
                            unsafe
                            {
                                // Error checking omitted
                                var DllGetActivationFactory = (delegate* unmanaged[Stdcall]<void*, void**, int>)pDllGetActivationFactory;
                                void* factoryPtr = default;
                                DllGetActivationFactory((void*)MarshalString.FromManaged(name), &factoryPtr);
                                void* obj = default;
                                ((delegate* unmanaged[Stdcall]<void*, void**, int>)(*(void**)((nint)(*(void**)factoryPtr) + 6 * sizeof(void*))))(factoryPtr, &obj);
                                element = UIElement.FromAbi((nint)obj);
                            }
                        }
                        else
                        {
                            var asm = Assembly.LoadFile(dll.Path);
                            var type = asm.GetType(name, true, false);
                            element = (UIElement)Activator.CreateInstance(type);
                        }

                        if (element is not null)
                        {
                            Content = element;
                            return;
                        }
                    }
                }
            }

            if (result is not null && result.Count is not 0)
            {
                MessageDialog dialog = new("Please select a .dll and a .pri file to load.", "Invalid Files Selected");
                InitializeWithWindow.Initialize(dialog, WindowNative.GetWindowHandle(this));
                await dialog.ShowAsync();
                goto Pick;
            }

            Application.Current.Exit();
        }
    }
}
