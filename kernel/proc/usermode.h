#pragma once
#include <stdint.h>
#include "process.h"

void usermode_map_stack(uint64_t *pml4);
void usermode_exec(process_t *proc,
                   uint64_t entry,
                   uint64_t load_end);
