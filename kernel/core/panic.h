#pragma once
#include <stdint.h>

void handle_exception(uint64_t vec, uint64_t err);
void kernel_panic(const char *msg);
