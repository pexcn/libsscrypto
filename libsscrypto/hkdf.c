#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "openssl3.h"

#include <windows.h>

#include <stdlib.h>
#include <string.h>

#define SSCRYPTO_HKDF_BAD_INPUT_DATA (-0x5F80)
#define SSCRYPTO_OPENSSL_ERROR (-1)
#define SHA1_SIZE 20
#define HKDF_MAX_OUTPUT_SIZE (255 * SHA1_SIZE)

static INIT_ONCE hmac_once = INIT_ONCE_STATIC_INIT;
static EVP_MAC *hmac_mac = NULL;

static void sscrypto_zeroize(void *buffer, size_t length)
{
	volatile unsigned char *p = (volatile unsigned char *) buffer;

	while (length-- != 0) {
		*p++ = 0;
	}
}

/*
 * The fetched EVP_MAC is immutable after publication. Each HKDF context owns
 * its own EVP_MAC_CTX, so concurrent callers share only the algorithm object,
 * not mutable MAC state.
 *
 * MSVC runs a DLL's atexit callbacks when the DLL is unloaded. Register the
 * cache cleanup only after EVP_MAC_fetch() has completed. Any OpenSSL cleanup
 * callback registered during that fetch is therefore older and runs after our
 * callback because atexit is LIFO. Returning FALSE leaves INIT_ONCE
 * uninitialized and allows a later
 * caller to retry if either the fetch or atexit registration failed.
 */
static void sscrypto_hmac_cache_cleanup(void)
{
	EVP_MAC *mac = hmac_mac;

	hmac_mac = NULL;
	EVP_MAC_free(mac);
}

static BOOL CALLBACK sscrypto_hmac_cache_init(PINIT_ONCE once, PVOID parameter,
                                               PVOID *context)
{
	EVP_MAC *mac;

	(void) once;
	(void) parameter;
	(void) context;

	mac = EVP_MAC_fetch(NULL, "HMAC", NULL);
	if (mac == NULL) {
		return FALSE;
	}
	if (atexit(sscrypto_hmac_cache_cleanup) != 0) {
		EVP_MAC_free(mac);
		return FALSE;
	}

	hmac_mac = mac;
	return TRUE;
}

static EVP_MAC *sscrypto_hmac_get(void)
{
	if (!InitOnceExecuteOnce(&hmac_once, sscrypto_hmac_cache_init, NULL, NULL)) {
		return NULL;
	}
	return hmac_mac;
}

static int sscrypto_hkdf_validate(const unsigned char *ikm, int ikm_len,
                                  int salt_len, int info_len,
                                  unsigned char *okm, int okm_len)
{
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
	return 1;
}

/*
 * Allocate a reusable HKDF-SHA1 context. The digest parameter is constant, so
 * set it once here rather than once per derivation. A context is mutable and
 * must not be used concurrently by multiple threads.
 */
void *sscrypto_hkdf_ctx_new(void)
{
	static char digest[] = "SHA1";
	OSSL_PARAM params[] = {
		SSCRYPTO_PARAM_UTF8("digest", digest),
		SSCRYPTO_PARAM_END
	};
	EVP_MAC *mac = sscrypto_hmac_get();
	EVP_MAC_CTX *ctx;

	if (mac == NULL) {
		return NULL;
	}

	ctx = EVP_MAC_CTX_new(mac);
	if (ctx == NULL) {
		return NULL;
	}
	if (EVP_MAC_CTX_set_params(ctx, params) != 1) {
		EVP_MAC_CTX_free(ctx);
		return NULL;
	}
	return ctx;
}

void sscrypto_hkdf_ctx_free(void *opaque_ctx)
{
	EVP_MAC_CTX_free((EVP_MAC_CTX *) opaque_ctx);
}

/*
 * Derive HKDF-SHA1 using a caller-owned context. Reinitializing HMAC after
 * EVP_MAC_final() is supported, so one context can serve repeated datagrams.
 */
int sscrypto_hkdf_ctx_derive(void *opaque_ctx,
                             const unsigned char *salt, int salt_len,
                             const unsigned char *ikm, int ikm_len,
                             const unsigned char *info, int info_len,
                             unsigned char *okm, int okm_len)
{
	static const unsigned char empty = 0;
	static const unsigned char null_salt[SHA1_SIZE] = { 0 };
	EVP_MAC_CTX *ctx = (EVP_MAC_CTX *) opaque_ctx;
	unsigned char prk[SHA1_SIZE] = { 0 };
	unsigned char t[SHA1_SIZE] = { 0 };
	size_t mac_len = 0;
	size_t where = 0;
	size_t t_len = 0;
	unsigned int block = 1;
	int valid;
	int result = SSCRYPTO_OPENSSL_ERROR;

	valid = sscrypto_hkdf_validate(ikm, ikm_len, salt_len, info_len, okm,
	                               okm_len);
	if (valid <= 0) {
		return valid;
	}
	if (ctx == NULL) {
		return SSCRYPTO_HKDF_BAD_INPUT_DATA;
	}

	/* RFC 5869 treats an omitted salt as HashLen zero octets. */
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
	sscrypto_zeroize(prk, sizeof(prk));
	sscrypto_zeroize(t, sizeof(t));
	return result;
}
