/*
 * stu_hmac.c
 *
 *  Created on: 2018年1月18日
 *      Author: Tony Lau
 */

#include "../stu_config.h"
#include "stu_core.h"


stu_hmac_t *
stu_hmac_create() {
	stu_hmac_t *hmac;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	static stu_hmac_t  shmac;
	hmac = &shmac;
	stu_hmac_init(hmac);
#else
	hmac = HMAC_CTX_new();
	if (hmac == NULL) {
		return NULL;
	}
#endif

	return hmac;
}

void
stu_hmac_free(stu_hmac_t *hmac) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	static stu_hmac_t  shmac;
	hmac = &shmac;
	stu_hmac_init(hmac);
#else
	HMAC_CTX_free(hmac);
#endif
}
