#include <mbedtls/hkdf.h>
#include <mbedtls/md.h>

/*
 * HKDF-SHA1, exported from libsscrypto.def as "hkdf".
 *
 * mbedTLS had no HKDF when this shim was written, so it carried a private
 * implementation that happened to be named mbedtls_hkdf(). Upstream added its
 * own in 2.11, which collides at link time, so this is now a thin wrapper.
 *
 * The wrapper is kept rather than exporting mbedtls_hkdf() directly because
 * the entry point the host binds to hardcodes SHA-1 and takes int lengths,
 * while upstream takes an md_info and size_t lengths.
 */
int sscrypto_hkdf_sha1(const unsigned char *salt, int salt_len,
                       const unsigned char *ikm, int ikm_len,
                       const unsigned char *info, int info_len,
                       unsigned char *okm, int okm_len)
{
	const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);

	if (salt_len < 0 || ikm_len < 0 || info_len < 0 || okm_len < 0) {
		return MBEDTLS_ERR_HKDF_BAD_INPUT_DATA;
	}

	/*
	 * The previous implementation substituted a block of zeros for a NULL
	 * salt whatever salt_len said. Upstream rejects a NULL salt unless the
	 * length is 0, so normalise it here to keep the old behaviour.
	 */
	if (salt == NULL) {
		salt_len = 0;
	}

	return mbedtls_hkdf(md, salt, (size_t) salt_len, ikm, (size_t) ikm_len,
			    info, (size_t) info_len, okm, (size_t) okm_len);
}
