#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

#include <openssl/sha.h>
#include <openssl/crypto.h>
#include <stdint.h>

#define DEFAULT_BYTES 32

void H(uint8_t *digest, const uint8_t *message, size_t len);

#endif // HASH_H_INCLUDED
