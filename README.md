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

     Release is the only configuration; there is no Debug build.

     For x64 you must first provide an x64 OpenSSL static library:
     follow `openssl-prebuilt-lib/README.txt` (the `VC-WIN64A` target)
     and drop the resulting `libcrypto.lib` into `openssl-prebuilt-lib\x64\`.

  c) Right click the project mbedTLS, select Properties, then C/C++ / Code
     Generation, and change Runtime Library to /MT. mbedTLS leaves this at the
     VC++ default, which is /MD, and mixing it with the /MT libsscrypto fails
     the link with LNK2038. The setting is per configuration/platform, so Win32
     and x64 have to be changed separately.

     The Platform Toolset no longer has to be touched: 3.6.7 builds v141 in
     every configuration, where 2.7.0 pinned `Windows7.1SDK` in Release|x64.

  d) Right click Solution, and select Build Solution.

3) Output

  * Win32 -> `Release\libsscrypto.dll`
  * x64   -> `x64\Release\libsscrypto64.dll`

  The two DLLs export the same symbols, so a 64-bit host loads the x64 build
  unchanged. Note the different file name: it keeps the two architectures from
  colliding when a host unpacks them to the same directory.
