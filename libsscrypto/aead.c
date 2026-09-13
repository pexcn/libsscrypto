#include "openssl3.h"

#define SSCRYPTO_EVP_CTRL_AEAD_GET_TAG 0x10
#define SSCRYPTO_EVP_CTRL_AEAD_SET_TAG 0x11

int sscrypto_aead_encrypt(EVP_CIPHER_CTX *ctx, const unsigned char *nonce,
			 const unsigned char *input, int input_len,
			 unsigned char *output, int tag_len)
{
	int written = 0;
	int final_len = 0;

	if (ctx == NULL || nonce == NULL || output == NULL || input_len < 0 ||
	    tag_len <= 0 || (input == NULL && input_len != 0)) {
		return -1;
	}

	if (EVP_CipherInit_ex(ctx, NULL, NULL, NULL, nonce, 1) != 1 ||
	    EVP_CipherUpdate(ctx, output, &written, input, input_len) != 1 ||
	    EVP_CipherFinal_ex(ctx, output + written, &final_len) != 1) {
		return -1;
	}
	written += final_len;

	if (EVP_CIPHER_CTX_ctrl(ctx, SSCRYPTO_EVP_CTRL_AEAD_GET_TAG, tag_len,
				output + written) != 1) {
		return -1;
	}
	return written + tag_len;
}

int sscrypto_aead_decrypt(EVP_CIPHER_CTX *ctx, const unsigned char *nonce,
			 const unsigned char *input, int input_len,
			 unsigned char *output, int tag_len)
{
	int payload_len;
	int written = 0;
	int final_len = 0;

	if (ctx == NULL || nonce == NULL || input == NULL || output == NULL ||
	    tag_len <= 0 || input_len < tag_len) {
		return -1;
	}
	payload_len = input_len - tag_len;

	if (EVP_CipherInit_ex(ctx, NULL, NULL, NULL, nonce, 0) != 1 ||
	    EVP_CIPHER_CTX_ctrl(ctx, SSCRYPTO_EVP_CTRL_AEAD_SET_TAG, tag_len,
				(void *)(input + payload_len)) != 1 ||
	    EVP_CipherUpdate(ctx, output, &written, input, payload_len) != 1) {
		return -1;
	}

	if (EVP_CipherFinal_ex(ctx, output + written, &final_len) <= 0) {
		return -2;
	}
	return written + final_len;
}
