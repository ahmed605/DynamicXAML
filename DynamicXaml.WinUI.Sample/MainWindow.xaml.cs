#pragma warning disable CS9123 // The '&' operator should not be used on parameters or local variables in async methods.

using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Markup;
using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
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
                FileTypeFilter = { ".dll", ".pri", ".winmd" },
                CommitButtonText = "Load"
            };

            InitializeWithWindow.Initialize(picker, WindowNative.GetWindowHandle(this));

        Pick:
            var result = await picker.PickMultipleFilesAsync();
            if (result is { Count: >= 2 } files)
            {
                var pri = files.FirstOrDefault(f => f.FileType == ".pri");
                var dll = files.FirstOrDefault(f => f.FileType == ".dll");
                var winmd = files.FirstOrDefault(f => f.FileType == ".winmd");

                if (pri is not null && dll is not null)
                {
                    var appdata = ApplicationData.Current.LocalFolder;

                    nint pDllGetActivationFactory = 0;
                    dll = await dll.CopyAsync(appdata, dll.Name, NameCollisionOption.ReplaceExisting);
                    bool isNative = NativeLibrary.TryLoad(dll.Path, out nint handle) && NativeLibrary.TryGetExport(handle, "DllGetActivationFactory", out pDllGetActivationFactory);

                    var name = $"{dll.Name[0..^4]}.DynamicPage";

                    var box = new TextBox
                    {
                        Text = name,
                        PlaceholderText = "UIElement fully qualified class name (e.g. MyLibrary.Controls.MyPage)",
                        Margin = new(15, 15, 15, 10),
                        HorizontalAlignment = HorizontalAlignment.Stretch
                    };

                    var checkBox = new CheckBox
                    {
                        Content = "Use XamlReader (needs WinMD in case of native dlls)",
                        Margin = new(15, 0, 15, 15),
                        IsChecked = false,
                        IsEnabled = !isNative || winmd is not null,
                        HorizontalAlignment = HorizontalAlignment.Left
                    };

                    var stack = new StackPanel()
                    {
                        Orientation = Orientation.Vertical,
                        Children =
                        {
                            box,
                            checkBox
                        }
                    };

                    var dialog = new ContentDialog
                    {
                        Title = "Enter UIElement fully qualified class name to load",
                        Content = stack,
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
                        pri = await pri.CopyAsync(appdata, pri.Name, NameCollisionOption.ReplaceExisting);
                        winmd = winmd is not null ? await winmd.CopyAsync(appdata, winmd.Name, NameCollisionOption.ReplaceExisting) : null;

                        DynamicLoader.LoadPri(pri);

                        UIElement element = null;
                        bool useXamlReader = checkBox.IsChecked is true;

                        IReadOnlyList<string> providerTypeNames = null;
                        List<IXamlMetadataProvider> providers = null;

                        if (useXamlReader)
                        {
                            providerTypeNames = isNative ?
                                (winmd is not null ? await XamlMetadataProviderHelper.GetProviderTypeNamesFromAssemblyAsync(winmd) : null)
                                : await XamlMetadataProviderHelper.GetProviderTypeNamesFromAssemblyAsync(dll);

                            useXamlReader = providerTypeNames is not null && providerTypeNames.Count > 0;
                        }

                        if (isNative)
                        {
                            unsafe
                            {
                                var DllGetActivationFactory = (delegate* unmanaged[Stdcall]<void*, void**, int>)pDllGetActivationFactory;

                                if (useXamlReader)
                                {
                                    providers = new();
                                    foreach (var providerTypeName in providerTypeNames)
                                    {
                                        void* factoryPtr = default;
                                        if (DllGetActivationFactory((void*)MarshalString.FromManaged(providerTypeName), &factoryPtr) >= 0)
                                        {
                                            void* obj = default;
                                            var vtftbl = *(void***)factoryPtr;
                                            var ActivateInstance = (delegate* unmanaged[Stdcall]<void*, void**, int>)vtftbl[6];

                                            if (ActivateInstance(factoryPtr, &obj) >= 0)
                                            {
                                                Marshal.ThrowExceptionForHR(Marshal.QueryInterface((nint)obj, typeof(IXamlMetadataProvider).GUID, out nint pProvider));
                                                providers.Add(MarshalInspectable<IXamlMetadataProvider>.FromAbi(pProvider));
                                                Marshal.Release((nint)factoryPtr);
                                                Marshal.Release(pProvider);
                                                Marshal.Release((nint)obj);
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    void* factoryPtr = default;
                                    if (DllGetActivationFactory((void*)MarshalString.FromManaged(name), &factoryPtr) >= 0)
                                    {
                                        void* obj = default;
                                        var vtftbl = *(void***)factoryPtr;
                                        var ActivateInstance = (delegate* unmanaged[Stdcall]<void*, void**, int>)vtftbl[6];

                                        if (ActivateInstance(factoryPtr, &obj) >= 0)
                                        {
                                            element = UIElement.FromAbi((nint)obj);
                                            Marshal.Release((nint)factoryPtr);
                                            Marshal.Release((nint)obj);
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            var asm = Assembly.LoadFile(dll.Path);

                            if (useXamlReader)
                            {
                                providers = new();
                                foreach (var providerTypeName in providerTypeNames)
                                {
                                    var providerType = asm.GetType(providerTypeName, true, false);
                                    var provider = (IXamlMetadataProvider)Activator.CreateInstance(providerType);
                                    providers.Add(provider);
                                }
                            }
                            else
                            {
                                var type = asm.GetType(name, true, false);
                                element = (UIElement)Activator.CreateInstance(type);
                            }
                        }

                        if (useXamlReader && providers is not null)
                        {
                            foreach (var provider in providers)
                            {
                                _ = DynamicLoader.RegisterXamlMetadataProvider(provider);
                            }

                            var shortName = name.Split('.').Last();
                            var namespaceName = name[0..^(shortName.Length + 1)];

                            var xaml =
                            $@"
                                <Page xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation'
                                      xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml'
                                      xmlns:lib='using:{namespaceName}'>
                                    <lib:{shortName}/>
                                </Page>
                            ";

                            element = (UIElement)XamlReader.Load(xaml);
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
