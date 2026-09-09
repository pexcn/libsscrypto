# libsscrypto

Build libsscrypto.dll for shadowsocks-windows.

## Build

1) Get source code

```
git clone https://github.com/shadowsocks/libsscrypto.git
cd libsscrypto
git submodule update --init
```

2) Compile

  a) Open libsscrypto.sln with Visual Studio 2017.

  b) Change the configuration to Release

  c) Change the platform to Win32 (32-bit) or x64 (64-bit).

     For x64 you must first provide an x64 OpenSSL static library:
     follow `openssl-prebuilt-lib/README.txt` (the `VC-WIN64A` target)
     and drop the resulting `libcrypto.lib` into `openssl-prebuilt-lib\x64\`.

  d) Right click the project mbedTLS, select Properties, then General, and
     change Platform Toolset to v141. This is per configuration/platform, so
     Win32 and x64 have to be changed separately -- and for x64 it is not
     merely an inherited default, the project file pins it to `Windows7.1SDK`.

     The Runtime Library no longer has to be changed by hand: mbedTLS would
     otherwise build /MD and fail to link against the /MT libsscrypto, so
     `Directory.Build.targets` forces /MT for it.

  e) Right click Solution, and select Build Solution.

3) Output

  * Win32 -> `Release\libsscrypto.dll`
  * x64   -> `x64\Release\libsscrypto64.dll`

  The two DLLs export the same symbols, so a 64-bit host loads the x64 build
  unchanged. Note the different file name: it keeps the two architectures from
  colliding when a host unpacks them to the same directory.

## Continuous integration

`.github/workflows/build.yml` builds both platforms on every push. It differs
from the steps above in two ways, because a hosted runner has neither VS2017
nor a prebuilt OpenSSL:

  * The toolset is overridden on the MSBuild command line
    (`/p:PlatformToolset` and `/p:WindowsTargetPlatformVersion`), which also
    takes care of step 2d. libsodium's output directory is derived from
    `$(PlatformToolset)` and libsscrypto.vcxproj follows it, so the two stay in
    step whichever toolset is selected.
  * OpenSSL is built from source and cached, for both platforms, rather than
    using the `libcrypto.lib` committed under `openssl-prebuilt-lib/`.
