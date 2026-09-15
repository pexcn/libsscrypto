#include "openssl3.h"

#include <blake3.h>

int sscrypto_blake3_derive_key(const unsigned char *context,
			       size_t context_len,
			       const unsigned char *input1,
			       size_t input1_len,
			       const unsigned char *input2,
			       size_t input2_len,
			       unsigned char *output, size_t output_len)
{
	blake3_hasher hasher;

	if ((context == NULL && context_len != 0) ||
	    (input1 == NULL && input1_len != 0) ||
	    (input2 == NULL && input2_len != 0) ||
	    (output == NULL && output_len != 0)) {
		return -1;
	}

	blake3_hasher_init_derive_key_raw(&hasher, context, context_len);
	if (input1_len != 0) {
		blake3_hasher_update(&hasher, input1, input1_len);
	}
	if (input2_len != 0) {
		blake3_hasher_update(&hasher, input2, input2_len);
	}
	blake3_hasher_finalize(&hasher, output, output_len);
	OPENSSL_cleanse(&hasher, sizeof(hasher));
	return 0;
}
