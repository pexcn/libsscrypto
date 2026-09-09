# libsscrypto

Build libsscrypto.dll for shadowsocks-windows.

## Build

1) Get source code

```
git clone https://github.com/shadowsocks/libsscrypto.git
cd libsscrypto
git submodule update --init --recursive
```

2) Compile

  a) Open libsscrypto.sln with Visual Studio 2017.

  b) Change the platform to Win32 (32-bit) or x64 (64-bit).

     Release is the only configuration; there is no Debug build. Both
     platforms link the `libcrypto.lib` committed under
     `openssl-prebuilt-lib/`, so nothing has to be fetched or built first.

  c) Right click Solution, and select Build Solution.

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

`.github/workflows/build.yml` builds both platforms on every push and, on a
tag, publishes the DLLs as a release. It differs from the steps above in one
way, because a hosted runner has no VS2017: the toolset is overridden on the
MSBuild command line, as above. libsodium's output directory is derived from
`$(PlatformToolset)` and libsscrypto.vcxproj follows it, so the two stay in
step whichever toolset is selected. Everything else, OpenSSL included, is
what a local build uses.

`.github/workflows/openssl.yml` rebuilds `libcrypto.lib` for both platforms
from source, following the same steps as `openssl-prebuilt-lib/README.txt`, and
publishes each as the `openssl-<platform>` artifact. It runs only when started
by hand (Actions -> openssl -> Run workflow, with the version to build), since
the resulting `.lib` is committed to this repository; use it to regenerate
`openssl-prebuilt-lib/` without installing Perl and NASM locally.
