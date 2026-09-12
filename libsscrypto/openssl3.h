#ifndef SSCRYPTO_OPENSSL3_H
#define SSCRYPTO_OPENSSL3_H

#include <stddef.h>

/*
 * Minimal OpenSSL 3 public ABI surface used by libsscrypto.
 *
 * The repository intentionally carries only libcrypto.lib, not OpenSSL's
 * headers. Keep this mirror small and limited to opaque pointers plus
 * OSSL_PARAM. Define SSCRYPTO_USE_OPENSSL_HEADERS when official OpenSSL
 * headers are available to validate the mirrored OSSL_PARAM layout at
 * compile time.
 */
typedef struct sscrypto_ossl_param_abi {
	const char *key;
	unsigned int data_type;
	void *data;
	size_t data_size;
	size_t return_size;
} SSCRYPTO_OSSL_PARAM_ABI;

#ifdef SSCRYPTO_USE_OPENSSL_HEADERS

#include <openssl/evp.h>
#include <openssl/core.h>

#if defined(__cplusplus)
#define SSCRYPTO_STATIC_ASSERT(expr, msg) static_assert((expr), msg)
#else
#define SSCRYPTO_STATIC_ASSERT(expr, msg) _Static_assert((expr), msg)
#endif

SSCRYPTO_STATIC_ASSERT(sizeof(OSSL_PARAM) == sizeof(SSCRYPTO_OSSL_PARAM_ABI),
	"OSSL_PARAM size mismatch");
SSCRYPTO_STATIC_ASSERT(offsetof(OSSL_PARAM, key) ==
	offsetof(SSCRYPTO_OSSL_PARAM_ABI, key), "OSSL_PARAM.key offset mismatch");
SSCRYPTO_STATIC_ASSERT(offsetof(OSSL_PARAM, data_type) ==
	offsetof(SSCRYPTO_OSSL_PARAM_ABI, data_type),
	"OSSL_PARAM.data_type offset mismatch");
SSCRYPTO_STATIC_ASSERT(offsetof(OSSL_PARAM, data) ==
	offsetof(SSCRYPTO_OSSL_PARAM_ABI, data), "OSSL_PARAM.data offset mismatch");
SSCRYPTO_STATIC_ASSERT(offsetof(OSSL_PARAM, data_size) ==
	offsetof(SSCRYPTO_OSSL_PARAM_ABI, data_size),
	"OSSL_PARAM.data_size offset mismatch");
SSCRYPTO_STATIC_ASSERT(offsetof(OSSL_PARAM, return_size) ==
	offsetof(SSCRYPTO_OSSL_PARAM_ABI, return_size),
	"OSSL_PARAM.return_size offset mismatch");
SSCRYPTO_STATIC_ASSERT(OSSL_PARAM_UTF8_STRING == 4,
	"OSSL_PARAM_UTF8_STRING value mismatch");

#define SSCRYPTO_PARAM_UTF8_STRING OSSL_PARAM_UTF8_STRING

#else

typedef struct ossl_lib_ctx_st OSSL_LIB_CTX;
typedef struct evp_mac_st EVP_MAC;
typedef struct evp_mac_ctx_st EVP_MAC_CTX;
typedef SSCRYPTO_OSSL_PARAM_ABI OSSL_PARAM;

#define SSCRYPTO_PARAM_UTF8_STRING 4

int EVP_Q_digest(OSSL_LIB_CTX *libctx, const char *name, const char *propq,
		 const void *data, size_t datalen, unsigned char *md,
		 size_t *mdlen);

EVP_MAC *EVP_MAC_fetch(OSSL_LIB_CTX *libctx, const char *algorithm,
		       const char *properties);
void EVP_MAC_free(EVP_MAC *mac);
EVP_MAC_CTX *EVP_MAC_CTX_new(EVP_MAC *mac);
void EVP_MAC_CTX_free(EVP_MAC_CTX *ctx);
int EVP_MAC_CTX_set_params(EVP_MAC_CTX *ctx, const OSSL_PARAM params[]);
int EVP_MAC_init(EVP_MAC_CTX *ctx, const unsigned char *key, size_t keylen,
		 const OSSL_PARAM params[]);
int EVP_MAC_update(EVP_MAC_CTX *ctx, const unsigned char *data, size_t datalen);
int EVP_MAC_final(EVP_MAC_CTX *ctx, unsigned char *out, size_t *outl,
		  size_t outsize);

#endif

#define SSCRYPTO_PARAM_UNMODIFIED ((size_t) -1)
#define SSCRYPTO_PARAM_END \
	{ NULL, 0, NULL, 0, 0 }
#define SSCRYPTO_PARAM_UTF8(key, data) \
	{ (key), SSCRYPTO_PARAM_UTF8_STRING, (data), sizeof(data), \
	  SSCRYPTO_PARAM_UNMODIFIED }

#endif
