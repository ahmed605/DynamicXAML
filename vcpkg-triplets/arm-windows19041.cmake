set(VCPKG_TARGET_ARCHITECTURE arm)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_VERSION "10.0.19041.0")
set(CMAKE_SYSTEM_VERSION "10.0.19041.0")
set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0.19041.0")
list(APPEND VCPKG_KEEP_ENV_VARS "WindowsSDKVersion" "WindowsSDKLibVersion" "WindowsSdkVerBinPath" "WindowsSdkVerBinPath" "WindowsLibPath")
set(VCPKG_ENV_PASSTHROUGH "LIB;PATH;CPATH;VARSDIR;WindowsSDKVersion;WindowsSDKLibVersion;WindowsSdkVerBinPath;WindowsSdkVerBinPath;WindowsLibPath")
set(VCPKG_LOAD_VCVARS_ENV OFF CACHE BOOL "")