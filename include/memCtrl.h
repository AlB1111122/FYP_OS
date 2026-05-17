#pragma once
#include <stddef.h>
#include <stdint.h>
// defined in boot.s
extern "C" void memzero(uint64_t src, uint32_t n);
void cleanInvalidateCache(void *buffer, uint64_t size);
