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

  b) Change the platform to Win32 (32-bit) or x64 (64-bit).

     Release is the only configuration; there is no Debug build.

     For x64 you must first provide an x64 OpenSSL static library:
     follow `openssl-prebuilt-lib/README.txt` (the `VC-WIN64A` target)
     and drop the resulting `libcrypto.lib` into `openssl-prebuilt-lib\x64\`.

  c) Right click the project mbedTLS, and select Properties.

     i) Select General on left panel, and change Platform Toolset to v141.
        (For the x64 configuration this is not just an inherited default --
        the project file pins it to `Windows7.1SDK`, so it must be changed.)

     ii) Select C/C++ / Code Generation, and change Runtime Library to /MT .

     Do this for the configuration you are building; the setting is per
     configuration/platform, so Win32 and x64 have to be changed separately.

  d) Right click Solution, and select Build Solution.

3) Output

  * Win32 -> `Release\libsscrypto.dll`
  * x64   -> `x64\Release\libsscrypto64.dll`

  The two DLLs export the same symbols, so a 64-bit host loads the x64 build
  unchanged. Note the different file name: it keeps the two architectures from
  colliding when a host unpacks them to the same directory.
