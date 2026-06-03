#pragma once
#include <stdint.h>
#include "process.h"

#define USER_CODE_SEL  0x1B
#define USER_DATA_SEL  0x23

void umode_enter(process_t *p);
void umode_jump(uint64_t entry, uint64_t stack, uint64_t cr3);
