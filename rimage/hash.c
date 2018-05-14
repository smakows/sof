/*
 * Copyright (c) 2017, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 *  Author: Liam Girdwood <liam.r.girdwood@linux.intel.com>
 *          Keyon Jie <yang.jie@linux.intel.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "rimage.h"
#include "file_format.h"
#include "manifest.h"

#if OPENSSL_VERSION_NUMBER < 0x10100000L
void EVP_MD_CTX_free(EVP_MD_CTX *ctx);
EVP_MD_CTX *EVP_MD_CTX_new(void);

static void *OPENSSL_zalloc(size_t num)
{
	void *ret = OPENSSL_malloc(num);

	if (ret != NULL)
		memset(ret, 0, num);
	return ret;
}

EVP_MD_CTX *EVP_MD_CTX_new(void)
{
	return OPENSSL_zalloc(sizeof(EVP_MD_CTX));
}

void EVP_MD_CTX_free(EVP_MD_CTX *ctx)
{
	EVP_MD_CTX_cleanup(ctx);
	OPENSSL_free(ctx);
}
#endif


#define DEBUG_HASH 0

void module_sha256_create(struct image *image)
{
	image->md = EVP_sha256();
	image->mdctx = EVP_MD_CTX_create();

	EVP_DigestInit_ex(image->mdctx, image->md, NULL);
}

void module_sha256_update(struct image *image, uint8_t *data, size_t bytes)
{
	EVP_DigestUpdate(image->mdctx, data, bytes);
}

void module_sha256_complete(struct image *image, uint8_t *hash)
{
	unsigned char md_value[EVP_MAX_MD_SIZE];
	unsigned int md_len;
#if DEBUG_HASH
	int i;
#endif
	EVP_DigestFinal_ex(image->mdctx, md_value, &md_len);
	EVP_MD_CTX_destroy(image->mdctx);

	memcpy(hash, md_value, md_len);
#if DEBUG_HASH
	fprintf(stdout, "Module digest is: ");
	for (i = 0; i < md_len; i++)
		fprintf(stdout, "%02x", md_value[i]);
	fprintf(stdout, "\n");
#endif
}

void ri_hash(struct image *image, unsigned int offset, unsigned int size, uint8_t *hash)
{
	module_sha256_create(image);
	module_sha256_update(image, image->fw_image + offset, size);
	module_sha256_complete(image, hash);
}
