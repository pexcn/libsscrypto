OpenSSL static library guide

# OpenSSL (version: 3.5.8)
curl -LO https://github.com/openssl/openssl/releases/download/openssl-3.5.8/openssl-3.5.8.tar.gz
tar -xzf openssl-3.5.8.tar.gz
cd openssl-3.5.8

# Win32 x86
set PATH=D:\NASM-32;%PATH%
perl Configure VC-WIN32 no-shared no-apps no-docs no-tests --release
perl -i -pe "s{\s*/Zi\b}{}g, s{\s*/Fd\S+}{}g if /^(?:CNF_)?LIB_CFLAGS\s*=/" makefile
nmake build_libs

# x64
set PATH=D:\NASM-64;%PATH%
perl Configure VC-WIN64A no-shared no-apps no-docs no-tests --release
# others are the same as x86

# Then copy libcrypto.lib from the build root into Win32\ or x64\ here.
