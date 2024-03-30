#include"crypto_core.h"
#include "base.h"
#include <mbedtls/aes.h>
#include <mbedtls/base64.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/md5.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include "core/os/memory.h"

namespace lain {
	Error CryptoCore::RandomGenerator::get_random_bytes(uint8_t* r_buffer, size_t p_bytes) {
		ERR_FAIL_NULL_V(ctx, ERR_UNCONFIGURED);
		int ret = mbedtls_ctr_drbg_random((mbedtls_ctr_drbg_context*)ctx, r_buffer, p_bytes);
		ERR_FAIL_COND_V_MSG(ret, FAILED, " failed\n  ! mbedtls_ctr_drbg_seed returned an error" + itos(ret));
		return OK;
	}
	CryptoCore::RandomGenerator::RandomGenerator() {
		entropy = memalloc(sizeof(mbedtls_entropy_context));
		mbedtls_entropy_init((mbedtls_entropy_context*)entropy);
		mbedtls_entropy_add_source((mbedtls_entropy_context*)entropy, &CryptoCore::RandomGenerator::_entropy_poll, nullptr, 256, MBEDTLS_ENTROPY_SOURCE_STRONG);
		ctx = memalloc(sizeof(mbedtls_ctr_drbg_context));
		mbedtls_ctr_drbg_init((mbedtls_ctr_drbg_context*)ctx);
	}
	CryptoCore::RandomGenerator::~RandomGenerator() {
		mbedtls_ctr_drbg_free((mbedtls_ctr_drbg_context*)ctx);
		memfree(ctx);
		mbedtls_entropy_free((mbedtls_entropy_context*)entropy);
		memfree(entropy);
	}

	Error CryptoCore::RandomGenerator::init() {
		// set seed
		int ret = mbedtls_ctr_drbg_seed((mbedtls_ctr_drbg_context*)ctx, mbedtls_entropy_func, (mbedtls_entropy_context*)entropy, nullptr, 0);
		if (ret) {
			ERR_FAIL_COND_V_MSG(ret, FAILED, " failed\n  ! mbedtls_ctr_drbg_seed returned an error" + itos(ret));
		}
		return OK;
	}

}