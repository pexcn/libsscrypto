# libsscrypto

Build libsscrypto.dll for shadowsocks-windows.

## Build

1) Get source code

```
git clone https://github.com/shadowsocks/libsscrypto.git
cd libsscrypto
git submodule update --init --recursive
```

`--recursive` matters: mbedTLS has a submodule of its own, and its Visual
Studio project does not build without it.

2) Compile

  a) Open libsscrypto.sln with Visual Studio 2017.

  b) Change the configuration to Release

  c) Change the platform to Win32 (32-bit) or x64 (64-bit).

     For x64 you must first provide an x64 OpenSSL static library:
     follow `openssl-prebuilt-lib/README.txt` (the `VC-WIN64A` target)
     and drop the resulting `libcrypto.lib` into `openssl-prebuilt-lib\x64\`.

  d) Right click Solution, and select Build Solution.

     Nothing has to be changed by hand in the mbedTLS project any more. It
     builds v141 in every configuration now, and `Directory.Build.targets`
     forces its Runtime Library to /MT so it links against the /MT
     libsscrypto. On a newer Visual Studio, override the toolset on the
     command line instead of retargeting the projects, the way CI does:

     ```
     msbuild libsscrypto.sln /p:Configuration=Release /p:Platform=x64 ^
       /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0
     ```

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

  * The toolset is overridden on the MSBuild command line, as above.
    libsodium's output directory is derived from `$(PlatformToolset)` and
    libsscrypto.vcxproj follows it, so the two stay in step whichever toolset
    is selected.
  * OpenSSL is built from source and cached, for both platforms, rather than
    using the `libcrypto.lib` committed under `openssl-prebuilt-lib/`. The
    result is published as the `openssl-<platform>` artifact.
