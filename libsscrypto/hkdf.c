#include "openssl3.h"

#include <string.h>

#define SSCRYPTO_HKDF_BAD_INPUT_DATA (-0x5F80)
#define SSCRYPTO_OPENSSL_ERROR (-1)
#define SHA1_SIZE 20
#define HKDF_MAX_OUTPUT_SIZE (255 * SHA1_SIZE)

static void sscrypto_zeroize(void *buffer, size_t length)
{
	volatile unsigned char *p = (volatile unsigned char *) buffer;

	while (length-- != 0) {
		*p++ = 0;
	}
}

/*
 * HKDF-SHA1, exported from libsscrypto.def as "hkdf". Keep the legacy ABI:
 * the entry point hardcodes SHA-1 and takes int lengths.
 *
 * Implement RFC 5869 directly on top of OpenSSL 3's EVP_MAC HMAC provider.
 * Besides avoiding EVP_KDF's 32 KiB info limit, this lets one MAC context be
 * reused for Extract and every Expand block, which matters for AEAD-2018 UDP
 * where a new subkey is derived for every datagram.
 */
int sscrypto_hkdf_sha1(const unsigned char *salt, int salt_len,
                       const unsigned char *ikm, int ikm_len,
                       const unsigned char *info, int info_len,
                       unsigned char *okm, int okm_len)
{
	static const unsigned char empty = 0;
	static const unsigned char null_salt[SHA1_SIZE] = { 0 };
	static char digest[] = "SHA1";
	EVP_MAC *mac = NULL;
	EVP_MAC_CTX *ctx = NULL;
	OSSL_PARAM params[] = {
		SSCRYPTO_PARAM_UTF8("digest", digest),
		SSCRYPTO_PARAM_END
	};
	unsigned char prk[SHA1_SIZE] = { 0 };
	unsigned char t[SHA1_SIZE] = { 0 };
	size_t mac_len = 0;
	size_t where = 0;
	size_t t_len = 0;
	unsigned int block = 1;
	int result = SSCRYPTO_OPENSSL_ERROR;

	if (salt_len < 0 || ikm_len < 0 || info_len < 0 || okm_len < 0) {
		return SSCRYPTO_HKDF_BAD_INPUT_DATA;
	}
	if ((ikm == NULL && ikm_len != 0) || okm == NULL ||
	    okm_len > HKDF_MAX_OUTPUT_SIZE) {
		return SSCRYPTO_HKDF_BAD_INPUT_DATA;
	}

	/* mbedTLS accepted a non-NULL output with length zero. */
	if (okm_len == 0) {
		return 0;
	}

	/*
	 * Preserve the wrapper's historical NULL handling. A NULL salt meant
	 * "salt not provided" regardless of salt_len, which RFC 5869 defines as
	 * HashLen zero octets. A NULL info likewise means an empty info string.
	 */
	if (salt == NULL) {
		salt = null_salt;
		salt_len = SHA1_SIZE;
	}
	if (info == NULL) {
		info = &empty;
		info_len = 0;
	}
	if (ikm == NULL) {
		ikm = &empty;
	}

	mac = EVP_MAC_fetch(NULL, "HMAC", NULL);
	if (mac == NULL) {
		goto exit;
	}
	ctx = EVP_MAC_CTX_new(mac);
	EVP_MAC_free(mac);
	mac = NULL;
	if (ctx == NULL || EVP_MAC_CTX_set_params(ctx, params) != 1) {
		goto exit;
	}

	/* HKDF-Extract(salt, IKM) -> PRK. */
	if (EVP_MAC_init(ctx, salt, (size_t) salt_len, NULL) != 1 ||
	    EVP_MAC_update(ctx, ikm, (size_t) ikm_len) != 1 ||
	    EVP_MAC_final(ctx, prk, &mac_len, sizeof(prk)) != 1 ||
	    mac_len != SHA1_SIZE) {
		goto exit;
	}

	/* HKDF-Expand(PRK, info, L) -> OKM. */
	while (where < (size_t) okm_len) {
		unsigned char counter = (unsigned char) block++;
		size_t to_copy;

		if (EVP_MAC_init(ctx, prk, sizeof(prk), NULL) != 1 ||
		    (t_len != 0 && EVP_MAC_update(ctx, t, t_len) != 1) ||
		    (info_len != 0 && EVP_MAC_update(ctx, info, (size_t) info_len) != 1) ||
		    EVP_MAC_update(ctx, &counter, 1) != 1 ||
		    EVP_MAC_final(ctx, t, &mac_len, sizeof(t)) != 1 ||
		    mac_len != SHA1_SIZE) {
			goto exit;
		}

		to_copy = (size_t) okm_len - where;
		if (to_copy > SHA1_SIZE) {
			to_copy = SHA1_SIZE;
		}
		memcpy(okm + where, t, to_copy);
		where += to_copy;
		t_len = SHA1_SIZE;
	}

	result = 0;

exit:
	EVP_MAC_CTX_free(ctx);
	EVP_MAC_free(mac);
	sscrypto_zeroize(prk, sizeof(prk));
	sscrypto_zeroize(t, sizeof(t));
	return result;
}
