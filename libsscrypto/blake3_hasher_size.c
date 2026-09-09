#include <blake3.h>

/*
 * sizeof(blake3_hasher), exported from libsscrypto.def as "blake3_hasher_size".
 *
 * blake3_hasher is a working buffer the caller has to allocate before calling
 * blake3_hasher_init*(), the same arrangement mbedtls_cipher_context_t and
 * cipher_get_size_ex() already use here. Upstream calls the struct a private
 * implementation detail and is free to grow it, so let the host ask for the
 * size rather than hardcoding today's value on the managed side.
 */
size_t sscrypto_blake3_hasher_size(void)
{
	return sizeof(blake3_hasher);
}
