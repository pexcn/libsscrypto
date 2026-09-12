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

  a) Open libsscrypto.sln with Visual Studio 2022.

  b) x64 is the only platform and Release the only configuration; there is no
     32-bit build and no Debug build. The `libcrypto.lib` committed under
     `openssl-prebuilt-lib/` is what gets linked, so nothing has to be fetched
     or built first.

     BLAKE3, which the 2022-blake3-* methods need for key derivation, is
     compiled from source into the DLL and picks its SIMD backend at run time.
     Its backends are upstream's hand written x86-64 assembly, assembled with
     MASM, which the C++ workload already installs. Nothing extra is needed.

  c) Right click Solution, and select Build Solution.

     The equivalent command-line build is:

     ```
     msbuild libsscrypto.sln /p:Configuration=Release /p:Platform=x64 ^
       /p:PlatformToolset=v143
     ```

3) Output

  `x64\Release\libsscrypto64.dll`

  The `64` in the file name dates from when a 32-bit `libsscrypto.dll` was
  built alongside it and the two had to be told apart. It is kept because
  hosts load the DLL by that name.

## Continuous integration

`.github/workflows/build.yml` builds on every push and, on a tag, publishes
the DLL as a release. It passes `/p:PlatformToolset` on the MSBuild command
line so CI and local builds agree on the toolset. libsodium's output directory
is derived from `$(PlatformToolset)` and libsscrypto.vcxproj follows it, so the
two stay in step whichever toolset is selected.

`.github/workflows/openssl.yml` rebuilds `libcrypto.lib` from source,
following the same steps as `openssl-prebuilt-lib/README.txt`, and publishes
it as the `openssl-x64` artifact. It runs only when started by hand
(Actions -> openssl -> Run workflow, with the version to build), since the
resulting `.lib` is committed to this repository; use it to regenerate
`openssl-prebuilt-lib/` without installing Perl and NASM locally.
