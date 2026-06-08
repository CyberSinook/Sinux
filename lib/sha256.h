#pragma once
#include <stdint.h>
#include <stddef.h>

#define SHA256_BLOCK_SIZE 32

typedef struct {
    uint8_t  data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} sha256_ctx_t;

void sha256_init(sha256_ctx_t *ctx);
void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, size_t len);
void sha256_final(sha256_ctx_t *ctx, uint8_t hash[SHA256_BLOCK_SIZE]);

void sha256_hash(const uint8_t *data, size_t len, uint8_t hash[SHA256_BLOCK_SIZE]);
void sha256_hmac(const uint8_t *key, size_t key_len,
                 const uint8_t *data, size_t data_len,
                 uint8_t hash[SHA256_BLOCK_SIZE]);

void sha256_to_hex(const uint8_t hash[SHA256_BLOCK_SIZE], char *out);
int  sha256_verify(const char *password, const char *hash_hex);
void sha256_make_hash(const char *password, char *hash_hex);
