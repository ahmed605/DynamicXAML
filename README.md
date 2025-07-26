# DynamicXAML

**DynamicXaml** is a library that allows you to dynamically load XAML resources (such as XAML/XBF files embedded inside PRI files) at runtime in **UWP** and **WinUI 3** applications.

With **DynamicXaml** you can dynamically load external XAML libraries at runtime, which is useful for scenarios like plugin systems, theming, or modular applications where you want to load UI components that aren't part of the application itself nor compiled with it.

## Installation

NuGet packages are coming soon, for now you can clone the repository and build the solution to get the library.

## Usage

### On libraries side

Libraries have to be either native Windows Runtime Components or .NET Class Libraries.

Libraries should add the following property to their project (e.g. csproj, vcxproj, ...) file:
```xml
<DisableEmbeddedXbf>false</DisableEmbeddedXbf>
```

This property will ensure that all XAML resources are embedded into the library's PRI file, which is required for **DynamicXaml** to work.

#### Additional considerations for UWP libraries

If the library is accessing its resources via `ms-appx:///` URIs, these should be changed to include the library's PRI index name after the second slash like the following example:
```xml
<!-- Before -->
<ResourceDictionary Source="ms-appx:///My.Dynamic.Lib/Themes/Generic.xaml" />
<!-- After -->
<ResourceDictionary Source="ms-appx://My.Dynamic.Lib/My.Dynamic.Lib/Themes/Generic.xaml" />
```

This limitation exists because `Windows.UI.Xaml` (aka **UWP XAML**) uses a private MRM API to load these resources and **DynamicXaml** aims to avoid using or hooking any private APIs, this limitation however does **NOT** exist in **WinUI 3** since it uses a public API (the WinRT MRT Core API) for loading these resources and **DynamicXaml** hooks that API and makes the required redirections for it to work.

### On application side

To use **DynamicXaml** in your application, you need to add a reference to the library and then after initializing XAML, call `DynamicLoader.LoadPri` with a PRI file StorageFile, on **WinUI 3** there's also an overload that accepts a path to the PRI file and a `Try` variant of the `LoadPri` function.

Example usage in C#:
```csharp
using DynamicXaml;

...

// Load the PRI file
StorageFile priFile = await ApplicationData.Current.LocalFolder.GetFileAsync("My.Dynamic.Lib.pri");
DynamicLoader.LoadPri(priFile);
```

Then you can load the library dll and use the XAML resources defined in it, this is done via either `NativeLibrary.Load` (C#) or `LoadLibrary` (Native) in case of native libraries, or `Assembly.Load` in case of .NET libraries, in case of native libraries you can activate the XAML classes defined in it by calling the `DllGetActivationFactory` function exported by the dll.

There are sample projects in the repository that demonstrate how to use **DynamicXaml** for both Native and .NET libraries in both [UWP](https://github.com/ahmed605/DynamicXAML/tree/master/DynamicXaml.UWP.Sample) and [WinUI 3](https://github.com/ahmed605/DynamicXAML/tree/master/DynamicXaml.WinUI.Sample) applications.