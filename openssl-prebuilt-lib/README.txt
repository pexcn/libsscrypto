OpenSSL static library guide

Version: 3.5.8 (the current LTS series; 1.1.0g, which this directory used to
hold, has been end-of-life since 2019).

Only libcrypto is needed. libsscrypto.def re-exports ten EVP_* functions from
it, and those have not changed between 1.1.0 and 3.x.

# Read NOTES-WINDOWS.md and NOTES-PERL.md in the OpenSSL source tree.

# Requirements:
#   - Visual Studio native tools command prompt (x86 or x64, matching the
#     target below)
#   - Perl (Strawberry Perl works; unlike 1.1.0 there is no dmake to install)
#   - NASM, on PATH

# https://github.com/openssl/openssl/issues/1061
# A static build needs these on the linker command line:
#     ws2_32.lib gdi32.lib advapi32.lib crypt32.lib user32.lib
# Otherwise you get unresolved __imp_Cert* / __imp_Crypt* and friends, e.g.
# 1>libcrypto.lib(e_capi.obj) : error LNK2001: unresolved external symbol __imp_CertOpenStore
# They are already listed in libsscrypto.vcxproj, so nothing to do here --
# but keep them in mind if you link libcrypto.lib into anything else.
# (Authoritative list: "VC-noCE-common" in Configurations/10-main.conf.)

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

# no-apps / no-docs / no-tests skip everything but the libraries, which cuts
# most of the build time. Drop them and run "nmake test" if you want the test
# suite. To rebuild from scratch: nmake distclean

# The Windows targets hard-code "/Zi /Fdossl_static.pdb" into lib_cflags
# ("VC-common" in Configurations/10-main.conf) and --release does not undo it.
# /Zi puts type information in ossl_static.pdb but leaves the symbol and line
# tables in the .debug$S section of every object, that is, inside
# libcrypto.lib: 63% of the 43 MB x86 archive and 53% of the 51 MB x64 one.
# The linker only pulls in objects it references, so none of it ever reached
# the shipped DLL; it was purely weight in this repository. The perl line
# above strips both flags out of the generated makefile, which cuts the
# archive to roughly a third. /Zi has no effect on code generation -- the
# emitted code is the same either way -- and without it cl produces no
# ossl_static.pdb at all, which is why none is committed here any more.

# Then copy libcrypto.lib from the build root into Win32\ or x64\ here.

# no-shared makes OpenSSL compile libcrypto with /MT /Zl. /Zl leaves the CRT
# choice out of the object files, so the library links into either a /MT or a
# /MD host -- but libsscrypto builds /MT, so keep it that way.

# .github/workflows/openssl.yml runs exactly these steps for both platforms
# and uploads the result as the "openssl-<platform>" artifact, so you can take
# the .lib from there instead of building it locally. It is triggered by hand
# (Actions -> openssl -> Run workflow) because the .lib next to this file is
# committed and only has to be regenerated when the version above changes.
