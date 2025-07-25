using ABI.WinRT.Interop;
using System;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Windows.ApplicationModel.Resources.Core;
using Windows.Storage;
using Windows.Storage.Pickers;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using WinRT;

namespace DynamicXaml.UWP.Sample
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a <see cref="Frame">.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            InitializeComponent();
        }

        private async void Page_Loaded(object sender, RoutedEventArgs e)
        {
            FileOpenPicker picker = new()
            {
                FileTypeFilter = { ".dll", ".pri" },
                CommitButtonText = "Load"
            };

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

                        var manager = ResourceManager.Current;
                        manager.LoadPriFiles([pri]);

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
                            Frame.Content = element;
                            return;
                        }
                    }
                }
            }

            if (result is not null && result.Count is not 0)
            {
                MessageDialog dialog = new("Please select a .dll and a .pri file to load.", "Invalid Files Selected");
                await dialog.ShowAsync();
                goto Pick;
            }

            Application.Current.Exit();
        }
    }
}
