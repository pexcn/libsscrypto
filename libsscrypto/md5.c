#include "openssl3.h"

#define MD5_SIZE 16
#define SSCRYPTO_OPENSSL_ERROR (-1)

/* One-shot MD5 with the ABI historically exported as "md5_ret". */
int sscrypto_md5(const unsigned char *input, size_t input_len,
		 unsigned char output[MD5_SIZE])
{
	static const unsigned char empty = 0;
	size_t output_len = 0;

	if (output == NULL || (input == NULL && input_len != 0)) {
		return SSCRYPTO_OPENSSL_ERROR;
	}
	if (input == NULL) {
		input = &empty;
	}

	if (EVP_Q_digest(NULL, "MD5", NULL, input, input_len, output,
			 &output_len) != 1 || output_len != MD5_SIZE) {
		return SSCRYPTO_OPENSSL_ERROR;
	}

	return 0;
}
